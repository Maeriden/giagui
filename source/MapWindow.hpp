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

struct SimulationConfig;
struct DatasetListModel;


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
	
	
	// Pointer to data source
	DatasetListModel* datasets = nullptr;
	
	// Collection of user-selected cells to highlight in UI
	HashSet<H3Index> highlightedIndices;
	
	// Collection of indices used to draw the grid
	HashSet<H3Index> gridIndices;
	
	HashMap<Dataset*, DatasetSaveState> datasetSaveStates;
	
	MapTool mapTool = MapTool::Mark;
	
	// Hack to store file to load. Used when loading a project and the user chooses to save the old project before loading 
	QString loadPath;
	
	
	DatasetListWidget*    datasetListWidget    = nullptr;
	DatasetControlWidget* datasetControlWidget = nullptr;
	MapView*              mapView              = nullptr;
	QLineEdit*            geoValueEditLine     = nullptr;
	IntSpinBox*           resolutionSpinbox    = nullptr;
	QLabel*               statusLabel          = nullptr;
	QToolBar*             toolBar              = nullptr;
	
	
	explicit MapWindow(QWidget* parent = nullptr);
	
	
	void addActionsToMenuBar(QMenuBar* menuBar);
	void addActionsToToolBar(QToolBar* toolBar);
	void addWidgetsToCentralWidget(QWidget* centralWidget);
	
	
	bool eventFilter(QObject* o, QEvent* e) override;
	
	void keyPressEvent(QKeyEvent *event) override;
	void closeEvent(QCloseEvent* event) override;
	
	void onActionOpenFile();
	void onOpenFileDialogAccepted();
	
	void onActionOpenProject();
	void onOpenProjectDialogAccepted();
	void openProject(const QString& directoryPath);
	
	void onActionSaveFile();
	void onActionSaveAs();
	void onActionSaveAll();
	void saveFileBegin(Dataset* dataset, const QString& path);
	void onSaveFileDialogAccepted();
	void saveFileEnd(Dataset* dataset, const QString& path);
	
	void onActionSaveProject();
	void onActionSaveProjectAs();
	void saveProjectBegin(const QString& path);
	void onSaveProjectDialogAccepted();
	void saveProjectEnd(const QString& path);
	
	void onActionExportSimulation();
	void exportSimulationBegin(const QString& sourcePath);
	void onExportSimulationDialogAccepted();
	void exportSimulationEnd(const QString& sourcePath, const QString& targetPath);
	
	void onActionConfigureSimulation();
	
	void onActionZoomOut();
	void onActionZoomIn();
	
	void onMarkToolTriggered();
	void onUnmarkToolTriggered();
	void onGridToolTriggered();
	
	void onDatasetListItemCreated(Dataset* dataset);
	void onDatasetListItemSelected(Dataset* current, Dataset* previous);
	void onDatasetListItemDeleted(Dataset* dataset);
	
	void onDatasetResolutionChanged(Dataset* dataset, int oldResolution);
	void onDatasetResolutionDecreased(int newResolution, int oldResolution);
	void onDatasetResolutionIncreased(int newResolution, int oldResolution);
	void onDatasetDefaultChanged(Dataset* dataset, GeoValue oldDefaultValue);
	void onDatasetDensityChanged(Dataset* dataset, double oldValue);
	void onDatasetValueRangeChanged(Dataset* dataset, GeoValue oldMinValue, GeoValue oldMaxValue);
	
	void onGeoValueEditFinished();
	
	void onMapViewMouseMove(QMouseEvent* event);
	void onMapViewCellSelected(H3Index index);
	void onMapViewAreaSelected(QRectF area);
	
	void writeHighlightedGeoValuesIntoLineEdit();
	
	bool deserializeSimulationConfig(const QString& path, SimulationConfig* config, const std::list<Dataset*>& datasets);
	bool serializeSimulationConfig(const QString& path, SimulationConfig* config);
	
	bool deserializeDataset(const QString& path, Dataset* dataset);
	bool serializeDataset(const QString& path, Dataset* dataset);
};


#endif // GIAGUI_MAPWINDOW_H
