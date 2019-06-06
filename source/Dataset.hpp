#ifndef GIAGUI_MAP_H
#define GIAGUI_MAP_H

#include <cmath>
#include <QMetaType>
#include <h3/h3api.h>

#include "Containers.hpp"
#include "GeoValue.hpp"


constexpr double DOUBLE_NAN = std::numeric_limits<double>::quiet_NaN();


using DatasetID_t = std::string;

struct Dataset
{
	static constexpr double NO_DENSITY = DOUBLE_NAN;
	
	
	DatasetID_t                id;
	HashMap<H3Index, GeoValue> geoValues;
	int                        resolution;
	GeoValue                   defaultValue;
	double                     density;
	bool                       isInteger;
	std::string                measureUnit;
	GeoValue                   minValue;
	GeoValue                   maxValue;
	
	
	explicit Dataset();
//	explicit Dataset(DatasetID_t id);
	Dataset(DatasetID_t id, bool hasDensity, bool isInteger);
	
	bool   geoValuesAreEqual(GeoValue a, GeoValue b);
	bool   hasDensity();
	void   increaseResolution(int newResolution);
	void   decreaseResolution(int newResolution);
	bool   findGeoValue(H3Index index, GeoValue* outValue);
	size_t removeGeoValue(H3Index index);
	size_t updateGeoValue(H3Index index, GeoValue newValue);
};
Q_DECLARE_METATYPE(Dataset*)


#if 0
// https://uber.github.io/h3/#/documentation/core-library/resolution-table
inline
uint64_t h3GetIndexCount(int resolution)
{
	assert(IS_VALID_RESOLUTION(resolution));
	static constexpr uint64_t INDEX_COUNT_PER_RES[MAX_SUPPORTED_RESOLUTION+1] = {
		122,
		842,
		5882,
		41162,
		288122,
		2016842,
		14117882,
	};
	return INDEX_COUNT_PER_RES[resolution];
}


// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/lib/baseCells.c#L904
inline
H3Index h3GetIndexRes0(int baseCellIndex)
{
	assert(0 <= baseCellIndex && baseCellIndex <= 121);
	H3Index h3Index = H3_INIT;
	h3Index = H3_SET_MODE(h3Index, H3_HEXAGON_MODE);
	h3Index = H3_SET_BASE_CELL(h3Index, baseCellIndex);
	return h3Index;
}


// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/lib/h3Index.c#L193
inline
uint64_t h3GetAllIndices(int resolution, H3Index** outIndices)
{
	assert(IS_VALID_RESOLUTION(resolution));
	
	uint64_t indicesLength = numHexagons(resolution);
	H3Index* indices       = new H3Index[indicesLength];
	
	uint64_t childrenBufferLength = h3MaxChildrenCount(0, resolution);
	H3Index* childrenBuffer       = new H3Index[childrenBufferLength];
	
	uint64_t indicesCount = 0;
	for(int i = 0; i < res0IndexCount(); ++i)
	{
		H3Index res0Index = h3GetIndexRes0(i);
		h3ToChildren(res0Index, resolution, childrenBuffer);
		for(uint64_t j = 0; j < childrenBufferLength; ++j)
		{
			if(childrenBuffer[j] != H3_INVALID_INDEX)
				indices[indicesCount++] = childrenBuffer[j];
		}
	}
	delete[] childrenBuffer;
	
	*outIndices = indices;
	return indicesCount;
}
#endif


#endif // GIAGUI_MAP_H
