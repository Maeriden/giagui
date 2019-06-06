#ifndef GIAGUI_MAPUTILS_HPP
#define GIAGUI_MAPUTILS_HPP


#include <cmath>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <h3/h3api.h>

#include "GeoValue.hpp"

#define _STR(x) #x
#define STR(x) _STR(x)


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
#define POLYFILL_INDEX_THRESHOLD 100000
#define UI_DOUBLE_PRECISION 6
#define UI_MULTIPLE_GEOVALUES_STRING "—"


constexpr double DOUBLE_MAX = std::numeric_limits<double>::max();


inline
GeoCoord toGeoCoord(QPointF ssPoint, QSizeF surfaceSize)
{
	GeoCoord result;
	result.lon = (ssPoint.x() / surfaceSize.width())  * (2*PI) - (PI  );
	result.lat = (ssPoint.y() / surfaceSize.height()) * ( -PI) + (PI/2);
	return result;
}


inline
void toGeoCoord(QRectF ssArea, QSizeF sceneSize, GeoCoord* vertices)
{
	vertices[0] = toGeoCoord(ssArea.topLeft(), sceneSize);
	vertices[1] = toGeoCoord(ssArea.bottomLeft(), sceneSize);
	vertices[2] = toGeoCoord(ssArea.bottomRight(), sceneSize);
	vertices[3] = toGeoCoord(ssArea.topRight(), sceneSize);
}


inline
QPointF toMapCoord(GeoCoord coord, QSizeF surfaceSize)
{
	QPointF ssPoint = {
		NORMALIZE_LON(coord.lon) * surfaceSize.width(),
		NORMALIZE_LAT(coord.lat) * surfaceSize.height()
	};
	return ssPoint;
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


// https://github.com/uber/h3/blob/5a55394937466f6d8b50e2da62813db29f40bdd0/src/h3lib/lib/h3Index.c#L159
inline
uint64_t h3MaxChildrenCount(int parentRes, int childRes)
{
	assert(IS_VALID_RESOLUTION(parentRes));
	assert(IS_VALID_RESOLUTION(childRes));
	assert(IS_VALID_RESOLUTION(childRes - parentRes));
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
	{
		double a_lon = boundary->verts[i].lon;
		double b_lon = boundary->verts[(i+1)%boundary->numVerts].lon;
		if(edgeCrossesAntimeridian(a_lon, b_lon))
			return true;
	}
	return false;
}


inline
int countEdgesCrossingAntimeridian(GeoBoundary* boundary)
{
	int result = 0;
	for(int i = 0; i < boundary->numVerts; ++i)
	{
		double a_lon = boundary->verts[i].lon;
		double b_lon = boundary->verts[(i+1)%boundary->numVerts].lon;
		if(edgeCrossesAntimeridian(a_lon, b_lon))
			result += 1;
	}
	return result;
}


inline
GeoValue toGeoValue(const QString& text, bool isInteger, bool* ok)
{
	GeoValue result = {0};
	if(isInteger)
		result.integer = (int64_t)text.toLong(ok);
	else
		result.real = text.toDouble(ok);
	return result;
}


#endif //GIAGUI_MAPUTILS_HPP
