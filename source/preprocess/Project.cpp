#include "preprocess/Project.hpp"
#include "preprocess/H3Map.hpp"
#include <cpptoml.h>
#include <QApplication> // tr()
#include <iostream>

namespace poglar
{
  namespace {
    struct HistoryEntry {
      HistoryEntry(const double time,
                   const std::vector<std::string> &datasets)
      : time(time), datasets(datasets) {}

      double time;
      std::vector<std::string> datasets;
    };

    bool SortByTime(const HistoryEntry &a, const HistoryEntry &b)
    {
      return a.time < b.time;
    }

    double RescaleTime(const double &time)
    {
      return -time * 1000 * 31536000 / 2e11;
    }
  } /* <anon> */
	
	
	Project::Project(const QDir &path)
	: path_(path)
	{}
	
	bool
	Project::export_to(const QDir &destination)
	{
		/* preparing the directories hierarchy */
		if(!destination.mkpath("mesh"))
		{
			errorMessage = QApplication::tr("Failed to create '%1/mesh/'").arg(destination.path());
			return false;
		}
		if(!destination.mkpath("load"))
		{
			errorMessage = QApplication::tr("Failed to create '%1/load/'").arg(destination.path());
			return false;
		}
		if(!destination.mkpath("output"))
		{
			errorMessage = QApplication::tr("Failed to create '%1/output/'").arg(destination.path());
			return false;
		}
		
		/* open the input file for poglar */
		std::string poglarPath = destination.filePath("input.toml").toStdString();
		std::ofstream poglarFile(poglarPath);
		if(!poglarFile.is_open())
		{
			errorMessage = QApplication::tr("Failed to open destination file");
			return false;
		}
		
		// Parse source file
		std::shared_ptr<cpptoml::table> root;
		try
		{
			QString path = path_.filePath("_project.toml");
			root = cpptoml::parse_file(path.toStdString());
		}
		catch(cpptoml::parse_exception& ex)
		{
			errorMessage = QApplication::tr("Failed to open project file");
			return false;
		}
		
		std::ostringstream stream;
		stream << std::showpoint << std::fixed;
		
		/* setting up the mesh section */
		stream << "[mesh]\n"
		          "power = 0.5\n"
		          "output = \"mesh/mesh.vtu\"\n"
		          "\n";
		
		
		stream << "[mesh.inner]\n"
		          "value = " << root->get_qualified_as<int>("mesh.inner.value").value_or(0) << "\n";
		
		if(auto name = root->get_qualified_as<std::string>("mesh.inner.input"))
		{
			// Copy content of input file into poglar directory 
			QString sourcePath = path_.filePath(QString::fromStdString(*name + ".h3"));
			std::ifstream sourceFile(sourcePath.toStdString(), std::ios::binary);
			if(!sourceFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'")
				       .arg(sourcePath);
				return false;
			}
			
			QString targetPath = destination.filePath(QString::fromStdString("mesh/" + *name + ".h3"));
			std::ofstream targetFile(targetPath.toStdString(), std::ios::binary);
			if(!targetFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'")
				       .arg(targetPath);
				return false;
			}
			
			targetFile << sourceFile.rdbuf();
			if(targetFile.fail())
			{
				errorMessage = QApplication::tr("Failed to copy '%1' to '%2'")
				       .arg(sourcePath)
				       .arg(targetPath);
				return false;
			}
			
			stream << "input = \"mesh/" << *name << ".h3\"\n";
		}
		stream << "\n";
		
		stream << "[mesh.outer]\n"
		          "value = " << root->get_qualified_as<int>("mesh.outer.value").value_or(0) << "\n";
		
		if(auto name = root->get_qualified_as<std::string>("mesh.outer.input"))
		{
			// Copy content of input file into poglar directory 
			QString sourcePath = path_.filePath(QString::fromStdString(*name + ".h3"));
			std::ifstream sourceFile(sourcePath.toStdString(), std::ios::binary);
			if(!sourceFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'")
				       .arg(sourcePath);
				return false;
			}
			
			QString targetPath = destination.filePath(QString::fromStdString("mesh/" + *name + ".h3"));
			std::ofstream targetFile(targetPath.toStdString(), std::ios::binary);
			if(!targetFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'")
				       .arg(targetPath);
				return false;
			}
			
			targetFile << sourceFile.rdbuf();
			if(targetFile.fail())
			{
				errorMessage = QApplication::tr("Failed to copy '%1' to '%2'")
				       .arg(sourcePath)
				       .arg(targetPath);
				return false;
			}
			
			stream << "input = \"mesh/" << *name << ".h3\"\n";
		}
		stream << "\n";
		
		
		/* setting up the load section */
		stream << "[load]\n"
		          "scaling = " << root->get_qualified_as<double>("load.scaling").value_or(0.0) << "\n"
		          "\n";
		
		std::vector<HistoryEntry> history;
		if(std::shared_ptr<cpptoml::table_array> history_array = root->get_table_array_qualified("load.history"))
		{
			for(std::shared_ptr<cpptoml::table> entry: *history_array)
			{
				history.emplace_back(RescaleTime(*entry->get_as<double>("time")),
				                     *entry->get_array_of<std::string>("filename"));
			}
			std::sort(history.begin(), history.end(), SortByTime);
			
			
			for(uint i = 0; i < history.size(); ++i)
			{
				const HistoryEntry &entry = history[i];
				
				H3Map<double> load;
				for(const std::string &filename: entry.datasets)
				{
					QString sourcePath = path_.filePath(QString::fromStdString(filename + ".h3"));
					H3Map<double> layer;
					layer.read(sourcePath.toStdString());
					
					load.add(layer);
				}
				load.scale(9.80655 / 3.1392202754452325e7);
				
				H3Map<vec3d> vload = SphericalTopography(load.resolution());
				vload.scale(load);
				
				QString filename = QString("load/%1.h3").arg(i, 5, 10, QChar('0'));
				QString targetPath = destination.filePath(filename);
				vload.write(targetPath.toStdString());
				
				stream << "[[load.history]]\n"
				          "time = " << entry.time << "\n"
				          "filename = \"" << filename.toStdString() << "\"\n"
				          "\n";
			}
		}
		
		
		/* setting up the time section */
		stream << "[time]\n"
		          "start = " << (history.size() > 0 ? history.front().time : 0) << "\n"
		          "end = "   << (history.size() > 0 ? history.back().time  : 0) << "\n"
		          "steps = " << root->get_qualified_as<int>("time.steps").value_or(10) << "\n"
		          "\n";
		
		poglarFile << stream.str();
		if(poglarFile.fail())
		{
			errorMessage = QApplication::tr("Failed to write configuration to '%1'").arg(QString::fromStdString(poglarPath));
			return false;
		}
		return true;
	}

} /* namespace poglar */
