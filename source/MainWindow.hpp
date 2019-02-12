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


// https://stackoverflow.com/questions/35178569/doublevalidator-is-not-checking-the-ranges-properly
class DoubleValidator : public QDoubleValidator
{
public:
	DoubleValidator(double min, double max, int decimals, QObject* parent = nullptr) : QDoubleValidator(min, max, decimals, parent)
	{}
	
	
	State validate(QString& input, int& pos) const override
	{
		if(input.isEmpty())
			return QValidator::Acceptable;
		return QDoubleValidator::validate(input, pos);
	}
};


#endif // MAINWINDOW_H
