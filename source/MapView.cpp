#include "MapView.hpp"

#include <cmath>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QGraphicsSvgItem>

#include "Dataset.hpp"


#define POLYFILL_WIDTH_FACTOR 0.45


inline
float getLineThickness(int resolution)
{
	assert(IS_VALID_RESOLUTION(resolution));
	float result = 0.5f * std::pow(2, -resolution);
	return result;
}


GeoCoord getEasternAntimeridianCrossingPoint(const GeoCoord& east, const GeoCoord& west)
{
	assert(east.lon >= 0.0);
	assert(west.lon <= 0.0);
	
	GeoCoord anti; // antimeridian crossing point
	anti.lon = PI; // we always draw the eastern segment first because I like it this way
	
	// Compute longitude `WEST` (move `west` by one full globe in the direction of the prime meridian) 
	// Basically, we remap the longitude of `west` from [-180, 0] to [180, 360]
	// On the globe, `WEST` has the same angular distance from `east` as the original `west`, and computing the length by difference is correct
	double WEST_lon = west.lon + 2*PI;
	double east2anti_longitude_len = anti.lon - east.lon;
	double east2west_longitude_len = WEST_lon - west.lon;
	
	// Compute latitude by linear interpolation
	// c = a + t*(b-a)    =>    t = (c-a) / (b-a)
	double t = east2anti_longitude_len / east2west_longitude_len;
	anti.lat = east.lat + t * (west.lat - east.lat);
	
	return anti;
}


void drawBoundaryEdgesAntimeridian(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	for(int i = 0; i < geoBoundary->numVerts; ++i)
	{
		const GeoCoord& p = geoBoundary->verts[i];
		const GeoCoord& q = geoBoundary->verts[(i+1) % geoBoundary->numVerts];
		
		if(edgeCrossesAntimeridian(p.lon, q.lon))
		{
			const GeoCoord& east = p.lon > 0 ? p : q;
			const GeoCoord& west = p.lon > 0 ? q : p;
			GeoCoord anti = getEasternAntimeridianCrossingPoint(east, west);
			
			painter->drawLine(QLineF(toMapCoord(east, surfaceSize), toMapCoord(anti, surfaceSize)));
			anti.lon = -anti.lon;
			painter->drawLine(QLineF(toMapCoord(west, surfaceSize), toMapCoord(anti, surfaceSize)));
		}
		else
		{
			QLineF line(toMapCoord(p, surfaceSize), toMapCoord(q, surfaceSize));
			painter->drawLine(line);
		}
	}
}


void drawBoundaryEdges(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	if(!polyCrossesAntimeridian(geoBoundary))
	{
		QPointF points[MAX_CELL_BNDRY_VERTS];
		for(int i = 0; i < geoBoundary->numVerts; ++i)
			points[i] = toMapCoord(geoBoundary->verts[i], surfaceSize);
		painter->drawConvexPolygon(points, geoBoundary->numVerts);
	}
	else
	{
		drawBoundaryEdgesAntimeridian(painter, geoBoundary, surfaceSize);
	}
}


