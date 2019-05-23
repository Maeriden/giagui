#ifndef GIAGUI_MAPWINDOW_H
#define GIAGUI_MAPWINDOW_H


#include <utility>
#include <queue>
#include <QMainWindow>
#include <cpptoml.h>
#include "Dataset.hpp"
#include "MapUtils.hpp"


class QObject;
class QEvent;
class QKeyEvent;
class QCloseEvent;
class QMouseEvent;
class QWidget;
class QMenuBar;
class QToolBar;
class QLabel;
class QLineEdit;
class IntSpinBox;
class DatasetListWidget;
class DatasetControlWidget;
class MapView;

class DatasetListModel;


class MapWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	enum MapTool
	{
		Mark,
		Unmark,
		Grid,
	};
	
	struct DatasetSaveState
	{
		std::string path     = "";
		bool        modified = false;
		inline DatasetSaveState() : path(""), modified(false) {}
		inline DatasetSaveState(std::string path, bool modified) : path(std::move(path)), modified(modified) {}
	};
	
	
protected:
	// Pointer to data source
	DatasetListModel* datasets = nullptr;
	
	// Collection of user-selected cells to highlight in UI
	HashSet<H3Index> highlightedIndices;
	
	// Collection of indices used to draw the grid
	HashSet<H3Index> gridIndices;
	
	HashMap<Dataset*, DatasetSaveState> datasetSaveStates;
	
	MapTool mapTool = MapTool::Mark;
	
	
	DatasetListWidget*    datasetListWidget    = nullptr;
	DatasetControlWidget* datasetControlWidget = nullptr;
	MapView*              mapView              = nullptr;
	QLineEdit*            geoValueEditLine     = nullptr;
	IntSpinBox*           resolutionSpinbox    = nullptr;
	QLabel*               statusLabel          = nullptr;
	QToolBar*             toolBar              = nullptr;
	
public:
	explicit MapWindow(QWidget* parent = nullptr);
	
	
private:
	void addActionsToMenuBar(QMenuBar* menuBar);
	void addActionsToToolBar(QToolBar* toolBar);
	void addWidgetsToCentralWidget(QWidget* centralWidget);
	
	
protected:
	bool eventFilter(QObject* o, QEvent* e) override;
	
	void keyPressEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent* event) override;
	
	void onActionOpenFile();
	void onOpenFileDialogAccepted();
	
	void onActionSaveFile();
	void onActionSaveAs();
	void onActionSaveAll();
	void saveFileBegin(Dataset* dataset, const std::string& path);
	void onSaveFileDialogAccepted();
	void saveFileEnd(Dataset* dataset, const std::string& path);
	
	void onActionConfigureSimulation();
	void onSimulationConfigDialogAccepted();
	
	void onActionZoomOut();
	void onActionZoomIn();
	
	void onGridToolTriggered();
	void onMarkToolTriggered();
	void onUnmarkToolTriggered();
	
	void onDatasetListItemCreated(Dataset* dataset);
	void onDatasetListItemSelected(Dataset* dataset);
	void onDatasetListItemDeleted(Dataset* dataset);
	
	void onDatasetResolutionChanged(Dataset* dataset, int value);
	void onDatasetResolutionDecreased(Dataset* dataset, int oldResolution);
	void onDatasetResolutionIncreased(Dataset* dataset, int oldResolution);
	void onDatasetDefaultChanged(Dataset* dataset, GeoValue newDefaultValue);
	void onDatasetDensityChanged(Dataset* dataset, double value);
	void onDatasetValueRangeChanged(Dataset* dataset, GeoValue minValue, GeoValue maxValue);
	
	void onGeoValueEditFinished();
	
	void onMapViewMouseMove(QMouseEvent* event);
	void onMapViewCellSelected(H3Index index);
	void onMapViewAreaSelected(QRectF area);
	
	void writeHighlightedGeoValuesIntoLineEdit();
	
	Dataset* deserializeDataset(const std::string& path);
	Dataset* deserializeDataset(const std::string& path, const std::shared_ptr<cpptoml::table>& root, const std::string& datasetName);
	bool     serializeDataset(const std::string& path, Dataset* dataset);
};


#endif // GIAGUI_MAPWINDOW_H
