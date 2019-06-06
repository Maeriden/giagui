#ifndef GIAGUI_SIMULATIONCONFIG_HPP
#define GIAGUI_SIMULATIONCONFIG_HPP


#include <optional>
#include <list>
#include "Containers.hpp"

struct Dataset;

struct SimulationConfig
{
	struct Mesh
	{
		struct Inner
		{
			std::optional<int> value;
			Dataset*           input = nullptr;
		} inner;
		
		struct Outer
		{
			std::optional<int> value;
			Dataset*           input = nullptr;
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
			double            time;
			HashSet<Dataset*> datasets;
		};
		
		float                   scaling = 1.0;
		std::list<HistoryEntry> history;
	} load;
	
	
	SimulationConfig& operator=(SimulationConfig&& that) noexcept;
};

#endif //GIAGUI_SIMULATIONCONFIG_HPP
