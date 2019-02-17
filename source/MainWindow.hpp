#ifndef GIAGUI_MAINWINDOW_HPP
#define GIAGUI_MAINWINDOW_HPP

#include "ui_mainwindow.h"
#include <QMainWindow>


struct SimulationData
{
	double      power;
	std::string output;
	double      innerValue;
	double      outerValue;
	std::string outerInput;
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
	
	void onEditingFinishedMeshPower();
	void onEditingFinishedMeshOutput();
	void onEditingFinishedMeshInnerValue();
	void onEditingFinishedMeshOuterValue();
	void onEditingFinishedMeshOuterInput();
	
	void onDestroyedMapWindow(QObject* widget);
	
private:
	void setupUi();
	
	
private:
	Ui::MainWindow* ui;
};


#endif //GIAGUI_MAINWINDOW_HPP