void drawBoundaryPolar(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	QPointF points[MAX_CELL_BNDRY_VERTS+4];
	int     pointsCount = 0;
	for(int i = 0; i < geoBoundary->numVerts; ++i)
	{
		const GeoCoord& p = geoBoundary->verts[i];
		points[pointsCount++] = toMapCoord(p, surfaceSize);
		
		const GeoCoord& q = geoBoundary->verts[(i+1) % geoBoundary->numVerts];
		if(edgeCrossesAntimeridian(p.lon, q.lon))
		{
#if 0
			// c = p + t*(q-p) -> t = (c-p)/(q-p)
			double t;
			if(q.lon < 0)
			{
				double q_lon = q.lon + 2*PI;
				t = ( PI - p.lon) / (q_lon - p.lon);
			}
			else
			{
				double q_lon = q.lon - 2*PI;
				t = (-PI - p.lon) / (q_lon - p.lon);
			}
			
			GeoCoord amCrossPoint = {
				.lat = p.lat + t * (q.lat - p.lat),
				.lon = isNorthern ? PI : -PI,
			};
			points[pointsCount++] = toMapCoord(amCrossPoint, surfaceSize);
			
			// All points are either above or below the equator, so check any one point
			bool isNorthern = geoBoundary->verts[0].lat > 0;
			if(isNorthern)
			{
				points[pointsCount++] = QPointF(surfaceSize.width(), 0); // top-right corner
				points[pointsCount++] = QPointF(0, 0);                   // top-left corner
			}
			else
			{
				points[pointsCount++] = QPointF(0, surfaceSize.height());                   // bottom-left corner
				points[pointsCount++] = QPointF(surfaceSize.width(), surfaceSize.height()); // bottom-right corner
			}
			
			amCrossPoint.lon = -amCrossPoint.lon;
			points[pointsCount++] = toMapCoord(amCrossPoint, surfaceSize);
#else
			const GeoCoord& east = p.lon > 0 ? p : q;
			const GeoCoord& west = p.lon > 0 ? q : p;
			GeoCoord anti = getEasternAntimeridianCrossingPoint(east, west);
			
			// All points are either above or below the equator, so check any one point
			bool isNorthern = p.lat > 0;
			// NOTE: Points must be added in counterclockwise order
			if(isNorthern)
			{
				points[pointsCount++] = toMapCoord(anti, surfaceSize);   // eastern crossing point
				points[pointsCount++] = QPointF(surfaceSize.width(), 0); // top-right corner
				points[pointsCount++] = QPointF(0, 0);                   // top-left corner
				anti.lon = -anti.lon;
				points[pointsCount++] = toMapCoord(anti, surfaceSize);   // western crossing point
			}
			else
			{
				anti.lon = -anti.lon; // First point in ccw order is western, not eastern
				points[pointsCount++] = toMapCoord(anti, surfaceSize);                      // western crossing point
				points[pointsCount++] = QPointF(0, surfaceSize.height());                   // bottom-left corner
				points[pointsCount++] = QPointF(surfaceSize.width(), surfaceSize.height()); // bottom-right corner
				anti.lon = -anti.lon; // go back to eastern point
				points[pointsCount++] = toMapCoord(anti, surfaceSize);                      // eastern crossing point
			}
#endif
		}
	}
	
	// NOTE: Polygon can be concave
	painter->drawPolygon(points, pointsCount);
	
#if ENABLE_DEBUG_DRAW_GEOBOUNDARY_VERTICES
	QFont font = painter->font();
	font.setPointSizeF(font.pointSizeF() * 0.4);
	painter->setFont(font);
	for(int i = 0; i < pointsCount; ++i)
	{
		painter->setPen(QPen(QColor(255, 0, 255, 255), 1));
		painter->setBrush(QBrush(QColor(255, 0, 0, 63), Qt::BrushStyle::SolidPattern));
		painter->drawPoint(points[i]);
		
		painter->setPen(QPen(QColor(0, 0, 0, 255), 1));
		painter->setBrush(QBrush(QColor(0, 0, 0, 255), Qt::BrushStyle::SolidPattern));
		painter->drawText(points[i], QString::number(i));
	}
#endif
}


void drawBoundaryAntimeridian(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	QPointF pointsEast[MAX_CELL_BNDRY_VERTS];
	int     pointsEastCount = 0;
	QPointF pointsWest[MAX_CELL_BNDRY_VERTS];
	int     pointsWestCount = 0;
	
	for(int i = 0; i < geoBoundary->numVerts; ++i)
	{
		const GeoCoord& p = geoBoundary->verts[i];
		if(p.lon > 0)
			pointsEast[pointsEastCount++] = toMapCoord(p, surfaceSize);
		else
			pointsWest[pointsWestCount++] = toMapCoord(p, surfaceSize);
		
		const GeoCoord& q = geoBoundary->verts[(i+1) % geoBoundary->numVerts];
		if(edgeCrossesAntimeridian(p.lon, q.lon))
		{
#if 0
			// c = p + t*(q-p) -> t = (c-p)/(q-p)
			double t;
			if(q.lon < 0)
			{
				double q_lon = q.lon + 2*PI;
				t = ( PI - p.lon) / (q_lon - p.lon);
			}
			else
			{
				double q_lon = q.lon - 2*PI;
				t = (-PI - p.lon) / (q_lon - p.lon);
			}
			
			GeoCoord amCrossPoint = {
				.lat = p.lat + t * (q.lat - p.lat),
				.lon = PI,
			};
			pointsEast[pointsEastCount++] = toMapCoord(amCrossPoint, surfaceSize);
			amCrossPoint.lon = -amCrossPoint.lon;
			pointsWest[pointsWestCount++] = toMapCoord(amCrossPoint, surfaceSize);
#else
			const GeoCoord& east = p.lon > 0 ? p : q;
			const GeoCoord& west = p.lon > 0 ? q : p;
			GeoCoord anti = getEasternAntimeridianCrossingPoint(east, west);
			
			pointsEast[pointsEastCount++] = toMapCoord(anti, surfaceSize);
			anti.lon = -anti.lon;
			pointsWest[pointsWestCount++] = toMapCoord(anti, surfaceSize);
			
#endif
		}
	}
	
	assert(pointsEastCount > 0);
	assert(pointsWestCount > 0);
	painter->drawConvexPolygon(pointsEast, pointsEastCount);
	painter->drawConvexPolygon(pointsWest, pointsWestCount);
}


