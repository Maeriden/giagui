#include "MapWindow.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsSvgItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>

#include "MapView.hpp"
#include "GeoValueValidator.hpp"
#include "DatasetListWidget.hpp"
#include "DatasetControlWidget.hpp"
#include "models/DatasetListModel.hpp"
#include "dialogs/SimulationConfigDialog.hpp"
#include "dialogs/DatasetCreateDialog.hpp"


static SimulationConfig globalSimulationConfig;




MapWindow::MapWindow(QWidget* parent) : QMainWindow(parent)
{
	datasets = new DatasetListModel(this);
	
#if !DISABLE_CREATE_INNER_AND_OUTER_DATASETS_AT_STARTUP
	datasets->appendItem(new Dataset("inner", false, true));
	datasets->appendItem(new Dataset("outer", false, true));
#endif
	
	resize(1000, 750);
	
	QSplitter* hSplitter = new QSplitter(Qt::Orientation::Horizontal, this);
	setCentralWidget(hSplitter);
	addWidgetsToCentralWidget(hSplitter);
	hSplitter->setStretchFactor(0, 0);
	hSplitter->setStretchFactor(1, 1);
	
	
	QMenuBar* menuBar = new QMenuBar(this);
	menuBar->setGeometry(QRect(0, 0, 1000, 20));
	setMenuBar(menuBar);
	addActionsToMenuBar(menuBar);
	
	
	QToolBar* toolBar = new QToolBar(this);
	toolBar->setMovable(false);
	toolBar->setAllowedAreas(Qt::TopToolBarArea);
	toolBar->setIconSize(QSize(16, 16));
	addToolBar(Qt::TopToolBarArea, toolBar);
	addActionsToToolBar(toolBar);
	this->toolBar = toolBar;
	
	
	QStatusBar* statusBar = new QStatusBar(this);
	statusBar->setSizeGripEnabled(true);
	setStatusBar(statusBar);
	
	
	statusLabel = new QLabel();
	statusBar->addPermanentWidget(statusLabel);
}


void MapWindow::addWidgetsToCentralWidget(QWidget* centralWidget)
{
	{
		QSplitter* group = new QSplitter(Qt::Orientation::Vertical, centralWidget);
		
		datasetListWidget = new DatasetListWidget(datasets, group);
		QObject::connect(datasetListWidget, &DatasetListWidget::itemCreated, this, &MapWindow::onDatasetListItemCreated);
		QObject::connect(datasetListWidget, &DatasetListWidget::itemSelected, this,
		                 &MapWindow::onDatasetListItemSelected);
		QObject::connect(datasetListWidget, &DatasetListWidget::itemDeleted, this, &MapWindow::onDatasetListItemDeleted);
		
		datasetControlWidget = new DatasetControlWidget(group);
		QObject::connect(datasetControlWidget, &DatasetControlWidget::resolutionChanged, this, &MapWindow::onDatasetResolutionChanged);
		QObject::connect(datasetControlWidget, &DatasetControlWidget::defaultChanged,    this, &MapWindow::onDatasetDefaultChanged);
		QObject::connect(datasetControlWidget, &DatasetControlWidget::densityChanged,    this, &MapWindow::onDatasetDensityChanged);
		QObject::connect(datasetControlWidget, &DatasetControlWidget::valueRangeChanged, this, &MapWindow::onDatasetValueRangeChanged);
		
		group->setStretchFactor(0, 0);
		group->setStretchFactor(1, 1);
	}
	
	
	mapView = new MapView(&highlightedIndices, &gridIndices, centralWidget);
	mapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mapView->setDragMode(QGraphicsView::NoDrag);
	mapView->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	mapView->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
	
	mapView->viewport()->installEventFilter(this); // NOTE: https://stackoverflow.com/questions/2445997/qgraphicsview-and-eventfilter
	QObject::connect(mapView, &MapView::cellSelected, this, &MapWindow::onMapViewCellSelected);
	QObject::connect(mapView, &MapView::areaSelected, this, &MapWindow::onMapViewAreaSelected);
}


void MapWindow::addActionsToMenuBar(QMenuBar* menuBar)
{
	QMenu* menuFile = new QMenu(tr("File"), this);
	{
//		QAction* action = new QAction(this);
//		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-open")));
//		action->setText(tr("Open..."));
//		action->setShortcuts(QKeySequence::StandardKey::Open);
//		action->setStatusTip(tr("Open file"));
//		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionOpenFile);
//		menuFile->addAction(action);
//	} {
//		QAction* action = new QAction(this);
//		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save")));
//		action->setText(tr("Save"));
//		action->setShortcuts(QKeySequence::StandardKey::Save);
//		action->setStatusTip(tr("Save file"));
//		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveFile);
//		menuFile->addAction(action);
//	} {
//		QAction* action = new QAction(this);
//		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save-as")));
//		action->setText(tr("Save As..."));
//		action->setShortcuts(QKeySequence::StandardKey::SaveAs);
//		action->setStatusTip(tr("Save file as"));
//		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveAs);
//		menuFile->addAction(action);
	} {
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-open")));
		action->setText(tr("Open Project..."));
		action->setShortcuts(QKeySequence::StandardKey::Open);
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionOpenProject);
		menuFile->addAction(action);
	} {
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save")));
		action->setText(tr("Save Project"));
		action->setShortcuts(QKeySequence::StandardKey::Save);
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveProject);
		menuFile->addAction(action);
	} {
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save-as")));
		action->setText(tr("Save Project As..."));
		action->setShortcuts(QKeySequence::StandardKey::SaveAs);
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveProjectAs);
		menuFile->addAction(action);
	}
	menuFile->addSeparator();
	{
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-close")));
		action->setText(tr("Close"));
		action->setShortcuts(QKeySequence::StandardKey::Close);
		action->setStatusTip(tr("Close window"));
		QObject::connect(action, &QAction::triggered, this, &MapWindow::close);
		menuFile->addAction(action);
	}
	
	
	QMenu* menuTools = new QMenu(tr("Tools"), this);
	{
		QAction* action = new QAction(this);
		action->setText(tr("Configure Simulation..."));
		action->setStatusTip(tr("Create a configuration file for a simulation"));
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionConfigureSimulation);
		menuTools->addAction(action);
	}
	
	
	menuBar->addAction(menuFile->menuAction());
	menuBar->addAction(menuTools->menuAction());
}


