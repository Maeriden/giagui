#ifndef GIAGUI_MAINWINDOW_HPP
#define GIAGUI_MAINWINDOW_HPP

#include "ui_mainwindow.h"
#include <QMainWindow>


struct SimulationData
{
	double      power;
	const char* output;
	double      innerValue;
	double      outerValue;
	const char* outerInput;
};


class MapWindow;

class MainWindow : public QMainWindow
{
Q_OBJECT
	MapWindow*      mapWindow;
	SimulationData* simulationData;
	QString         exportPath;
	
public:
	explicit MainWindow(QWidget* parent = nullptr);
	
	
protected:
	void onActionOpenFile();
	void onActionSaveFile();
	void onActionSaveFileAs();
	void onActionOpenEditor();
	
	void onEditingFinishedPower();
	void onEditingFinishedOutput();
	void onEditingFinishedInnerValue();
	void onEditingFinishedOuterValue();
	void onEditingFinishedOuterInput();
	
	
private:
	void setupUi();
	
	
private:
	Ui::MainWindow* ui;
};


#endif //GIAGUI_MAINWINDOW_HPP
