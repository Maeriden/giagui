#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QtWidgets/QRubberBand>
#include <h3/h3api.h>
#include "map.hpp"


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
	
protected:
	void drawForeground(QPainter* painter, const QRectF& exposed) override;

	void mousePressEvent   (QMouseEvent* event) override;
	void mouseReleaseEvent (QMouseEvent* event) override;
	void mouseMoveEvent    (QMouseEvent* event) override;
	void wheelEvent        (QWheelEvent* event) override;
	
	void zoom(QPoint anchor, int steps);
};

#endif // MAPVIEW_H
