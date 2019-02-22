#ifndef MAP_H
#define MAP_H

#include <cmath>
#include <map>
#include <h3/h3api.h>
#include <QRectF>
#include <QSizeF>
#include <fstream>
#include <cpptoml.h>


#define NEW new(std::nothrow)


#define PI 3.14159265358979323846
#define NORMALIZE_LON(lon) ( ((lon) + PI)   / (2*PI) )
#define NORMALIZE_LAT(lat) ( ((lat) - PI/2) / ( -PI) ) // NOTE: In Qt scene, 0 is top

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L35
#ifndef H3_MODE_OFFSET
#define H3_MODE_OFFSET 59
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L50
#ifndef H3_MODE_MASK
#define H3_MODE_MASK 0b0111100000000000000000000000000000000000000000000000000000000000
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L38
#ifndef H3_BC_OFFSET
#define H3_BC_OFFSET 45
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L56
#ifndef H3_BC_MASK
#define H3_BC_MASK 0b0000000000001111111000000000000000000000000000000000000000000000
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/constants.h#L79
#ifndef H3_HEXAGON_MODE
#define H3_HEXAGON_MODE 1
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L90
#ifndef H3_SET_MODE
#define H3_SET_MODE(i, m) ( ((i) & (~H3_MODE_MASK)) | (((uint64_t)(m)) << H3_MODE_OFFSET) )
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L101
#ifndef H3_SET_BASE_CELL
#define H3_SET_BASE_CELL(i, c) ( ((i) & (~H3_BC_MASK)) | (((uint64_t)(c)) << H3_BC_OFFSET) )
#endif

// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/include/h3Index.h#L80
#ifndef H3_INIT // H3 index with mode 0, res 0, base cell 0, and 7 for all index digits
#define H3_INIT 0b0000000000000000000111111111111111111111111111111111111111111111
#endif

#ifndef H3_INVALID_INDEX
#define H3_INVALID_INDEX 0
#endif

// https://uber.github.io/h3/#/documentation/core-library/resolution-table
#ifndef MAX_SUPPORTED_RESOLUTION
#define MAX_SUPPORTED_RESOLUTION 6
#endif


#define IS_VALID_RESOLUTION(r) ( 0 <= (r) && (r) <= MAX_SUPPORTED_RESOLUTION )


constexpr double DOUBLE_MAX = std::numeric_limits<double>::max();
constexpr double DOUBLE_NAN = std::numeric_limits<double>::quiet_NaN();



struct CellData
{
	double water;
	double ice;
	double sediment;
	double density;
};
#define CELLDATA_INIT ( (CellData){ .water = DOUBLE_NAN, .ice = DOUBLE_NAN, .sediment = DOUBLE_NAN, .density = DOUBLE_NAN } )


struct H3State
{
	int                         resolution;
	std::map<H3Index, CellData> cellsData;
	H3Index                     activeIndex;
	uint64_t                    polyfillIndicesCount;
	H3Index*                    polyfillIndices;
};
extern H3State globalH3State;


inline
void H3State_reset(H3State* h3State, int resolution)
{
	delete[] h3State->polyfillIndices;
	h3State->resolution = resolution;
	h3State->cellsData.clear();
	h3State->activeIndex = H3_INVALID_INDEX;
	h3State->polyfillIndices = nullptr;
	h3State->polyfillIndicesCount = 0;
}


// https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
inline
int powi(int base, int8_t exp)
{
	int result = 1;
	for(;;)
	{
		if(exp & 1)
			result *= base;
		exp = ((uint8_t)exp) >> 1;
		if(!exp)
			break;
		base *= base;
	}
	return result;
}


inline
qreal getLineThickness(int resolution)
{
	assert(IS_VALID_RESOLUTION(resolution));
	qreal result = 0.5f * std::pow(2, -resolution);
	return result;
}


inline
GeoCoord toGeocoord(QPointF ssPoint, QSizeF surfaceSize)
{
	GeoCoord result;
	result.lon = (ssPoint.x() / surfaceSize.width())  * (2*PI) - (PI  );
	result.lat = (ssPoint.y() / surfaceSize.height()) * ( -PI) + (PI/2);
	return result;
}


