#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QtWidgets/QRubberBand>
#include <h3/h3api.h>
#include "map.hpp"
#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>

enum PolyfillError
{
	THRESHOLD_EXCEEDED = 1,
	MEMORY_ALLOCATION = 2,
};


class MapView : public QGraphicsView
{
Q_OBJECT
	
public:
	H3State* h3State = nullptr;
	
	QPoint vsMouseLeftDownPos  = QPoint();
	QPoint vsMouseRightDownPos = QPoint();
	QPoint vsMouseMovePos      = QPoint();
	
protected:
	QRubberBand* rubberband;
	
public:
	explicit MapView(QWidget* parent);
	
	void zoom(QPoint vsAnchor, double steps);
	
protected:
	void drawForeground(QPainter* painter, const QRectF& exposed) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	
signals:
	void polyfillFailed(PolyfillError error);
};

#endif // MAPVIEW_H