void MapWindow::addActionsToToolBar(QToolBar* toolBar)
{
	{	QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* actionZoomOut = new QAction();
		actionZoomOut->setIcon(QIcon::fromTheme(QString::fromUtf8("zoom-out")));
		actionZoomOut->setText(tr("Zoom Out"));
		actionZoomOut->setActionGroup(actionGroup);
		actionZoomOut->setShortcuts(QKeySequence::StandardKey::ZoomOut);
		actionZoomOut->setStatusTip(tr("Zoom out"));
		QObject::connect(actionZoomOut, &QAction::triggered, this, &MapWindow::onActionZoomOut);
		
		QAction* actionZoomIn = new QAction();
		actionZoomIn->setIcon(QIcon::fromTheme(QString::fromUtf8("zoom-in")));
		actionZoomIn->setText(tr("Zoom In"));
		actionZoomIn->setActionGroup(actionGroup);
		actionZoomIn->setShortcuts(QKeySequence::StandardKey::ZoomIn);
		actionZoomIn->setStatusTip(tr("Zoom in"));
		QObject::connect(actionZoomIn, &QAction::triggered, this, &MapWindow::onActionZoomIn);
		
		toolBar->addActions(actionGroup->actions());
	}
	
	toolBar->addSeparator();
	
	{	QActionGroup* actionGroup = new QActionGroup(this);
	
		QAction* actionMark = new QAction();
		actionMark->setIcon(QIcon(QString::fromUtf8(":/images/icon-cell-mark.svg")));
		actionMark->setText(tr("Mark"));
		actionMark->setActionGroup(actionGroup);
		actionMark->setStatusTip(tr("Click on the map to mark cells for editing"));
		actionMark->setCheckable(true);
		QObject::connect(actionMark, &QAction::triggered, this, &MapWindow::onMarkToolTriggered);
		
#if ENABLE_CELL_SELECTION_TOOLS
		QAction* actionUnmark = new QAction();
		actionUnmark->setIcon(QIcon(QString::fromUtf8(":/images/icon-cell-unmark.svg")));
		actionUnmark->setText(tr("Unmark"));
		actionUnmark->setActionGroup(actionGroup);
		actionUnmark->setStatusTip(tr("Click on the map to unmark cells for editing"));
		actionUnmark->setCheckable(true);
		QObject::connect(actionUnmark, &QAction::triggered, this, &MapWindow::onUnmarkToolTriggered);
#endif
		
		QAction* actionGrid = new QAction();
		actionGrid->setIcon(QIcon(QString::fromUtf8(":/images/icon-grid.svg")));
		actionGrid->setText(tr("Grid"));
		actionGrid->setActionGroup(actionGroup);
		actionGrid->setStatusTip(tr("Select an area on the map to show the grid"));
		actionGrid->setCheckable(true);
		QObject::connect(actionGrid, &QAction::triggered, this, &MapWindow::onGridToolTriggered);
		
		toolBar->addActions(actionGroup->actions());
		actionMark->setChecked(true);
//		actionGrid->setEnabled(false);
	}
	
	toolBar->addSeparator();
	
	{
		QWidget*     group  = new QWidget(this);
		QHBoxLayout* layout = new QHBoxLayout(group);
		layout->setSpacing(6);
		layout->setContentsMargins(5, 0, 5, 0);
		toolBar->addWidget(group);
		
		QLabel* label = new QLabel(tr("Value"), group);
		layout->addWidget(label);
		
		geoValueEditLine = new QLineEdit(group);
		geoValueEditLine->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		geoValueEditLine->setMinimumWidth(80);
		geoValueEditLine->setMaximumWidth(100);
		geoValueEditLine->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		geoValueEditLine->setEnabled(false);
//		geoValueEditLine->setPlaceholderText(tr("N/A"));
		geoValueEditLine->setValidator(new GeoValueValidator(-DOUBLE_MAX, DOUBLE_MAX, UI_DOUBLE_PRECISION));
		QObject::connect(geoValueEditLine, &QLineEdit::editingFinished, this, &MapWindow::onGeoValueEditFinished);
		layout->addWidget(geoValueEditLine);
	}
}


bool MapWindow::eventFilter(QObject* object, QEvent* event)
{
	assert(object == mapView->viewport()); // We want to filter map events only
	
	if(event->type() == QEvent::MouseMove)
	{
		onMapViewMouseMove((QMouseEvent*)event);
	}
	if(event->type() == QEvent::MouseButtonPress && ((QMouseEvent*)event)->button() == Qt::LeftButton)
	{
		onMapViewMouseMove((QMouseEvent*)event);
	}
	if(event->type() == QEvent::MouseButtonRelease && ((QMouseEvent*)event)->button() == Qt::LeftButton)
	{
		onMapViewMouseMove((QMouseEvent*)event);
	}
	return false; // Do not block event
}