inline
void toGeocoord(QRectF ssArea, QSizeF sceneSize, GeoCoord* vertices)
{
	vertices[0] = toGeocoord(ssArea.topLeft(),     sceneSize);
	vertices[1] = toGeocoord(ssArea.bottomLeft(),  sceneSize);
	vertices[2] = toGeocoord(ssArea.bottomRight(), sceneSize);
	vertices[3] = toGeocoord(ssArea.topRight(),    sceneSize);
}


inline
QPointF toSurfaceSpace(GeoCoord coord, QSizeF surfaceSize)
{
	QPointF ssPoint = {
		NORMALIZE_LON(coord.lon) * surfaceSize.width(),
		NORMALIZE_LAT(coord.lat) * surfaceSize.height()
	};
	return ssPoint;
}


// https://uber.github.io/h3/#/documentation/core-library/resolution-table
inline
uint64_t h3GetIndexCount(int resolution)
{
	static constexpr uint64_t INDEX_COUNT_PER_RES[MAX_SUPPORTED_RESOLUTION+1] = {
		       122,
		       842,
		     5882,
		    41162,
		   288122,
		 2016842,
		14117882,
	};
	if(IS_VALID_RESOLUTION(resolution))
		return INDEX_COUNT_PER_RES[resolution];
	return 0;
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


// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/lib/h3Index.c#L159
inline
uint64_t h3MaxChildrenCount(int parentRes, int childRes)
{
	assert(IS_VALID_RESOLUTION(parentRes));
	assert(IS_VALID_RESOLUTION(childRes));
	assert(IS_VALID_RESOLUTION(parentRes - childRes));
	return powi(7, (int8_t)(childRes - parentRes));
}


inline
bool edgeCrossesAntimeridian(double a_lon, double b_lon)
{
	// Edge crosses antimeridian if its extremes are more than half a globe away
	if(std::abs(a_lon - b_lon) > PI)
		return true;
	return false;
}


inline
bool polyCrossesAntimeridian(GeoBoundary* boundary)
{
	for(int i = 0; i < boundary->numVerts; ++i)
		if(edgeCrossesAntimeridian(boundary->verts[i].lon, boundary->verts[(i+1)%boundary->numVerts].lon))
			return true;
	return false;
}


inline
int countCrossingPoints(GeoBoundary* boundary)
{
	int result = 0;
	for(int i = 0; i < boundary->numVerts; ++i)
		if(edgeCrossesAntimeridian(boundary->verts[i].lon, boundary->verts[(i+1)%boundary->numVerts].lon))
			result += 1;
	return result;
}


inline
uint64_t polyfillAreaCount(QRectF ssArea, QSizeF surfaceSize, int resolution)
{
	GeoCoord geoCorners[4];
	toGeocoord(ssArea, surfaceSize, geoCorners);
	
	GeoPolygon geoPolygon = {};
	geoPolygon.geofence.numVerts = 4;
	geoPolygon.geofence.verts = geoCorners;
	
	// TODO: I suspect the result of maxPolyfillSize is not the actual number of polygons we need to draw
	// If that is the case, find a way to return the actual number of polygons
	uint64_t indicesLen = maxPolyfillSize(&geoPolygon, resolution);
	return indicesLen;
}


inline
int polyfillArea(QRectF ssArea, QSizeF surfaceSize, int resolution, H3Index** outIndices)
{
	GeoCoord geoCorners[4];
	toGeocoord(ssArea, surfaceSize, geoCorners);
	
	GeoPolygon geoPolygon = {};
	geoPolygon.geofence.numVerts = 4;
	geoPolygon.geofence.verts = geoCorners;
	
	uint64_t indicesLen = maxPolyfillSize(&geoPolygon, resolution);
	if(indicesLen > 0)
	{
		*outIndices = NEW H3Index[indicesLen];
		if(!*outIndices)
		{
			return 1;
		}
		polyfill(&geoPolygon, resolution, *outIndices);
	}
	else
	{
		*outIndices = nullptr;
	}
	
	return 0;
}


int importFile(const char* filePath, int* resolution, std::map<H3Index, CellData>* data);
int exportFile(const char* filePath, int  resolution, std::map<H3Index, CellData>& data);

#endif // MAP_H
