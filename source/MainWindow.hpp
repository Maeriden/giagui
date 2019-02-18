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
	MapWindow* mapWindow = nullptr;
	
public:
	explicit MainWindow(QWidget* parent = nullptr);
	
protected:
	void closeEvent(QCloseEvent* event) override;
	
	void onActionOpenFile();
	void onActionSaveFile();
	void onActionSaveFileAs();
	void onActionOpenEditor();
	
	void onTextEditedMeshPower(const QString& text);
	void onTextEditedMeshOutput(const QString& text);
	void onTextEditedMeshInnerValue(const QString& text);
	void onTextEditedMeshOuterValue(const QString& text);
	void onTextEditedMeshOuterInput(const QString& text);

#if ENABLE_BUTTONS_MESH_IO
	void onClickedMeshOutput(bool);
	void onClickedMeshOuterInput(bool);
#endif // ENABLE_BUTTONS_MESH_IO
	
	void onDestroyedMapWindow(QObject* widget);

protected:
	QString textMeshPower;
	QString textMeshOutput;
	QString textMeshInnerValue;
	QString textMeshOuterValue;
	QString textMeshOuterInput;
	
private:
	void setupUi();
	
private:
	Ui::MainWindow* ui;
};


#endif //GIAGUI_MAINWINDOW_HPP