void MapWindow::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		event->accept();
		highlightedIndices.clear();
		writeHighlightedGeoValuesIntoLineEdit();
		mapView->requestRepaint();
	}
	else
	{
		event->ignore();
	}
}


void MapWindow::closeEvent(QCloseEvent* event)
{
	bool confirmed = !isWindowModified();
//	for(auto& [dataset, saveState] : datasetSaveStates)
//	{
//		if(saveState.modified)
//		{
//			confirmed = false;
//			break;
//		}
//	}
	
	if(!confirmed)
	{
		QString question = tr("There are unsaved changes. Close anyway?");
		int reply = QMessageBox::question(this, QString(), question);
		confirmed = reply == QMessageBox::Yes;
	}
	
	if(confirmed)
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


void MapWindow::onActionOpenFile()
{
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setWindowTitle(tr("Open File"));
	dialog->setNameFilter(tr("TOML+H3 (*.h3);;All Files (*)"));
	dialog->setAcceptMode(QFileDialog::AcceptOpen);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(dialog, &QFileDialog::accepted, this, &MapWindow::onOpenFileDialogAccepted);
	dialog->open();
}


void MapWindow::onOpenFileDialogAccepted()
{
	QFileDialog* fileDialog = static_cast<QFileDialog*>(sender());
	
	try
	{
		for(const QString& path : fileDialog->selectedFiles())
		{
			assert(path.size() > 0);
			
			Dataset* dataset = new Dataset();
			bool success = deserializeDataset(path, dataset);
			if(success)
			{
				if(datasets->appendItem(dataset))
				{
					datasetSaveStates[dataset].path     = path.toStdString();
					datasetSaveStates[dataset].modified = false;
				}
				else
				{
					QString datasetName = QString::fromStdString(dataset->id);
					
					QMessageBox* dialog = new QMessageBox(this);
					dialog->setWindowTitle(tr("Error"));
					dialog->setText(tr("Error while adding %1 to the datasets list").arg(datasetName));
					dialog->setAttribute(Qt::WA_DeleteOnClose);
					dialog->open();
					
					delete dataset;
				}
			}
			else
			{
				// TODO: Error dialog?
			}
		}
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(__FILE__ ":" STR(__LINE__) " Memory allocation error");
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
	}
}


void MapWindow::onActionOpenProject()
{
	// TODO: Show unsaved changes dialog
	
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setWindowTitle(tr("Open Project"));
	dialog->setAcceptMode(QFileDialog::AcceptOpen);
	dialog->setFileMode(QFileDialog::FileMode::Directory);
	dialog->setOption(QFileDialog::Option::ShowDirsOnly, true);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(dialog, &QFileDialog::accepted, this, &MapWindow::onOpenProjectDialogAccepted);
	dialog->open();
}


void MapWindow::onOpenProjectDialogAccepted()
{
	QFileDialog* dialog = static_cast<QFileDialog*>(sender());
	assert(dialog->selectedFiles().size() == 1);
	assert(dialog->selectedFiles().first().size() > 0);
	QString directoryPath = dialog->selectedFiles().first();
	
	
	QDir    directory = QDir(directoryPath);
	QString filePath;
	bool    success = true;
	
	
	std::list<Dataset*> datasetList;
	try
	{
		for(QFileInfo& fileInfo : directory.entryInfoList(QDir::Filter::Files, QDir::SortFlag::NoSort))
		{
			if(fileInfo.suffix() != "h3")
				continue;
			
			filePath = fileInfo.filePath();
			Dataset* dataset = new Dataset();
			success = deserializeDataset(filePath, dataset);
			if(success)
				datasetList.push_back(dataset);
			else
				break;
		}
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(__FILE__ ":" STR(__LINE__) " Memory allocation error");
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
	}
	
	
	SimulationConfig config;
	try
	{
		if(success)
		{
			filePath = directory.filePath("_project.toml");
			success  = deserializeSimulationConfig(filePath, &config, datasetList);
		}
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(__FILE__ ":" STR(__LINE__) " Memory allocation error");
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
	}
	
	
	if(success)
	{
		globalSimulationConfig = std::move(config);
		datasets->reset(std::move(datasetList));
		
		setWindowFilePath(directoryPath);
		setWindowModified(false);
	}
	else
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Error while saving %1").arg(filePath));
		dialog->setInformativeText(tr("Operation aborted, data may be corrupted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		
		for(Dataset* dataset : datasetList)
			delete dataset;
	}
}


void MapWindow::onActionSaveFile()
{
	Dataset* dataset = datasetListWidget->selection();
	if(!dataset)
		return;
	DatasetSaveState& saveState = datasetSaveStates[dataset];
	QString           path = QString::fromStdString(saveState.path);
	saveFileBegin(dataset, path);
}


void MapWindow::onActionSaveAs()
{
	Dataset* dataset = datasetListWidget->selection();
	if(!dataset)
		return;
	saveFileBegin(dataset, "");
}


void MapWindow::onActionSaveAll()
{
	for(Dataset* dataset : *datasets)
	{
		DatasetSaveState& saveState = datasetSaveStates[dataset];
		QString           path = QString::fromStdString(saveState.path);
		saveFileBegin(dataset, path);
	}
}


void MapWindow::saveFileBegin(Dataset* dataset, const QString& path)
{
	assert(dataset);
	if(path.isEmpty())
	{
		QString filename = QString::fromStdString(dataset->id) + ".h3";
		
		QFileDialog* dialog = new QFileDialog(this);
		dialog->setWindowTitle(tr("Save File"));
		dialog->setNameFilter(tr("TOML+H3 (*.h3);;All Files (*)"));
		dialog->setAcceptMode(QFileDialog::AcceptSave);
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->setProperty("dataset", QVariant::fromValue(dataset));
		dialog->selectFile(filename);
		dialog->setDefaultSuffix("h3");
		QObject::connect(dialog, &QFileDialog::accepted, this, &MapWindow::onSaveFileDialogAccepted);
		dialog->open();
	}
	else
	{
		saveFileEnd(dataset, path);
	}
}


void MapWindow::onSaveFileDialogAccepted()
{
	QFileDialog* dialog = static_cast<QFileDialog*>(sender());
	assert(dialog->selectedFiles().size() == 1);
	assert(dialog->selectedFiles().first().size() > 0);
	QString path = dialog->selectedFiles().first();
	
	Dataset* dataset = dialog->property("dataset").value<Dataset*>();
	assert(dataset);
	
	saveFileEnd(dataset, path);
}


void MapWindow::saveFileEnd(Dataset* dataset, const QString& path)
{
	bool success = serializeDataset(path, dataset);
	if(success)
	{
		DatasetSaveState& saveState = datasetSaveStates[dataset];
		saveState.path     = path.toStdString();
		saveState.modified = false;
	}
}


void MapWindow::onActionSaveProject()
{
	QString path = windowFilePath();
	saveProjectBegin(path);
}


void MapWindow::onActionSaveProjectAs()
{
	QString path = "";
	saveProjectBegin(path);
}


void MapWindow::saveProjectBegin(const QString& directoryPath)
{
	if(directoryPath.isEmpty())
	{
		QFileDialog* dialog = new QFileDialog(this);
		dialog->setWindowTitle(tr("Save Project"));
		dialog->setAcceptMode(QFileDialog::AcceptOpen);
		dialog->setFileMode(QFileDialog::FileMode::Directory);
		dialog->setOption(QFileDialog::Option::ShowDirsOnly, true);
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		QObject::connect(dialog, &QFileDialog::accepted, this, &MapWindow::onSaveProjectDialogAccepted);
		dialog->open();
	}
	else
	{
		saveProjectEnd(directoryPath);
	}
}


void MapWindow::onSaveProjectDialogAccepted()
{
	QFileDialog* dialog = static_cast<QFileDialog*>(sender());
	assert(dialog->selectedFiles().size() == 1);
	assert(dialog->selectedFiles().first().size() > 0);
	QString directoryPath = dialog->selectedFiles().first();
	
	saveProjectEnd(directoryPath);
}


void MapWindow::saveProjectEnd(const QString& directoryPath)
{
	assert(directoryPath.size() > 0);
	
	QDir    directory = QDir(directoryPath);
	QString filePath;
	bool    success = true;
	
	for(Dataset* dataset : *datasets)
	{
		QString filename = QString::fromStdString(dataset->id) + ".h3";
		filePath = directory.filePath(filename);
		success = serializeDataset(filePath, dataset);
		if(!success)
			break;
		
		datasetSaveStates[dataset].path     = filePath.toStdString();
		datasetSaveStates[dataset].modified = false;
	}
	
	if(success)
	{
		SimulationConfig* config = &globalSimulationConfig;
		filePath = directory.filePath("_project.toml");
		success = serializeSimulationConfig(filePath, config);
	}
	
	if(success)
	{
		setWindowFilePath(directoryPath);
		setWindowModified(false);
	}
	else
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Error while saving %1").arg(filePath));
		dialog->setInformativeText(tr("Operation aborted, data may be corrupted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
	}
}


void MapWindow::onActionConfigureSimulation()
{
	SimulationConfig* config = &globalSimulationConfig;
	SimulationConfigDialog* simulationWindow = new SimulationConfigDialog(datasets, config, this);
	simulationWindow->setAttribute(Qt::WA_DeleteOnClose);
	simulationWindow->open();
}


void MapWindow::onActionZoomOut()
{
	QPoint vsAnchor = mapView->viewport()->rect().center();
	mapView->zoom(vsAnchor, -1.0);
}


void MapWindow::onActionZoomIn()
{
	QPoint vsAnchor = mapView->viewport()->rect().center();
	mapView->zoom(vsAnchor, +1.0);
}


void MapWindow::onMarkToolTriggered()
{
	mapTool = MapTool::Mark;
	mapView->setInteractionMode(MapView::InteractionMode::Cell);
}


void MapWindow::onUnmarkToolTriggered()
{
	mapTool = MapTool::Unmark;
	mapView->setInteractionMode(MapView::InteractionMode::Cell);
}


void MapWindow::onGridToolTriggered()
{
	mapTool = MapTool::Grid;
	mapView->setInteractionMode(MapView::InteractionMode::Grid);
}


void MapWindow::onDatasetListItemCreated(Dataset* dataset)
{
	assert(dataset);
	datasetSaveStates.insert({dataset, DatasetSaveState()});
}


void MapWindow::onDatasetListItemSelected(Dataset* currentDataset, Dataset* previousDataset)
{
	
	if(currentDataset)
	{
		highlightedIndices.clear();
		gridIndices.clear();
		
		geoValueEditLine->setPlaceholderText(QString::fromStdString(currentDataset->measureUnit));
		writeHighlightedGeoValuesIntoLineEdit();
		if(QApplication::focusWidget() == geoValueEditLine)
			geoValueEditLine->selectAll();
	}
	else
	{
		highlightedIndices.clear();
		gridIndices.clear();
		
		geoValueEditLine->blockSignals(true);
		geoValueEditLine->clear();
		geoValueEditLine->setPlaceholderText("");
		geoValueEditLine->setEnabled(false);
		geoValueEditLine->blockSignals(false);
	}
	
	datasetControlWidget->setDataSource(currentDataset);
	mapView->setDataSource(currentDataset);
}


void MapWindow::onDatasetListItemDeleted(Dataset* dataset)
{
	datasetSaveStates.erase(dataset);
	
	for(SimulationConfig::Load::HistoryEntry& entry : globalSimulationConfig.load.history)
		if(entry.datasets.count(dataset) > 0)
			entry.datasets.erase(dataset);
}


void MapWindow::onDatasetResolutionChanged(Dataset* dataset, int oldResolution)
{
	assert(IS_VALID_RESOLUTION(dataset->resolution));
	
	// TODO: Profile this to see if it's worth doing the operation in another thread
	try
	{
		if(dataset->resolution < oldResolution)
		{
			// NOTE: We keep this line for future reference just in case Qt feels like being a dick
//			QApplication::postEvent(this->resolutionSpinbox, new QMouseEvent(QEvent::MouseButtonRelease, QPoint( 0, 0 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) );
			onDatasetResolutionDecreased(dataset->resolution, oldResolution);
		}
		else
		{
			onDatasetResolutionIncreased(dataset->resolution, oldResolution);
		}
		
		mapView->requestRepaint();
		setWindowModified(true);
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox::critical(this, tr("Memory allocation error"), tr("Not enough memory to store new values"));
		// TODO: Let application crash? Data consistency is not enforced anyway
	}
}


void MapWindow::onDatasetResolutionDecreased(int newResolution, int oldResolution)
{
	HashSet<H3Index> hlIndices;
	for(H3Index childIndex : highlightedIndices)
	{
		H3Index parentIndex = h3ToParent(childIndex, newResolution);
		assert(parentIndex != H3_INVALID_INDEX);
		hlIndices.insert(parentIndex);
	}
	this->highlightedIndices = std::move(hlIndices);
	
	
	HashSet<H3Index> gIndices;
	for(H3Index childIndex : gridIndices)
	{
		H3Index parentIndex = h3ToParent(childIndex, newResolution);
		assert(parentIndex != H3_INVALID_INDEX);
		gIndices.insert(parentIndex);
	}
	gridIndices = std::move(gIndices);
}


void MapWindow::onDatasetResolutionIncreased(int newResolution, int oldResolution)
{
	uint64_t childrenBufferLength = h3MaxChildrenCount(oldResolution, newResolution);
	H3Index* childrenBuffer       = new H3Index[childrenBufferLength];
	
	HashSet<H3Index> newHighlightIndices;
	for(H3Index parentIndex : highlightedIndices)
	{
		h3ToChildren(parentIndex, newResolution, childrenBuffer);
		newHighlightIndices.insert(childrenBuffer, childrenBuffer + childrenBufferLength);
	}
	newHighlightIndices.erase(H3_INVALID_INDEX);
	highlightedIndices = std::move(newHighlightIndices);
	
	
	HashSet<H3Index> newGridIndices;
	for(H3Index parentIndex : gridIndices)
	{
		h3ToChildren(parentIndex, newResolution, childrenBuffer);
		newGridIndices.insert(childrenBuffer, childrenBuffer + childrenBufferLength);
	}
	newGridIndices.erase(H3_INVALID_INDEX);
	gridIndices = std::move(newGridIndices);
	
	delete[] childrenBuffer;
}


void MapWindow::onDatasetDefaultChanged(Dataset* dataset, GeoValue oldDefaultValue)
{
	setWindowModified(true);
}


void MapWindow::onDatasetDensityChanged(Dataset* dataset, double oldValue)
{
	setWindowModified(true);
}


void MapWindow::onDatasetValueRangeChanged(Dataset* dataset, GeoValue oldMinValue, GeoValue oldMaxValue)
{
	mapView->redrawValuesRange();
	setWindowModified(true);
}


void MapWindow::onGeoValueEditFinished()
{
	Dataset* dataset = datasetListWidget->selection();
	assert(dataset);
	
	size_t affectedCellsCount = 0;
	if(geoValueEditLine->text().isEmpty())
	{
		for(H3Index hlIndex : highlightedIndices)
		{
			affectedCellsCount += dataset->removeGeoValue(hlIndex);
		}
	}
	else
	if(geoValueEditLine->text() == QString::fromUtf8(UI_MULTIPLE_GEOVALUES_STRING))
	{
		// Ignore this input
	}
	else
	{
		bool     isValidNumber;
		GeoValue geoValue = toGeoValue(geoValueEditLine->text(), dataset->isInteger, &isValidNumber);
		if(isValidNumber)
		{
			try
			{
				for(H3Index hlIndex : highlightedIndices)
				{
					affectedCellsCount += dataset->updateGeoValue(hlIndex, geoValue);
				}
			}
			catch(std::bad_alloc& ex)
			{
				QMessageBox::critical(this, tr("Memory allocation error"), tr("Not enough memory to store new values"));
			}
		}
	}
	
	if(affectedCellsCount > 0)
	{
//		DatasetSaveState& saveState = datasetSaveStates[dataset];
//		saveState.modified = true;
//		setWindowModified(saveState.modified);
		
		setWindowModified(true);
		mapView->requestRepaint();
	}
}


void MapWindow::writeHighlightedGeoValuesIntoLineEdit()
{
	if(highlightedIndices.size() > 0)
	{
		Dataset* dataset = datasetListWidget->selection();
		assert(dataset);
		
		bool     haveAnyValue  = false;
		bool     haveSameValue = true;
		bool     emptyCellsExist = false;
		GeoValue commonGeoValue = {0};
		
		for(H3Index index : highlightedIndices)
		{
			GeoValue geoValue = {0};
			if(dataset->findGeoValue(index, &geoValue))
			{
				if(!haveAnyValue)
				{
					haveAnyValue = true;
					if(emptyCellsExist)
					{
						haveSameValue = false;
						break;
					}
					
					commonGeoValue = geoValue;
					continue;
				}
				
				haveSameValue = dataset->geoValuesAreEqual(geoValue, commonGeoValue); 
			}
			else
			{
				emptyCellsExist = true;
				haveSameValue = !haveAnyValue;
			}
			
			if(!haveSameValue)
				break;
		}
		
		QString textGeoValue;
		if(haveAnyValue)
		{
			if(haveSameValue)
			{
				if(dataset->isInteger)
					textGeoValue = QString::number(commonGeoValue.integer);
				else
					textGeoValue = QString::number(commonGeoValue.real, 'f', UI_DOUBLE_PRECISION);
			}
			else
			{
				textGeoValue = QString::fromUtf8(UI_MULTIPLE_GEOVALUES_STRING);
			}
		}
		
		geoValueEditLine->blockSignals(true);
		geoValueEditLine->setEnabled(true);
		geoValueEditLine->setText(textGeoValue);
		geoValueEditLine->blockSignals(false);
	}
	else
	{
		geoValueEditLine->blockSignals(true);
		geoValueEditLine->clear();
		geoValueEditLine->setEnabled(false);
		geoValueEditLine->blockSignals(false);
	}
}


void MapWindow::onMapViewMouseMove(QMouseEvent* event)
{
	assert(datasets);
	
	QPointF scenePoint       = mapView->mapToScene(event->pos());
	bool    mouseIsInsideMap = mapView->sceneRect().contains(scenePoint); 
	if(mouseIsInsideMap)
	{
		// Update mouse coorinates in status bar
		GeoCoord mouseGeoCoord = toGeoCoord(scenePoint, mapView->sceneRect().size());
		QString message = QString::fromUtf8("%1 : %2")
			.arg(radsToDegs(mouseGeoCoord.lon))
			.arg(radsToDegs(mouseGeoCoord.lat));
		statusLabel->setText(message);
	}
}


void MapWindow::onMapViewCellSelected(H3Index index)
{
	assert(datasets);
	assert(index != H3_INVALID_INDEX);
	
	Dataset* dataset = datasetListWidget->selection();
	if(!dataset)
		return;
	
#if ENABLE_CELL_SELECTION_TOOLS
	if(mapTool == MapTool::Mark)
		highlightedIndices.insert(index);
	else if(mapTool == MapTool::Unmark)
		highlightedIndices.erase(index);
	
#else
	Qt::KeyboardModifiers ctrl = QApplication::keyboardModifiers() & Qt::ControlModifier;
	if(ctrl)
	{
		if(highlightedIndices.count(index) == 0)
			highlightedIndices.insert(index);
		else
			highlightedIndices.erase(index);
	}
	else
	{
		highlightedIndices.clear();
		highlightedIndices.insert(index);
	}
	
#endif
	
	writeHighlightedGeoValuesIntoLineEdit();
	if(QApplication::focusWidget() == geoValueEditLine)
		geoValueEditLine->selectAll();
	mapView->requestRepaint();
}


void MapWindow::onMapViewAreaSelected(QRectF area)
{
	assert(datasets);
	
	Dataset* dataset = datasetListWidget->selection();
	if(!dataset)
		return;
	
	GeoCoord geoCorners[4];
	toGeoCoord(area, mapView->sceneRect().size(), geoCorners);
	
	GeoPolygon geoPolygon        = {};
	geoPolygon.geofence.numVerts = 4;
	geoPolygon.geofence.verts    = geoCorners;
	uint64_t polyfillIndicesCount = maxPolyfillSize(&geoPolygon, dataset->resolution);
	
	if(polyfillIndicesCount < POLYFILL_INDEX_THRESHOLD)
	{
		try
		{
			H3Index* buffer = new H3Index[polyfillIndicesCount];
			polyfill(&geoPolygon, dataset->resolution, buffer);
			gridIndices.clear();
			gridIndices.insert(buffer, buffer+polyfillIndicesCount);
			gridIndices.erase(H3_INVALID_INDEX);
			delete[] buffer;
		}
		catch(std::bad_alloc& ex)
		{
			QMessageBox::critical(this, tr("Error"), tr("Not enough memory to polyfill the selected area"));
		}
		
		mapView->requestRepaint();
	}
	else
	{
		QString message      = tr("Area too large. Operation aborted to prevent application freeze");
		int     milliseconds = 10000;
		statusBar()->showMessage(message, milliseconds);
	}
}


bool MapWindow::deserializeSimulationConfig(const QString& path, SimulationConfig* config, const std::list<Dataset*>& datasets)
{
	std::ifstream stream(path.toStdString());
	if(!stream.is_open())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Cannot open %1 for reading").arg(path));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		return false;
	}
	
	
	std::shared_ptr<cpptoml::table> root = nullptr;
	try
	{
		cpptoml::parser parser(stream);
		root = parser.parse();
		stream.close();
	}
	catch(cpptoml::parse_exception& ex)
	{
		stream.close();
		
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Error while parsing '%1'").arg(path));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		return false;
	}
	
	
	struct MatchByName {
		const std::string& needle;
		explicit MatchByName(const std::string& needle) : needle(needle) {}
		bool operator()(const Dataset* x) { return needle == x->id; }
	};
	
	
	if(root->contains_qualified("mesh.inner.value"))
	{
		config->mesh.inner.value = *root->get_qualified_as<int>("mesh.inner.value");
	}
	
	
	if(root->contains_qualified("mesh.inner.input"))
	{
		std::string datasetName = *root->get_qualified_as<std::string>("mesh.inner.input");
		auto iter = std::find_if(datasets.begin(), datasets.end(), MatchByName(datasetName));
		// TODO: Show warning of nonexistent file reference?
		config->mesh.inner.input = iter != datasets.end() ? *iter : nullptr;
	}
	
	
	if(root->contains_qualified("mesh.outer.value"))
	{
		config->mesh.outer.value = *root->get_qualified_as<int>("mesh.outer.value");
	}
	
	
	if(root->contains_qualified("mesh.outer.input"))
	{
		std::string datasetName = *root->get_qualified_as<std::string>("mesh.outer.input");
		auto iter = std::find_if(datasets.begin(), datasets.end(), MatchByName(datasetName));
		// TODO: Show warning of nonexistent file reference?
		config->mesh.outer.input = iter != datasets.end() ? *iter : nullptr;
	}
	
	config->time.steps = root->get_qualified_as<int>("time.steps").value_or(1);
	
	
	config->load.scaling = root->get_qualified_as<double>("load.scaling").value_or(1.0);
	
	
	std::shared_ptr<cpptoml::table_array> history = root->get_table_array("load.history");
	if(history)
	{
		// FIXME: This code assumes there are no duplicate time entries
		for(const std::shared_ptr<cpptoml::table>& table : *history)
		{
			double time = table->get_as<double>("time").value_or(0.0);
			
			HashSet<Dataset*> datasets;
			for(const std::string& datasetName : *table->get_array_of<std::string>("filename"))
			{
				auto iter = std::find_if(datasets.begin(), datasets.end(), MatchByName(datasetName));
				// TODO: Show warning of nonexistent file reference?
				if(iter != datasets.end())
					datasets.insert(*iter);
			}
			
			SimulationConfig::Load::HistoryEntry entry = {time, std::move(datasets)};
			config->load.history.push_back(entry);
		}
	}
	
	return true;
}


bool MapWindow::serializeSimulationConfig(const QString& path, SimulationConfig* config)
{
	
	std::ofstream stream(path.toStdString());
	if(!stream.is_open())
	{
		QMessageBox* messageBox = new QMessageBox(this);
		messageBox->setWindowTitle(tr("I/O error"));
		messageBox->setText(tr("Could not open %1 for writing").arg(path));
		messageBox->setStandardButtons(QMessageBox::StandardButton::Ok);
		messageBox->setAttribute(Qt::WA_DeleteOnClose);
		messageBox->open();
		return false;
	}
	
	
	stream << "[mesh.inner]" << std::endl;
	if(config->mesh.inner.value.has_value())
		stream << "value = "  << config->mesh.inner.value.value()    << std::endl;
	if(config->mesh.inner.input)
		stream << "input = '" << config->mesh.inner.input->id << "'" << std::endl;
	stream << std::endl;
	
	
	stream << "[mesh.outer]" << std::endl;
	if(config->mesh.outer.value.has_value())
		stream << "value = "  << config->mesh.outer.value.value()    << std::endl;
	if(config->mesh.outer.input)
		stream << "input = '" << config->mesh.outer.input->id << "'" << std::endl;
	stream << std::endl;
	
	
	stream << "[time]"                         << std::endl;
	stream << "steps = " << config->time.steps << std::endl;
	stream << std::endl;
	
	
	stream << "[load]"                             << std::endl;
	stream << "scaling = " << config->load.scaling << std::endl;
	stream << std::endl;
	
	
	struct {
		bool operator()(const SimulationConfig::Load::HistoryEntry& a, const SimulationConfig::Load::HistoryEntry& b) {
			return a.time > b.time;
		}
	} descendingOrder;
	std::list sortedHistory = config->load.history;
	sortedHistory.sort(descendingOrder);
	for(const SimulationConfig::Load::HistoryEntry& entry : sortedHistory)
	{
		stream << "[[load.history]]"                <<        std::endl;
		stream << "time = "      << -entry.time     <<        std::endl;
//		stream << "filename = '" <<  entry.filename << "'" << std::endl; // TODO How to save set?
		stream << std::endl;
	}
	
	return true;
}


bool MapWindow::deserializeDataset(const QString& path, Dataset* dataset)
{
	std::ifstream stream(path.toStdString());
	if(!stream.is_open())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Cannot open %1 for reading").arg(path));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		return false;
	}
	
	
	std::shared_ptr<cpptoml::table> root = nullptr;
	try
	{
		cpptoml::parser parser(stream);
		root = parser.parse();
		stream.close();
	}
	catch(cpptoml::parse_exception& ex)
	{
		stream.close();
		
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Error while parsing '%1'").arg(path));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		return false;
	}
	
	
	if(!root->contains_qualified("h3.resolution"))
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Warning"));
		dialog->setText(tr("File: %1 is missing the 'resolution' attribute").arg(path));
		dialog->setInformativeText(tr("Default '0' will be used"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
	}
	
	if(!root->contains_qualified("h3.values"))
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Warning"));
		dialog->setText(tr("File: %1 is missing the 'values' list").arg(path));
		dialog->setInformativeText(tr("Default empty list will be used"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
	}
	
	
	std::string datasetName = root->get_qualified_as<std::string>("giagui.name").value_or("");
	if(datasetName.empty())
	{
		// NOTE: Use synchronous dialog because async is a huge pain
		QInputDialog dialog(this);
		dialog.setInputMode(QInputDialog::TextInput);
		dialog.setLabelText(tr("Enter name for the dataset"));
		dialog.setTextValue(QFileInfo(path).baseName());
		dialog.setCancelButtonText(tr("Abort"));
//		dialog.setAttribute(Qt::WA_DeleteOnClose);
		int resultCode = dialog.exec();
		if(resultCode != QDialog::Accepted)
			return false;
		datasetName = dialog.textValue().toStdString();
	}
	
	return deserializeDataset(path, dataset, root, datasetName);
}


bool MapWindow::deserializeDataset(const QString& path, Dataset* dataset, const std::shared_ptr<cpptoml::table>& root, const std::string& datasetName)
{
	dataset->id = datasetName;
	
	dataset->resolution  = root->get_qualified_as<int>("h3.resolution").value_or(0);
	
	dataset->isInteger   = root->get_qualified_as<std::string>("h3.type").value_or("").back() != 'f';
	
	if(dataset->isInteger)
		dataset->defaultValue.integer = root->get_qualified_as<int64_t>("h3.default").value_or(0);
	else
		dataset->defaultValue.real    = root->get_qualified_as<double>("h3.default").value_or(0);
	
	dataset->density     = root->get_qualified_as<double>("h3.density").value_or(Dataset::NO_DENSITY);
	
	dataset->measureUnit = "";
	
	dataset->minValue    = {0};
	dataset->maxValue    = {0};
	
	if(dataset->isInteger)
	{
		for(auto& [key, val] : *root->get_table_qualified("h3.values"))
		{
			H3Index  index    = std::stoull(key, nullptr, 16);
			GeoValue geoValue = {0};
			geoValue.integer = val->as<int64_t>()->get();
			dataset->geoValues.insert({index, geoValue});
				
			if(dataset->minValue.integer > geoValue.integer)
				dataset->minValue.integer = geoValue.integer;
			if(dataset->maxValue.integer < geoValue.integer)
				dataset->maxValue.integer = geoValue.integer;
		}
	}
	else
	{
		for(auto& [key, val] : *root->get_table_qualified("h3.values"))
		{
			H3Index  index    = std::stoull(key, nullptr, 16);
			GeoValue geoValue = {0};
			geoValue.real = val->as<double>()->get();
			dataset->geoValues.insert({index, geoValue});
			
			if(dataset->minValue.real > geoValue.real)
				dataset->minValue.real = geoValue.real;
			if(dataset->maxValue.real < geoValue.real)
				dataset->maxValue.real = geoValue.real;
		}
	}
	
	return true;
}


bool MapWindow::serializeDataset(const QString& path, Dataset* dataset)
{
	if(path.size() == 0)
		return false;

	std::ofstream stream(path.toStdString());
	if(!stream.is_open())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("File open error"));
		dialog->setInformativeText(tr("Cannot open %1 for writing").arg(path));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return false;
	}
	stream << std::fixed << std::showpoint;
	
	
	stream << "[giagui]"                       << std::endl;
	stream << "name = '" << dataset->id << "'" << std::endl;
	stream << std::endl;
	
	
	stream << "[h3]" << std::endl;
	stream << "resolution = "  << dataset->resolution << std::endl;
	
	if(dataset->isInteger)
	{
		stream << "type = '1i'"                                 << std::endl;
		stream << "default = " << dataset->defaultValue.integer << std::endl;
	}
	else
	{
		stream << "type = '1f'"                              << std::endl;
		stream << "default = " << dataset->defaultValue.real << std::endl;
	}
	
	if(dataset->hasDensity())
	{
		stream << "density = " << dataset->density << std::endl;
	}
	stream << std::endl;
	
	
	stream << "[h3.values]" << std::endl;
	if(dataset->isInteger)
	{
		for(auto [index, geoValue] : dataset->geoValues)
		{
			stream << std::hex << index;
			stream << " = ";
			stream << std::dec << geoValue.integer;
			stream << std::endl;
		}
	}
	else
	{
		for(auto [index, geoValue] : dataset->geoValues)
		{
			stream << std::hex << index;
			stream << " = ";
			stream << geoValue.real;
			stream << std::endl;
		}
	}
	
	
	if(stream.fail())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Error while writing data to %1").arg(path));
		dialog->setInformativeText(tr("Operation aborted, data may be corrupted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose);
		dialog->open();
		return false;
	}
	return true;
}