void drawBoundary(QPainter* painter, GeoBoundary* geoBoundary, QSizeF surfaceSize)
{
	int crossPointsCount = countEdgesCrossingAntimeridian(geoBoundary);
	if(crossPointsCount == 0)
	{
		QPointF points[MAX_CELL_BNDRY_VERTS];
		for(int i = 0; i < geoBoundary->numVerts; ++i)
			points[i] = toMapCoord(geoBoundary->verts[i], surfaceSize);
		painter->drawConvexPolygon(points, geoBoundary->numVerts);
		
#if ENABLE_DEBUG_DRAW_GEOBOUNDARY_VERTICES
		QFont font = painter->font();
		font.setPointSizeF(font.pointSizeF() * 0.4);
		painter->setFont(font);
		for(int i = 0; i < geoBoundary->numVerts; ++i)
		{
			painter->setPen(QPen(QColor(255, 0, 255, 255), 1));
			painter->setBrush(QBrush(QColor(255, 0, 0, 63), Qt::BrushStyle::SolidPattern));
			painter->drawPoint(points[i]);
			
			painter->setPen(QPen(QColor(0, 0, 0, 255), 1));
			painter->setBrush(QBrush(QColor(0, 0, 0, 255), Qt::BrushStyle::SolidPattern));
			painter->drawText(points[i], QString::number(i));
		}
#endif
	}
	else if(crossPointsCount == 1)
	{
		drawBoundaryPolar(painter, geoBoundary, surfaceSize);
	}
	else
	{
		drawBoundaryAntimeridian(painter, geoBoundary, surfaceSize);
	}
}


void MapView::drawForeground(QPainter* painter, const QRectF& exposed)
{
#if 1
	if(!dataset)
		return;
	
	QSizeF mapSize = this->mapSize();
	GeoBoundary geoBoundary;
	
	painter->setPen(datasetPen);
	
	if(dataset->isInteger)
	{
		for(auto [index, geoValue] : dataset->geoValues)
		{
			assert(index != H3_INVALID_INDEX);
			
			QColor color = getGeoValueColor(geoValue.integer);
			datasetBrush.setColor(color);
			painter->setBrush(datasetBrush);
			
			h3ToGeoBoundary(index, &geoBoundary);
			drawBoundary(painter, &geoBoundary, mapSize);
		}
	}
	else
	{
		for(auto [index, geoValue] : dataset->geoValues)
		{
			assert(index != H3_INVALID_INDEX);
			
			QColor color = getGeoValueColor(geoValue.real);
			datasetBrush.setColor(color);
			painter->setBrush(datasetBrush);
			
			h3ToGeoBoundary(index, &geoBoundary);
			drawBoundary(painter, &geoBoundary, mapSize);
		}
	}
	
	
	if(gridIndices)
	{
		float penStrokeWidth = getLineThickness(dataset->resolution);
		polyfillPen.setWidthF(penStrokeWidth);
		
		painter->setPen(polyfillPen);
		painter->setBrush(polyfillBrush);
		
		for(H3Index index : *gridIndices)
		{
			assert(index != H3_INVALID_INDEX);
			
			h3ToGeoBoundary(index, &geoBoundary);
			drawBoundaryEdges(painter, &geoBoundary, mapSize);
		}
	}
	
	
	if(highlightIndices)
	{
		float penStrokeWidth = 2 * getLineThickness(dataset->resolution);
		highlightPen.setWidthF(penStrokeWidth);
		
		painter->setPen(highlightPen);
		painter->setBrush(highlightBrush);
		
		for(H3Index index : *highlightIndices)
		{
			assert(index != H3_INVALID_INDEX);
			
			h3ToGeoBoundary(index, &geoBoundary);
			drawBoundary(painter, &geoBoundary, mapSize);
		}
	}
#else
	if(geoDataset && geoDataset->geoValues.size() > 0)
	{
		qreal penStrokeWidth = getLineThickness(geoDataset->resolution);
		datasetPen.setWidthF(penStrokeWidth);
		polyfillPen.setWidthF(penStrokeWidth);
		highlightPen.setWidthF(penStrokeWidth);
		
		
		QSizeF mapSize = this->mapSize();
		GeoBoundary geoBoundary;
		for(auto [index, geoValue] : geoDataset->geoValues)
		{
			assert(index != H3_INVALID_INDEX);
			h3ToGeoBoundary(index, &geoBoundary);
			
			painter->setBrush(datasetBrush);
			painter->setPen(datasetPen);
			drawBoundary(painter, &geoBoundary, mapSize);
			
			
			if(gridIndices && gridIndices->count(index) > 0)
			{
				painter->setBrush(polyfillBrush);
				painter->setPen(polyfillPen);
				drawBoundaryEdges(painter, &geoBoundary, mapSize);
			}
			
			
			if(highlightIndices && highlightIndices->count(index) > 0)
			{
				painter->setBrush(highlightBrush);
				painter->setPen(highlightPen);
				drawBoundary(painter, &geoBoundary, mapSize);
			}
		}
	}
#endif
}


