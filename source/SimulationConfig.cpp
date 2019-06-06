#include "SimulationConfig.hpp"


SimulationConfig& SimulationConfig::operator=(SimulationConfig&& that) noexcept
{
	this->mesh.inner.value = that.mesh.inner.value;
	this->mesh.inner.input = that.mesh.inner.input;
	
	this->mesh.outer.value = that.mesh.outer.value;
	this->mesh.outer.input = that.mesh.outer.input;
	
	this->time.steps = that.time.steps;
	
	this->load.scaling = that.load.scaling;
	this->load.history = std::move(that.load.history);
	
	return *this;
}
