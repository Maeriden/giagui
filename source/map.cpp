#include "map.hpp"




int importFile(const char* filePath, int* resolution, std::map<H3Index, CellData>* data)
{
	std::ifstream stream(filePath);
	if(!stream.is_open())
		return 1;
	
	try
	{
		cpptoml::parser parser(stream);
		std::shared_ptr<cpptoml::table> root = parser.parse();
		std::shared_ptr<cpptoml::table> h3 = root->get_table("h3");
		
		*resolution = h3->get_as<int>("resolution").value_or(0);
		for(auto& it : *h3->get_table("values"))
		{
			// TODO: Settle on a single TOML format
			CellData cell = { .water = DOUBLE_NAN, .ice = DOUBLE_NAN, .sediment = DOUBLE_NAN, .density = DOUBLE_NAN };
			if(it.second->is_array())
			{
				std::vector<std::shared_ptr<cpptoml::value<double>>> values = it.second->as_array()->array_of<double>();
				if(values.size() > 0)
					cell.water    = values[0]->get();
				if(values.size() > 1)
					cell.ice      = values[1]->get();
				if(values.size() > 2)
					cell.sediment = values[2]->get();
				if(values.size() > 3)
					cell.density  = values[3]->get();
			}
			else
			{
				cell.water = it.second->as<double>()->get();
			}
			
			H3Index index = std::stoull(it.first, nullptr, 16);
			data->emplace(index, cell);
		}
	}
	catch(cpptoml::parse_exception& ex)
	{
		return 2;
	}
	return 0;
}


int exportFile(const char* filePath, int resolution, std::map<H3Index, CellData>& data)
{
#if ENABLE_ASSERT
	for(auto& it : data)
	{
		assert(!std::isnan(it.second.water) || !std::isnan(it.second.ice) || !std::isnan(it.second.sediment) || !std::isnan(it.second.density));
	}
#endif
	std::ofstream stream(filePath);
	if(!stream.is_open())
		return 1;
	
	try
	{
		stream << "[h3]"          << std::endl;
		stream << "resolution = " << resolution << std::endl;
		stream << "type = '4f'"   << std::endl;
		stream << std::endl;
		stream << "[h3.values]"   << std::endl;
		stream << std::hex;
		for(auto& it : data)
		{
			double water    = std::isnan(it.second.water)    ? 0.0 : it.second.water;
			double ice      = std::isnan(it.second.ice)      ? 0.0 : it.second.ice;
			double sediment = std::isnan(it.second.sediment) ? 0.0 : it.second.sediment;
			double density  = std::isnan(it.second.density)  ? 0.0 : it.second.density;
			stream << it.first << " = [" << water << "," << ice << "," << sediment << "," << density << ']' << std::endl;
		}
	}
	catch(std::ostream::failure& ex)
	{
		return 2;
	}
	return 0;
}