MapView::MapView(HashSet<H3Index>* highlightIndices, HashSet<H3Index>* gridIndices, QWidget* parent) : QGraphicsView(parent)
{
	this->highlightIndices = highlightIndices;
	this->gridIndices      = gridIndices;
	setMouseTracking(true);
	
	QGraphicsScene* scene = new QGraphicsScene(this);
	setScene(scene);
	
	mapGraphicsItem = new QGraphicsSvgItem(":/images/world.svg");
	scene->addItem(mapGraphicsItem);
}


void MapView::setInteractionMode(InteractionMode mode)
{
	interactionMode = mode;
}


void MapView::setDataSource(Dataset* dataset)
{
	if(this->dataset == dataset)
		return;
	this->dataset = dataset;
	scene()->invalidate();
}


void MapView::zoom(QPoint vsAnchor, double steps)
{
	// TODO: Maybe set a min-max range on scale?
	QPointF ssAnchor = mapToScene(vsAnchor);
	qreal factor = std::pow(1.2, steps);
	scale(factor, factor);
	
	centerOn(ssAnchor);
	QPointF delta_viewport_pos = vsAnchor - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
	QPointF viewport_center = mapFromScene(ssAnchor) - delta_viewport_pos;
	centerOn(mapToScene(viewport_center.toPoint()));
}


void MapView::requestRepaint()
{
	scene()->invalidate();
}


void MapView::redrawValuesRange()
{
	assert(dataset);
	
//	QLinearGradient gradient = QLinearGradient(QPointF(), QPointF());
//	QColor minColor = dataset->isInteger ? getGeoValueColor(dataset->minValue.integer) : getGeoValueColor(dataset->minValue.real);
//	QColor maxColor = dataset->isInteger ? getGeoValueColor(dataset->maxValue.integer) : getGeoValueColor(dataset->maxValue.real);
//	gradient.setColorAt(0, minColor);
//	gradient.setColorAt(1, maxColor);
	
	scene()->invalidate();
}


QSizeF MapView::mapSize() const
{
	return mapGraphicsItem->boundingRect().size();
//	return sceneRect().size();
}


QColor MapView::getGeoValueColor(long geoValue)
{
	assert(dataset);
	long minValue = dataset->minValue.integer;
	long maxValue = dataset->maxValue.integer;
	assert(minValue <= maxValue);
	if(minValue == maxValue)
		return QColor(255, 0, 0, 64);
	
	geoValue = std::clamp(geoValue, minValue, maxValue);
	double t = double(geoValue - minValue) / double(maxValue - minValue);
	
	// NOTE: 0 = red, 240 = somewhere in the middle of blue hue; we want the opposite, i.e. red high values, hence 1-t
	int hue = int((1.0-t) * 240.0);
	QColor result = QColor::fromHsv(hue, 255, 255, 64);
	return result;
}


QColor MapView::getGeoValueColor(double geoValue)
{
	assert(dataset);
	double minValue = dataset->minValue.real;
	double maxValue = dataset->maxValue.real;
	assert(minValue <= maxValue);
	if(minValue == maxValue)
		return QColor(255, 0, 0, 64);
	
	geoValue = std::clamp(geoValue, minValue, maxValue);
	double t = (geoValue - minValue) / (maxValue - minValue);
	
	// NOTE: 0 = red, 240 = somewhere in the middle of blue hue; we want the opposite, i.e. red high values, hence 1-t
	int hue = int((1.0-t) * 240.0);
	QColor result = QColor::fromHsv(hue, 255, 255, 64);
	return result;
}


