#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QMouseEvent>

#include "map.hpp"
#include "MapView.hpp"


namespace Ui {
class MainWindow;
}


enum MapTool
{
	Rect,
	Edit,
};


class MainWindow : public QMainWindow
{
Q_OBJECT
	
public:
	static constexpr int DECIMAL_DIGITS = 6;

public:
	H3State h3State;
	MapTool mapTool;
	
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;
	
protected:
	bool eventFilter(QObject* o, QEvent* e) override;
	bool handleMapEventMouseMove(MapView* mapView, QMouseEvent* event);
	bool handleMapEventMouseDown(MapView* mapView, QMouseEvent* event);
	
	void keyPressEvent(QKeyEvent *event) override;
	
	void onActionTriggeredRectTool();
	void onActionTriggeredEditTool();
	
	void onResolutionChanged(int value);
	
	void onCellChangedWater();
	void onCellChangedIce();
	void onCellChangedSediment();
	
private:
	Ui::MainWindow*   ui;
	QLineEdit*        editWater;
	QLineEdit*        editIce;
	QLineEdit*        editSediment;
	QSpinBox*         resolutionSpinbox;
	QLabel*           statusLabel;
	
private:
	void setupToolbar();
};

#endif // MAINWINDOW_H
