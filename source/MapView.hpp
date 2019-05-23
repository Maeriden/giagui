#ifndef GIAGUI_MAPVIEW_H
#define GIAGUI_MAPVIEW_H

#include <QGraphicsView>
#include <QRubberBand>
#include <h3/h3api.h>

#include "Containers.hpp"
#include "MapUtils.hpp"


class QMouseEvent;
class QWheelEvent;

struct Dataset;


class MapView : public QGraphicsView
{
Q_OBJECT
	
public:
	enum InteractionMode
	{
		Cell, // Left-clicking adds/removes cells to the set of highlighted cells
		Grid, // Left-clicking shows a rubberband widget to select the area inside which to show the grid
	};
	
	
protected:
	// Data source to draw polygons
	Dataset*          dataset          = nullptr;
	
	// Data source to draw user-selected polygons
	HashSet<H3Index>* highlightIndices = nullptr;
	
	// Data source to draw polygon boundaries
	HashSet<H3Index>* gridIndices      = nullptr;
	
	// Interaction mode with map widget
	InteractionMode   interactionMode  = InteractionMode::Cell;
	
	// Last processed index while the user is moving the mouse holding left mouse button down 
	H3Index           mouseoverIndex   = H3_INVALID_INDEX;
	
	
protected:
	QPoint vsMouseLeftDownPos  = QPoint();
	QPoint vsMouseRightDownPos = QPoint();
	QRubberBand rubberband = QRubberBand(QRubberBand::Rectangle, this);
	
	QPen   datasetPen   = QPen(Qt::PenStyle::NoPen);
	QBrush datasetBrush = QBrush(Qt::BrushStyle::SolidPattern);
	
	QPen   polyfillPen   = QPen(QColor(0, 0, 0), 1.0, Qt::PenStyle::DotLine);
	QBrush polyfillBrush = QBrush(Qt::BrushStyle::NoBrush);
	
	QPen   highlightPen   = QPen(QColor(0, 0, 0), 1.0);
	QBrush highlightBrush = QBrush(Qt::BrushStyle::NoBrush);
	
	QGraphicsItem* mapGraphicsItem = nullptr;
	
	
public:
	explicit MapView(HashSet<H3Index>* highlightIndices, HashSet<H3Index>* gridIndices, QWidget* parent = nullptr);
	
	void   setDataSource(Dataset* dataset);
	void   setInteractionMode(InteractionMode mode);
	void   zoom(QPoint vsAnchor, double steps);
	void   redrawValuesRange();
	void   requestRepaint();
	QSizeF mapSize() const;
	
	
protected:
	QColor getGeoValueColor(int64_t geoValue);
	QColor getGeoValueColor(double  geoValue);
	void drawForeground(QPainter* painter, const QRectF& exposed) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	
	
signals:
	void cellSelected(H3Index index);
	void areaSelected(QRectF area);
};


#endif // GIAGUI_MAPVIEW_H
