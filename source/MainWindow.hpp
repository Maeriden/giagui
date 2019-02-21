#ifndef GIAGUI_MAINWINDOW_HPP
#define GIAGUI_MAINWINDOW_HPP

#include <QMainWindow>


class QFormLayout;
class QLabel;
class QLineEdit;
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
	
	void onDestroyedMapWindow(QObject* widget);
	
private:
	void setupUi();
	
private:
	QWidget*     centralwidget;
	QFormLayout* formLayout;
	QMenuBar*    menubar;
	
	QMenu*       menuFile;
	QAction*     actionOpen;
	QAction*     actionSave;
	QAction*     actionSaveAs;
	QAction*     actionQuit;
	
	QMenu*       menuTools;
	QAction*     actionEditor;
	
	QLabel*      labelPower;
	QLineEdit*   editPower;
	
	QLabel*      labelOutput;
	QLineEdit*   editOutput;
	
	QLabel*      labelInnerValue;
	QLineEdit*   editInnerValue;
	
	QLabel*      labelOuterValue;
	QLineEdit*   editOuterValue;
	
	QLabel*      labelOuterInput;
	QLineEdit*   editOuterInput;
};


#endif //GIAGUI_MAINWINDOW_HPP
