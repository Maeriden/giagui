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


class IntSpinBox;


class MainWindow : public QMainWindow
{
Q_OBJECT
	
public:
	static constexpr int DECIMAL_DIGITS = 6;

public:
	H3State* h3State;
	MapTool  mapTool;
	QString  exportPath;
	
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow() override;
	
protected:
	bool eventFilter(QObject* o, QEvent* e) override;
	
	void keyPressEvent(QKeyEvent *event) override;
	
	void onActionOpenFile();
	void onActionSaveFile();
	void onActionSaveFileAs();
	void onActionZoomOut();
	void onActionZoomIn();
	void onActionRectTool();
	void onActionEditTool();
	
	void onCellChangedWater();
	void onCellChangedIce();
	void onCellChangedSediment();
	void onCellChangedDensity();
	
	void onResolutionChanged(int value);
	void onResolutionChangedDialogFinished(int dialogResult);
	
	bool handleMapEventMousePress(MapView* mapView, QMouseEvent* event);
	bool handleMapEventMouseMove(MapView* mapView, QMouseEvent* event);
	bool handleMapEventMouseRelease(MapView* mapView, QMouseEvent* event);
	
	void setupToolbar();
	void highlightCell(H3Index index);
	void setAllLineEditEnabled(bool enabled);
	void clearAllLineEditNoSignal();
	
private:
	Ui::MainWindow* ui;
	QLineEdit*      editWater;
	QLineEdit*      editIce;
	QLineEdit*      editSediment;
	QLineEdit*      editDensity;
	IntSpinBox*     resolutionSpinbox;
	QLabel*         statusLabel;
	
};


#endif // MAINWINDOW_H
