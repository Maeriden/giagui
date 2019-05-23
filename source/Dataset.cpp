#include "Dataset.hpp"

#include <cmath>
#include <utility>
#include "MapUtils.hpp"


Dataset::Dataset(DatasetID_t  id) :
	id(std::move(id)),
	resolution(0),
	defaultValue{0},
	density(NO_DENSITY),
	isInteger(false),
	measureUnit(""),
	minValue{0},
	maxValue{0}
{}


Dataset::Dataset(DatasetID_t id, bool hasDensity, bool isInteger) :
	id(std::move(id)),
	resolution(0),
	defaultValue{0},
	density(hasDensity ? 0.0 : NO_DENSITY),
	isInteger(isInteger),
	measureUnit(""),
	minValue{0},
	maxValue{0}
{}


bool Dataset::geoValuesAreEqual(GeoValue a, GeoValue b)
{
	bool result;
	if(isInteger)
		result = a.integer == b.integer;
	else
		result = a.real == b.real;
	return result;
}


bool Dataset::hasDensity()
{
	bool result = !std::isnan(density);
	return result;
}


void Dataset::increaseResolution(int newResolution)
{
	assert(IS_VALID_RESOLUTION(newResolution));
	assert(newResolution > resolution);
	
	uint64_t childrenBufferLength = h3MaxChildrenCount(resolution, newResolution);
	H3Index* childrenBuffer       = new H3Index[childrenBufferLength];
	
	HashMap<H3Index, GeoValue> childrenGeoValues;
	for(auto& [parentIndex, parentGeoValue] : geoValues)
	{
		h3ToChildren(parentIndex, newResolution, childrenBuffer);
		
		for(uint64_t i = 0; i < childrenBufferLength; ++i)
		{
			H3Index childIndex = childrenBuffer[i];
			childrenGeoValues[childIndex] = parentGeoValue;
		}
	}
	childrenGeoValues.erase(H3_INVALID_INDEX);
	
	geoValues  = std::move(childrenGeoValues);
	resolution = newResolution;
	delete[] childrenBuffer;
}


void Dataset::decreaseResolution(int newResolution)
{
	assert(IS_VALID_RESOLUTION(newResolution));
	assert(newResolution < resolution);
	
	HashMap<H3Index, int>    childrenCount;
	HashMap<H3Index, GeoValue> newGeoValues;
	if(isInteger)
	{
		for(auto [childIndex, childGeoValue] : geoValues)
		{
			H3Index   parentIndex    = h3ToParent(childIndex, newResolution);
			GeoValue& parentGeoValue = newGeoValues[parentIndex];
			parentGeoValue.integer += childGeoValue.integer;
			childrenCount[parentIndex] += 1;
		}
		
		for(auto& [parentIndex, parentGeoValue] : newGeoValues)
		{
			assert(childrenCount.count(parentIndex) > 0);
			assert(childrenCount[parentIndex] > 0);
			parentGeoValue.integer = parentGeoValue.integer / childrenCount[parentIndex];
		}
	}
	else
	{
		for(auto [childIndex, childGeoValue] : geoValues)
		{
			H3Index   parentIndex    = h3ToParent(childIndex, newResolution);
			GeoValue& parentGeoValue = newGeoValues[parentIndex];
			parentGeoValue.real += childGeoValue.real;
			childrenCount[parentIndex] += 1;
		}
		
		for(auto& [parentIndex, parentGeoValue] : newGeoValues)
		{
			assert(childrenCount.count(parentIndex) > 0);
			assert(childrenCount[parentIndex] > 0);
			parentGeoValue.real = parentGeoValue.real / childrenCount[parentIndex];
		}
	}
	
	geoValues  = std::move(newGeoValues);
	resolution = newResolution;
}


bool Dataset::findGeoValue(H3Index index, GeoValue* outValue)
{
	assert(index != H3_INVALID_INDEX);
	assert(outValue);
	
	auto it = geoValues.find(index);
	if(it != geoValues.end())
	{
		*outValue = it->second;
		return true;
	}
	return false;
}


size_t Dataset::removeGeoValue(H3Index index)
{
	assert(index != H3_INVALID_INDEX);
	
	size_t affectedCount = geoValues.erase(index);
	return affectedCount;
}


size_t Dataset::updateGeoValue(H3Index index, GeoValue newValue)
{
	assert(index != H3_INVALID_INDEX);
	assert(isInteger || std::isfinite(newValue.real));
	
	auto [iter, created] = geoValues.insert({index, newValue});
	if(created)
		return 1;
	
	GeoValue oldValue = iter->second;
	if(isInteger)
	{
		if(oldValue.integer != newValue.integer)
		{
			geoValues[index] = newValue;
			return 1;
		}
	}
	else
	{
		if(oldValue.real != newValue.real)
		{
			geoValues[index] = newValue;
			return 1;
		}
	}
	return 0;
}
