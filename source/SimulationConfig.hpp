#ifndef GIAGUI_SIMULATIONCONFIG_HPP
#define GIAGUI_SIMULATIONCONFIG_HPP


#include <optional>
#include <vector>


struct SimulationConfig
{
	struct Mesh
	{
		struct Inner
		{
			std::optional<int>         value;
			std::optional<std::string> input;
		} inner;
		
		struct Outer
		{
			std::optional<int>         value;
			std::optional<std::string> input;
		} outer;
	} mesh;
	
	struct Time
	{
		int steps = 1;
	} time;
	
	struct Load
	{
		struct HistoryEntry
		{
			float       time;
			std::string filename;
		};
		
		float scaling = 1.0;
		std::vector<HistoryEntry> history;
	} load;
};

#endif //GIAGUI_SIMULATIONCONFIG_HPP
