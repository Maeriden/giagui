#include "preprocess/Project.hpp"
#include <cpptoml.h>
#include <QApplication> // tr()

namespace poglar
{
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
			stream << "input = " << *name << "\n";
			
			std::ifstream sourceFile(*name, std::ios::binary);
			if(!sourceFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'").arg(QString::fromStdString(*name));
				return false;
			}
			
			// Copy content of input file into poglar directory 
			std::string   targetPath = destination.filePath(QString::fromStdString("mesh/" + *name)).toStdString();
			std::ofstream targetFile(targetPath, std::ios::binary);
			if(!targetFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'").arg(QString::fromStdString(targetPath));
				return false;
			}
			
			targetFile << sourceFile.rdbuf();
			if(targetFile.fail())
			{
				errorMessage = QApplication::tr("Failed to copy '%1' to '%2'").arg(QString::fromStdString(*name)).arg(QString::fromStdString(targetPath));
				return false;
			}
		}
		stream << "\n";
		
		
		stream << "[mesh.outer]\n"
		          "value = " << root->get_qualified_as<int>("mesh.outer.value").value_or(0) << "\n";
		
		if(auto name = root->get_qualified_as<std::string>("mesh.outer.input"))
		{
			stream << "input = " << *name << "\n";
			
			// Copy content of input file into poglar directory 
			std::ifstream sourceFile(*name, std::ios::binary);
			if(!sourceFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'").arg(QString::fromStdString(*name));
				return false;
			}
			
			std::string   targetPath = destination.filePath(QString::fromStdString("mesh/" + *name)).toStdString();
			std::ofstream targetFile(targetPath, std::ios::binary);
			if(!targetFile.is_open())
			{
				errorMessage = QApplication::tr("Failed to open '%1'").arg(QString::fromStdString(targetPath));
				return false;
			}
			
			targetFile << sourceFile.rdbuf();
			if(targetFile.fail())
			{
				errorMessage = QApplication::tr("Failed to copy '%1' to '%2'").arg(QString::fromStdString(*name)).arg(QString::fromStdString(targetPath));
				return false;
			}
		}
		stream << "\n";
		
		
		/* setting up the load section */
		stream << "[load]\n"
		          "scaling = " << root->get_qualified_as<double>("load.scaling").value_or(0.0) << "\n"
		          "\n";
		
		if(std::shared_ptr<cpptoml::table_array> history = root->get_table_array_qualified("load.history"))
		{
			for(std::shared_ptr<cpptoml::table> entry: *history)
			{
				double                   time     = *entry->get_as<double>("time");
				std::vector<std::string> datasets = *entry->get_array_of<std::string>("datasets");
				
				// TODO: What now?
			}
		}
		
		
		/* setting up the time section */
		stream << "[time]\n"
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