void MapView::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		event->accept();
		vsMouseLeftDownPos = event->pos();
		
		switch(interactionMode)
		{
			case InteractionMode::Grid:
			{
				rubberband.setGeometry(QRect(event->pos(), QSize()));
				rubberband.show();
			} break;
			
			case InteractionMode::Cell:
			{
				if(dataset)
				{
					QPointF scenePoint       = mapToScene(event->pos());
					bool    mouseIsInsideMap = sceneRect().contains(scenePoint);
					if(mouseIsInsideMap)
					{
						GeoCoord geoCoord = toGeoCoord(scenePoint, mapSize());
						H3Index  index    = geoToH3(&geoCoord, dataset->resolution);
						assert(index != H3_INVALID_INDEX);
						
						mouseoverIndex = index;
						emit cellSelected(index);
					}
				}
			} break;
		}
	}
	else
	if(event->button() == Qt::RightButton)
	{
		event->accept();
		vsMouseRightDownPos = event->pos();
		setCursor(Qt::ClosedHandCursor);
	}
}


void MapView::mouseMoveEvent(QMouseEvent* event)
{
	if(event->buttons() == Qt::NoButton)
	{
		mouseoverIndex = H3_INVALID_INDEX;
	}
	
	if(event->buttons() & Qt::LeftButton)
	{
		event->accept();
		QPoint vsMouseMovePos = event->pos();
		
		switch(interactionMode)
		{
			case InteractionMode::Grid:
			{
				if(rubberband.isVisible())
				{
					QRectF ssRect = QRectF(mapToScene(vsMouseLeftDownPos), mapToScene(vsMouseMovePos));
					
					// Cap rubberband width to half the map (i.e. 180°; see https://github.com/uber/h3-js/issues/24)
					// NOTE: Do not use QRectF::normalized() because it moves the origin point
					// we want to keep the origin where the user clicked
					if(ssRect.width() < -sceneRect().width() * POLYFILL_WIDTH_FACTOR)
						ssRect.setWidth(-sceneRect().width() * POLYFILL_WIDTH_FACTOR);
					else
					if(ssRect.width() > sceneRect().width() * POLYFILL_WIDTH_FACTOR)
						ssRect.setWidth(sceneRect().width() * POLYFILL_WIDTH_FACTOR);
					
					QRect vsRect = mapFromScene(ssRect).boundingRect();
					rubberband.setGeometry(vsRect);
				}
			} break;
			
			case InteractionMode::Cell:
			{
				if(dataset)
				{
					QPointF scenePoint       = mapToScene(event->pos());
					bool    mouseIsInsideMap = sceneRect().contains(scenePoint);
					if(mouseIsInsideMap)
					{
						GeoCoord geoCoord = toGeoCoord(scenePoint, mapSize());
						H3Index  index    = geoToH3(&geoCoord, dataset->resolution);
						assert(index != H3_INVALID_INDEX);
						
						if(mouseoverIndex != index)
						{
							mouseoverIndex = index;
							emit cellSelected(index);
						}
					}
				}
			} break;
		}
	}
	
	if(event->buttons() & Qt::RightButton)
	{
		event->accept();
		QPoint delta = event->pos() - vsMouseRightDownPos;
		vsMouseRightDownPos = event->pos();
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
		verticalScrollBar()->setValue(verticalScrollBar()->value()     - delta.y());
	}
}


void MapView::mouseReleaseEvent(QMouseEvent* event)
{
	if(!dataset)
		return;
	
	if(event->button() == Qt::LeftButton)
	{
		event->accept();
		
		switch(interactionMode)
		{
			case InteractionMode::Grid:
			{
				if(rubberband.isVisible())
				{
					QRectF area = mapToScene(rubberband.geometry()).boundingRect();
					rubberband.setGeometry(0, 0, 0, 0);
					rubberband.hide();
					emit areaSelected(area);
				}
			} break;
			
			case InteractionMode::Cell:
			{
				mouseoverIndex = H3_INVALID_INDEX;
			} break;
		}
	}
	else
	if(event->button() == Qt::RightButton)
	{
		event->accept();
		setCursor(Qt::ArrowCursor);
	}
	else
	{
		event->ignore();
	}
}


void MapView::wheelEvent(QWheelEvent* event)
{
	// NOTE: https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView for a smooth zoom implementation
	event->accept();
	QPoint angleDelta = event->angleDelta();
	double steps = angleDelta.y() / 8.0 / 15.0; // see QWheelEvent documentation
	this->zoom(event->pos(), steps);
}
