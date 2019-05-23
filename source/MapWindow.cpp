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
#include "source/dialogs/SimulationConfigDialog.hpp"
#include "models/DatasetListModel.hpp"
#include "dialogs/DatasetCreateDialog.hpp"


MapWindow::MapWindow(QWidget* parent) : QMainWindow(parent)
{
	datasets = new DatasetListModel(this);
#if 0
	datasets->appendItem(new Dataset("prova", true, false));
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
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-open")));
		action->setText(tr("Open..."));
		action->setShortcuts(QKeySequence::StandardKey::Open);
		action->setStatusTip(tr("Open file"));
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionOpenFile);
		menuFile->addAction(action);
	} {
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save")));
		action->setText(tr("Save"));
		action->setShortcuts(QKeySequence::StandardKey::Save);
		action->setStatusTip(tr("Save file"));
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveFile);
		menuFile->addAction(action);
	} {
		QAction* action = new QAction(this);
		action->setIcon(QIcon::fromTheme(QString::fromUtf8("document-save-as")));
		action->setText(tr("Save As..."));
		action->setShortcuts(QKeySequence::StandardKey::SaveAs);
		action->setStatusTip(tr("Save file as"));
		QObject::connect(action, &QAction::triggered, this, &MapWindow::onActionSaveAs);
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
		
		QAction* actionUnmark = new QAction();
		actionUnmark->setIcon(QIcon(QString::fromUtf8(":/images/icon-cell-unmark.svg")));
		actionUnmark->setText(tr("Unmark"));
		actionUnmark->setActionGroup(actionGroup);
		actionUnmark->setStatusTip(tr("Click on the map to unmark cells for editing"));
		actionUnmark->setCheckable(true);
		QObject::connect(actionUnmark, &QAction::triggered, this, &MapWindow::onUnmarkToolTriggered);
		
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
	bool confirmed = true;
	for(auto& [dataset, saveState] : datasetSaveStates)
	{
		if(saveState.modified)
		{
			confirmed = false;
			break;
		}
	}
	
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
	
	for(const QString& qPath : fileDialog->selectedFiles())
	{
		assert(!qPath.isEmpty());
		const std::string& path = qPath.toStdString();
		
		Dataset* dataset = deserializeDataset(path);
		if(dataset)
		{
			if(datasets->appendItem(dataset))
			{
				datasetSaveStates[dataset].path = path;
				datasetSaveStates[dataset].modified = false;
			}
			else
			{
				QMessageBox* dialog = new QMessageBox(this);
				dialog->setWindowTitle(tr("Error"));
				dialog->setText(tr("Error while adding %1 to the datasets list").arg(QString::fromStdString(dataset->id)));
				dialog->setAttribute(Qt::WA_DeleteOnClose, true);
				dialog->open();
				
				delete dataset;
			}
		}
	}
}


void MapWindow::onActionSaveFile()
{
	Dataset* dataset = datasetListWidget->selection();
	if(!dataset)
		return;
	
	DatasetSaveState& saveState = datasetSaveStates[dataset];
	saveFileBegin(dataset, saveState.path);
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
		saveFileBegin(dataset, saveState.path);
	}
}


void MapWindow::saveFileBegin(Dataset* dataset, const std::string& path)
{
	assert(dataset);
	if(path.empty())
	{
		QFileDialog* dialog = new QFileDialog(this);
		dialog->setWindowTitle(tr("Save File"));
		dialog->setNameFilter(tr("TOML+H3 (*.h3);;All Files (*)"));
		dialog->setAcceptMode(QFileDialog::AcceptSave);
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->setProperty("dataset", QVariant::fromValue(dataset));
		dialog->selectFile(QString::fromStdString(dataset->id) + ".h3");
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
	
	saveFileEnd(dataset, path.toStdString());
}


void MapWindow::saveFileEnd(Dataset* dataset, const std::string& path)
{
	bool success = serializeDataset(path, dataset);
	if(success)
	{
		DatasetSaveState& saveState = datasetSaveStates[dataset];
		saveState.path     = path;
		saveState.modified = false;
		
		if(dataset == datasetListWidget->selection())
		{
			setWindowFilePath(QString::fromStdString(saveState.path));
			setWindowModified(saveState.modified);
		}
	}
}


void MapWindow::onActionConfigureSimulation()
{
	SimulationConfigDialog* simulationWindow = new SimulationConfigDialog(this);
	simulationWindow->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(simulationWindow, &SimulationConfigDialog::accepted, this, &MapWindow::onSimulationConfigDialogAccepted);
	simulationWindow->open();
}


void MapWindow::onSimulationConfigDialogAccepted()
{
	struct {
		bool operator()(const SimulationConfig::Load::HistoryEntry& a, const SimulationConfig::Load::HistoryEntry& b) {
			return a.time > b.time;
		}
	} descendingOrder;
	
	SimulationConfigDialog* simulationConfigDialog = static_cast<SimulationConfigDialog*>(sender());
	SimulationConfig&       config                 = simulationConfigDialog->configuration;
	
	std::sort(config.load.history.begin(), config.load.history.end(), descendingOrder);
	
	std::ofstream stream(simulationConfigDialog->path.toStdString());
	if(!stream.is_open())
	{
		QMessageBox* messageBox = new QMessageBox(this);
		messageBox->setWindowTitle(tr("I/O error"));
		messageBox->setText(tr("Could not open %1 for writing").arg(simulationConfigDialog->path));
		messageBox->setStandardButtons(QMessageBox::StandardButton::Ok);
		QObject::connect(messageBox, &QMessageBox::finished, messageBox, &QMessageBox::deleteLater);
		messageBox->open();
		return;
	}
	
	stream << "[mesh.inner]" << std::endl;
	if(config.mesh.inner.value.has_value())
		stream << "value = "  << config.mesh.inner.value.value()        << std::endl;
	if(config.mesh.inner.input.has_value())
		stream << "input = '" << config.mesh.inner.input.value() << "'" << std::endl;
	stream << std::endl;
	
	stream << "[mesh.outer]" << std::endl;
	if(config.mesh.outer.value.has_value())
		stream << "value = "  << config.mesh.outer.value.value()        << std::endl;
	if(config.mesh.outer.input.has_value())
		stream << "input = '" << config.mesh.outer.input.value() << "'" << std::endl;
	stream << std::endl;
	
	stream << "[time]"                        << std::endl;
	stream << "steps = " << config.time.steps << std::endl;
	stream << std::endl;
	
	stream << "[load]"                          << std::endl;
	stream << "steps = " << config.load.scaling << std::endl;
	stream << std::endl;
	
	for(const SimulationConfig::Load::HistoryEntry& entry : config.load.history)
	{
		stream << "[[load.history]]"                <<        std::endl;
		stream << "time = "      << -entry.time     <<        std::endl;
		stream << "filename = '" <<  entry.filename << "'" << std::endl;
		stream << std::endl;
	}
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


void MapWindow::onGridToolTriggered()
{
	mapTool = MapTool::Grid;
	mapView->setInteractionMode(MapView::InteractionMode::Grid);
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


void MapWindow::onDatasetListItemCreated(Dataset* dataset)
{
	assert(dataset);
	datasetSaveStates.insert({dataset, DatasetSaveState()});
}


void MapWindow::onDatasetListItemSelected(Dataset* dataset)
{
	if(dataset)
	{
		
		
		geoValueEditLine->setPlaceholderText(QString::fromStdString(dataset->measureUnit));
		writeHighlightedGeoValuesIntoLineEdit();
		if(QApplication::focusWidget() == geoValueEditLine)
			geoValueEditLine->selectAll();
		
		DatasetSaveState& saveState = datasetSaveStates[dataset];
		setWindowFilePath(QString::fromStdString(saveState.path));
		setWindowModified(saveState.modified);
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
		
		setWindowFilePath("");
		setWindowModified(false);
	}
	
	// NOTE: Setting data source triggers a resolution changed signal that will recompute highlighted cells 
	datasetControlWidget->setDataSource(dataset);
	mapView->setDataSource(dataset);
}


void MapWindow::onDatasetListItemDeleted(Dataset* dataset)
{
	datasetSaveStates.erase(dataset);
}


void MapWindow::onDatasetResolutionChanged(Dataset* dataset, int newResolution)
{
	assert(IS_VALID_RESOLUTION(newResolution));
	assert(dataset->resolution != newResolution);
	
	// TODO: Profile this to see if it's worth doing the operation in another thread
	try
	{
		int oldResolution = dataset->resolution;
		if(newResolution < oldResolution)
		{
			// NOTE: We keep this line for future reference just in case Qt feels like being a dick
//			QApplication::postEvent(this->resolutionSpinbox, new QMouseEvent(QEvent::MouseButtonRelease, QPoint( 0, 0 ), Qt::LeftButton, Qt::NoButton, Qt::NoModifier) );
			dataset->decreaseResolution(newResolution);
			onDatasetResolutionDecreased(dataset, oldResolution);
		}
		else
		{
			dataset->increaseResolution(newResolution);
			onDatasetResolutionIncreased(dataset, oldResolution);
		}
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox::critical(this, tr("Memory allocation error"), tr("Not enough memory to store new values"));
	}
	
	mapView->requestRepaint();
}


void MapWindow::onDatasetResolutionDecreased(Dataset* dataset, int oldResolution)
{
	HashSet<H3Index> hlIndices;
	for(H3Index childIndex : highlightedIndices)
	{
		H3Index parentIndex = h3ToParent(childIndex, dataset->resolution);
		assert(parentIndex != H3_INVALID_INDEX);
		hlIndices.insert(parentIndex);
	}
	this->highlightedIndices = std::move(hlIndices);
	
	
	HashSet<H3Index> gIndices;
	for(H3Index childIndex : gridIndices)
	{
		H3Index parentIndex = h3ToParent(childIndex, dataset->resolution);
		assert(parentIndex != H3_INVALID_INDEX);
		gIndices.insert(parentIndex);
	}
	gridIndices = std::move(gIndices);
}


void MapWindow::onDatasetResolutionIncreased(Dataset* dataset, int oldResolution)
{
	uint64_t childrenBufferLength = h3MaxChildrenCount(oldResolution, dataset->resolution);
	H3Index* childrenBuffer       = new H3Index[childrenBufferLength];
	
	HashSet<H3Index> newHighlightIndices;
	for(H3Index parentIndex : highlightedIndices)
	{
		h3ToChildren(parentIndex, dataset->resolution, childrenBuffer);
		newHighlightIndices.insert(childrenBuffer, childrenBuffer + childrenBufferLength);
	}
	newHighlightIndices.erase(H3_INVALID_INDEX);
	highlightedIndices = std::move(newHighlightIndices);
	
	
	HashSet<H3Index> newGridIndices;
	for(H3Index parentIndex : gridIndices)
	{
		h3ToChildren(parentIndex, dataset->resolution, childrenBuffer);
		newGridIndices.insert(childrenBuffer, childrenBuffer + childrenBufferLength);
	}
	newGridIndices.erase(H3_INVALID_INDEX);
	gridIndices = std::move(newGridIndices);
	
	delete[] childrenBuffer;
}


void MapWindow::onDatasetDefaultChanged(Dataset* dataset, GeoValue newDefaultValue)
{
	assert(datasets);
	
	dataset->defaultValue = newDefaultValue;
}


void MapWindow::onDatasetDensityChanged(Dataset* dataset, double value)
{
	assert(datasets);
	
	assert(dataset->hasDensity());
	assert(dataset->density != value);
	dataset->density = value;
}


void MapWindow::onDatasetValueRangeChanged(Dataset* dataset, GeoValue minValue, GeoValue maxValue)
{
	assert(datasets);
	
	dataset->minValue = minValue;
	dataset->maxValue = maxValue;
	mapView->redrawValuesRange();
	mapView->requestRepaint(); // update colors
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
		DatasetSaveState& saveState = datasetSaveStates[dataset];
		saveState.modified = true;
		
		setWindowModified(saveState.modified);
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
	
	if(mapTool == MapTool::Mark)
		highlightedIndices.insert(index);
	else if(mapTool == MapTool::Unmark)
		highlightedIndices.erase(index);
		
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


Dataset* MapWindow::deserializeDataset(const std::string& path)
{
	std::ifstream stream(path);
	if(!stream.is_open())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Cannot open %1 for reading").arg(QString::fromStdString(path)));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return nullptr;
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
		dialog->setText(tr("Error while parsing '%1'").arg(QString::fromStdString(path)));
		dialog->setInformativeText(tr("Operation aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return nullptr;
	}
	
	
	if(!root->contains_qualified("h3.resolution"))
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Warning"));
		dialog->setText(tr("File: %1 is missing the 'resolution' attribute").arg(QString::fromStdString(path)));
		dialog->setInformativeText(tr("Default '0' will be used"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
	}
	
	if(!root->contains_qualified("h3.values"))
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Warning"));
		dialog->setText(tr("File: %1 is missing the 'values' list").arg(QString::fromStdString(path)));
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
		dialog.setTextValue(QFileInfo(QString::fromStdString(path)).baseName());
		dialog.setCancelButtonText(tr("Abort"));
//		dialog.setAttribute(Qt::WA_DeleteOnClose);
		int resultCode = dialog.exec();
		if(resultCode != QDialog::Accepted)
			return nullptr;
		datasetName = dialog.textValue().toStdString();
	}
	
	return deserializeDataset(path, root, datasetName);
}


Dataset* MapWindow::deserializeDataset(const std::string& path, const std::shared_ptr<cpptoml::table>& root, const std::string& datasetName)
{
	Dataset* dataset = nullptr;
	try
	{
		dataset = new Dataset(datasetName);
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Memory allocation error"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return nullptr;
	}
	
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
	
	return dataset;
}


bool MapWindow::serializeDataset(const std::string& path, Dataset* dataset)
{
	if(path.empty())
		return false;

	std::ofstream stream(path);
	if(!stream.is_open())
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("File open error"));
		dialog->setInformativeText(tr("Cannot open %1 for writing").arg(QString::fromStdString(path)));
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
		dialog->setText(tr("Error while writing data to %1").arg(QString::fromStdString(path)));
		dialog->setInformativeText(tr("Operation aborted, data may be corrupted").arg(QString::fromStdString(path)));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return false;
	}
	return true;
}
