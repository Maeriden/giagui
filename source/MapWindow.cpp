#include "MapWindow.hpp"

#include <QApplication>
#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QtGui/QDoubleValidator>
#include <QtSvg/QGraphicsSvgItem>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>


#include <QSpinBox>
// https://bugreports.qt.io/browse/QTBUG-14259
class IntSpinBox : public QSpinBox
{
public:
	explicit IntSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {}
	
	// Override default behavior of stepping if step buttons are held down
	void timerEvent(QTimerEvent *event) override
	{
		QWidget::timerEvent(event); // NOLINT(bugprone-parent-virtual-call)
	}
	
	// Do not highlight the text in the spinbox after step buttons are pressed
	void stepBy(int steps) override
	{
		QSpinBox::stepBy(steps);
		lineEdit()->deselect();
	}
};


// https://stackoverflow.com/questions/35178569/doublevalidator-is-not-checking-the-ranges-properly
class DoubleValidator : public QDoubleValidator
{
public:
	DoubleValidator(double min, double max, int decimals, QObject* parent = nullptr) : QDoubleValidator(min, max, decimals, parent) {}
	
	// Make empty strings a valid "number" (to represent NaN values)
	State validate(QString& input, int& pos) const override
	{
		if(input.isEmpty())
			return QValidator::Acceptable;
		return QDoubleValidator::validate(input, pos);
	}
};


MapWindow::MapWindow(QWidget *parent) : QMainWindow(parent),
	h3State(&globalH3State)
{
	this->h3State->resolution = 0;
	this->h3State->activeIndex = H3_INVALID_INDEX;
	this->mapTool = MapTool::Rect;
	
	this->setupUi();
	this->setupToolbar();
	
	this->statusLabel = new QLabel();
	this->statusBar->addWidget(this->statusLabel);
	
	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addItem(new QGraphicsSvgItem(":/images/world.svg"));
	this->mapView->setScene(scene);
	
	// IMPORTANT: https://stackoverflow.com/questions/2445997/qgraphicsview-and-eventfilter 
	this->mapView->viewport()->installEventFilter(this);
	this->mapView->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	
	connect(this->mapView, &MapView::polyfillFailed, this, &MapWindow::onPolyfillFailed);
}


void MapWindow::setupUi()
{
	if(objectName().isEmpty())
		setObjectName(QString::fromUtf8("MapWindow"));
	resize(1000, 750);
	centralWidget = new QWidget(this);
	centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
	gridLayout = new QGridLayout(centralWidget);
	gridLayout->setSpacing(0);
	gridLayout->setContentsMargins(11, 11, 11, 11);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	gridLayout->setContentsMargins(0, 0, 0, 0);
	mapView = new MapView(centralWidget);
	mapView->setObjectName(QString::fromUtf8("mapView"));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(mapView->sizePolicy().hasHeightForWidth());
	mapView->setSizePolicy(sizePolicy);
	mapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mapView->setDragMode(QGraphicsView::NoDrag);
	
	gridLayout->addWidget(mapView, 0, 0, 1, 1);
	
	setCentralWidget(centralWidget);
	mainToolBar = new QToolBar(this);
	mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
	mainToolBar->setEnabled(true);
	mainToolBar->setMovable(false);
	mainToolBar->setAllowedAreas(Qt::TopToolBarArea);
	addToolBar(Qt::TopToolBarArea, mainToolBar);
	statusBar = new QStatusBar(this);
	statusBar->setObjectName(QString::fromUtf8("statusBar"));
	statusBar->setSizeGripEnabled(true);
	setStatusBar(statusBar);
	
	QMetaObject::connectSlotsByName(this);
}


void MapWindow::setupToolbar()
{
	QToolBar* toolbar = this->mainToolBar;
	
	{
		QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* openAction = new QAction(QIcon::fromTheme("document-open"), tr("Open"), actionGroup);
		openAction->setShortcuts(QKeySequence::StandardKey::Open);
		openAction->setStatusTip(tr("Open"));
		connect(openAction, &QAction::triggered, this, &MapWindow::onActionOpenFile);
		
		QAction* saveAction = new QAction(QIcon::fromTheme("document-save"), tr("Save"), actionGroup);
		saveAction->setShortcuts(QKeySequence::StandardKey::Save);
		saveAction->setStatusTip(tr("Save"));
		connect(saveAction, &QAction::triggered, this, &MapWindow::onActionSaveFile);
		
		QAction* saveAsAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save As"), actionGroup);
		saveAsAction->setShortcuts(QKeySequence::StandardKey::SaveAs);
		saveAsAction->setStatusTip(tr("Save as"));
		connect(saveAsAction, &QAction::triggered, this, &MapWindow::onActionSaveFileAs);
		
		toolbar->addActions(actionGroup->actions());
	}
	toolbar->addSeparator();
	{
		QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), tr("Zoom Out"), actionGroup);
		zoomOutAction->setShortcuts(QKeySequence::StandardKey::ZoomOut);
		zoomOutAction->setStatusTip(tr("Zoom out"));
		connect(zoomOutAction, &QAction::triggered, this, &MapWindow::onActionZoomOut);
		
		QAction* zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), tr("Zoom In"), actionGroup);
		zoomInAction->setShortcuts(QKeySequence::StandardKey::ZoomIn);
		zoomInAction->setStatusTip(tr("Zoom in"));
		connect(zoomInAction,  &QAction::triggered, this, &MapWindow::onActionZoomIn);
		
		toolbar->addActions(actionGroup->actions());
	}
	toolbar->addSeparator();
	{
		QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* rectAction = new QAction(QIcon(":/images/icon-rect.svg"), tr("Rect Tool"), actionGroup);
		rectAction->setStatusTip(tr("Select area to polyfill"));
		rectAction->setCheckable(true);
		rectAction->setChecked(true);
		connect(rectAction, &QAction::triggered, this, &MapWindow::onActionRectTool);
		
		QAction* editAction = new QAction(QIcon(":/images/icon-edit.svg"), tr("Edit Tool"), actionGroup);
		editAction->setStatusTip(tr("Edit cell values"));
		editAction->setCheckable(true);
		connect(editAction, &QAction::triggered, this, &MapWindow::onActionEditTool);
		
		toolbar->addActions(actionGroup->actions());
	}
	toolbar->addSeparator();
	{
		QWidget*     group  = new QWidget(this);
		QHBoxLayout* layout = new QHBoxLayout(group);
		layout->setSpacing(6);
		layout->setContentsMargins(5, 0, 5, 0);
		
		QLabel* labelW = new QLabel(tr("Water"), group);
		layout->addWidget(labelW);
		
		this->editWater = new QLineEdit("", group);
		this->editWater->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editWater->setMinimumWidth(80);
		this->editWater->setMaximumWidth(100);
		this->editWater->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editWater->setEnabled(false);
		this->editWater->setPlaceholderText(tr("N/A"));
		this->editWater->setValidator(new DoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editWater);
		connect(this->editWater,    &QLineEdit::editingFinished, this, &MapWindow::onCellChangedWater);
		
		QLabel* labelI = new QLabel(tr("Ice"), group);
		layout->addWidget(labelI);
		
		this->editIce = new QLineEdit("", group);
		this->editIce->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editIce->setMinimumWidth(80);
		this->editIce->setMaximumWidth(100);
		this->editIce->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editIce->setEnabled(false);
		this->editIce->setPlaceholderText(tr("N/A"));
		this->editIce->setValidator(new DoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editIce);
		connect(this->editIce,      &QLineEdit::editingFinished, this, &MapWindow::onCellChangedIce);
		
		QLabel* labelS = new QLabel(tr("Sediment"), group);
		layout->addWidget(labelS);
		
		this->editSediment = new QLineEdit("", group);
		this->editSediment->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editSediment->setMinimumWidth(80);
		this->editSediment->setMaximumWidth(100);
		this->editSediment->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editSediment->setEnabled(false);
		this->editSediment->setPlaceholderText(tr("N/A"));
		this->editSediment->setValidator(new DoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editSediment);
		connect(this->editSediment, &QLineEdit::editingFinished, this, &MapWindow::onCellChangedSediment);
		
		QLabel* labelD = new QLabel(tr("Sediment density"), group);
		layout->addWidget(labelD);
		
		this->editDensity = new QLineEdit("", group);
		this->editDensity->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editDensity->setMinimumWidth(80);
		this->editDensity->setMaximumWidth(100);
		this->editDensity->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editDensity->setEnabled(false);
		this->editDensity->setPlaceholderText(tr("N/A"));
		this->editDensity->setValidator(new DoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editDensity);
		connect(this->editDensity, &QLineEdit::editingFinished, this, &MapWindow::onCellChangedDensity);
		
		toolbar->addWidget(group);
	}
	toolbar->addSeparator();
	{
		QLabel* label = new QLabel(tr("Resolution"), this);
		toolbar->addWidget(label);
		
		this->resolutionSpinbox = new IntSpinBox(this);
		this->resolutionSpinbox->setFocusPolicy(Qt::NoFocus);
		this->resolutionSpinbox->setMinimum(0);
		this->resolutionSpinbox->setMaximum(MAX_SUPPORTED_RESOLUTION);
		connect(this->resolutionSpinbox, qOverload<int>(&IntSpinBox::valueChanged), this, &MapWindow::onResolutionChanged);
		toolbar->addWidget(this->resolutionSpinbox);
	}
}


bool MapWindow::eventFilter(QObject* object, QEvent* event)
{
	if(event->type() == QEvent::MouseButtonPress)
	{
		QWidget* viewport = (QWidget*)object;
		return this->handleMapEventMousePress((MapView*)viewport->parentWidget(), (QMouseEvent*)event);
	}
	
	if(event->type() == QEvent::MouseMove)
	{
		QWidget* viewport = (QWidget*)object;
		return this->handleMapEventMouseMove((MapView*)viewport->parentWidget(), (QMouseEvent*)event);
	}
	
	if(event->type() == QEvent::MouseButtonRelease)
	{
		QWidget* viewport = (QWidget*)object;
		return this->handleMapEventMouseRelease((MapView*)viewport->parentWidget(), (QMouseEvent*)event);
	}
	
	return false;
}


void MapWindow::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		event->accept();
		this->h3State->activeIndex = H3_INVALID_INDEX;
		this->mapView->scene()->invalidate();
		
		this->setAllLineEditEnabled(false);
		this->clearAllLineEditNoSignal();
	}
	else
	{
		event->ignore();
	}
}


void MapWindow::onActionOpenFile()
{
	QString caption = tr("Import data");
	QString dir     = "";
	QString filter  = tr("TOML (*.toml);;All Files (*)");
	QString filePath = QFileDialog::getOpenFileName(this, caption, dir, filter);
	if(filePath.isEmpty())
		return;
	
	bool confirmed = this->h3State->cellsData.empty(); 
	if(!confirmed)
	{
		QString question = tr("Current data will be overwritten. Proceed?");
		QMessageBox::StandardButton reply = QMessageBox::question(this, "", question, QMessageBox::Ok|QMessageBox::Cancel);
		confirmed = reply == QMessageBox::Ok;
	}
	
	if(confirmed)
	{
		int                         resolution;
		std::map<H3Index, CellData> cellsData;
		int error = importFile(filePath.toLocal8Bit().constData(), &resolution, &cellsData);
		if(error == 0)
		{
			H3State_reset(this->h3State, resolution);
			this->h3State->cellsData = cellsData;
			
			this->setAllLineEditEnabled(false);
			this->clearAllLineEditNoSignal();
			
			this->resolutionSpinbox->blockSignals(true);
			this->resolutionSpinbox->setValue(this->h3State->resolution);
			this->resolutionSpinbox->blockSignals(false);
			
			this->mapView->scene()->invalidate();
			
			this->exportPath = filePath;
			setWindowFilePath(this->exportPath);
			setWindowModified(false);
		}
		if(error == 1)
		{
			QMessageBox::information(this, tr("Error"), tr("Unable to open file"));
		}
		if(error == 2)
		{
			QMessageBox::information(this, tr("Error"), tr("Unable to parse file"));
		}
	}
}


void MapWindow::onActionSaveFile()
{
	if(this->exportPath.isEmpty())
	{
		this->onActionSaveFileAs();
		return;
	}
	
	int error = exportFile(this->exportPath.toLocal8Bit().constData(), this->h3State->resolution, this->h3State->cellsData);
	if(error == 0)
	{
		setWindowFilePath(this->exportPath);
		setWindowModified(false);
	}
	if(error == 1)
	{
		QMessageBox::information(this, tr("Error"), tr("Unable to save file"));
	}
	else if(error == 2)
	{
		QMessageBox::information(this, tr("Error"), tr("I/O error while writing data"));
	}
}


void MapWindow::onActionSaveFileAs()
{
	QString caption = tr("Export data");
	QString cwd     = "";
	QString filter  = tr("TOML (*.toml);;All Files (*)");
	// NOTE: using QFileDialog::getSaveFileName() does not automatically append the extension
	QFileDialog saveDialog(this, caption, cwd, filter);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.setDefaultSuffix("toml");
	int result = saveDialog.exec();
	if(result != QDialog::Accepted)
		return;
	QString filePath = saveDialog.selectedFiles().first();
	
	this->exportPath = filePath;
	this->onActionSaveFile();
}


void MapWindow::onActionZoomOut()
{
	QPoint vsAnchor = this->mapView->viewport()->rect().center();
	this->mapView->zoom(vsAnchor, -1.0);
}


void MapWindow::onActionZoomIn()
{
	QPoint vsAnchor = this->mapView->viewport()->rect().center();
	this->mapView->zoom(vsAnchor, +1.0);
}


void MapWindow::onActionRectTool()
{
	this->mapTool = MapTool::Rect;
	this->h3State->activeIndex = H3_INVALID_INDEX;
	
	if(QApplication::focusWidget())
		QApplication::focusWidget()->clearFocus();
	
	this->setAllLineEditEnabled(false);
	this->clearAllLineEditNoSignal();
	
	this->mapView->scene()->invalidate();
}


void MapWindow::onActionEditTool()
{
	this->mapTool = MapTool::Edit;
}


void MapWindow::onCellChangedWater()
{
	if(this->h3State->activeIndex == H3_INVALID_INDEX)
		return;
	
	// If all values would be NaN, remove cell
	if(this->editWater->text().isEmpty())
	{
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			if(std::isnan(it->second.ice) && std::isnan(it->second.sediment) && std::isnan(it->second.density))
				this->h3State->cellsData.erase(it);
			else
				it->second.water = DOUBLE_NAN;
			
			setWindowModified(true);
			this->mapView->scene()->invalidate();
		}
		return;
	}
	
	bool   isValidNumber;
	double value = this->editWater->text().toDouble(&isValidNumber);
	if(isValidNumber)
	{
		// Find item and set value, otherwise make new item with value
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			it->second.water = value;
		}
		else
		{
			CellData item = CELLDATA_INIT;
			item.water = value;
			this->h3State->cellsData[this->h3State->activeIndex] = item;
		}
		
		setWindowModified(true);
		this->mapView->scene()->invalidate();
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editWater->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MapWindow::onCellChangedIce()
{
	if(this->h3State->activeIndex == H3_INVALID_INDEX)
		return;
	
	// If all values would be NaN, remove cell
	if(this->editIce->text().isEmpty())
	{
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			if(std::isnan(it->second.water) && std::isnan(it->second.sediment) && std::isnan(it->second.density))
				this->h3State->cellsData.erase(it);
			else
				it->second.ice = DOUBLE_NAN;
			
			setWindowModified(true);
			this->mapView->scene()->invalidate();
		}
		return;
	}
	
	bool   isValidNumber;
	double value = this->editIce->text().toDouble(&isValidNumber);
	if(isValidNumber)
	{
		// Find item and set value, otherwise make new item with value
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			it->second.ice = value;
		}
		else
		{
			CellData item = CELLDATA_INIT;
			item.ice = value;
			this->h3State->cellsData[this->h3State->activeIndex] = item;
		}
		setWindowModified(true);
		this->mapView->scene()->invalidate();
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editIce->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MapWindow::onCellChangedSediment()
{
	if(this->h3State->activeIndex == H3_INVALID_INDEX)
		return;
	
	// If all values would be NaN, remove cell
	if(this->editSediment->text().isEmpty())
	{
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			if(std::isnan(it->second.water) && std::isnan(it->second.ice) && std::isnan(it->second.density))
				this->h3State->cellsData.erase(it);
			else
				it->second.sediment = DOUBLE_NAN;
			
			setWindowModified(true);
			this->mapView->scene()->invalidate();
		}
		return;
	}
	
	bool   isValidNumber;
	double value = this->editSediment->text().toDouble(&isValidNumber);
	if(isValidNumber)
	{
		// Find item and set value, otherwise make new item with value
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			it->second.sediment = value;
		}
		else
		{
			CellData item = CELLDATA_INIT;
			item.sediment = value;
			this->h3State->cellsData[this->h3State->activeIndex] = item;
		}
		
		setWindowModified(true);
		this->mapView->scene()->invalidate();
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editSediment->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MapWindow::onCellChangedDensity()
{
	if(this->h3State->activeIndex == H3_INVALID_INDEX)
		return;
	
	// If all values would be NaN, remove cell
	if(this->editDensity->text().isEmpty())
	{
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			if(std::isnan(it->second.water) && std::isnan(it->second.ice) && std::isnan(it->second.sediment))
				this->h3State->cellsData.erase(it);
			else
				it->second.density = DOUBLE_NAN;
			
			setWindowModified(true);
			this->mapView->scene()->invalidate();
		}
		return;
	}
	
	bool   isValidNumber;
	double value = this->editDensity->text().toDouble(&isValidNumber);
	if(isValidNumber)
	{
		// Find item and set value, otherwise make new item with value
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			it->second.density = value;
		}
		else
		{
			CellData item = CELLDATA_INIT;
			item.density = value;
			this->h3State->cellsData[this->h3State->activeIndex] = item;
		}
		
		setWindowModified(true);
		this->mapView->scene()->invalidate();
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editDensity->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MapWindow::onResolutionChanged(int resolution)
{
	if(resolution == this->h3State->resolution)
		return;
	
	if(!this->h3State->cellsData.empty())
	{
		QString question = tr("Changing resolution will reset stored data. Proceed?");
		QMessageBox* box = new QMessageBox(QMessageBox::Question, "", question, QMessageBox::Ok|QMessageBox::Cancel, this);
		box->setWindowModality(Qt::WindowModal);
		connect(box, &QMessageBox::finished, this, &MapWindow::onResolutionChangedDialogFinished);
		box->open();
	}
	else
	{
		H3State_reset(this->h3State, resolution);
		
		this->setAllLineEditEnabled(false);
		this->clearAllLineEditNoSignal();
		
		this->mapView->scene()->invalidate();
	}
}


void MapWindow::onResolutionChangedDialogFinished(int dialogResult)
{
	if(dialogResult == QMessageBox::Ok)
	{
		int resolution = this->resolutionSpinbox->value();
		H3State_reset(this->h3State, resolution);
		
		this->setAllLineEditEnabled(false);
		this->clearAllLineEditNoSignal();
		
		setWindowModified(true);
		
		this->mapView->scene()->invalidate();
	}
	else
	{
		this->resolutionSpinbox->blockSignals(true);
		this->resolutionSpinbox->setValue(this->h3State->resolution);
		this->resolutionSpinbox->blockSignals(false);
	}
}


void MapWindow::onPolyfillFailed(PolyfillError error)
{
	if(error == PolyfillError::THRESHOLD_EXCEEDED)
	{
		QMessageBox::warning(this, tr("Warning"), tr("Polyfill area too big, Request ignored to avoid slowdown"));
	}
	else
	if(error == PolyfillError::MEMORY_ALLOCATION)
	{
		QMessageBox::critical(this, tr("Memory allocation error"), tr("Not enough memory to polyfill the selected area"));
	}
}


void MapWindow::highlightCell(H3Index index)
{
	// NOTE: Should we handle this case?
	assert(index != H3_INVALID_INDEX);
	
	if(this->h3State->activeIndex != index)
	{
		this->h3State->activeIndex = index;
		this->mapView->scene()->invalidate();
		
		this->setAllLineEditEnabled(true);
		
		auto it = this->h3State->cellsData.find(this->h3State->activeIndex);
		if(it != this->h3State->cellsData.end())
		{
			QString text;
			text = std::isnan(it->second.water)    ? "" : QString::number(it->second.water,    'f', DECIMAL_DIGITS);
			this->editWater->blockSignals(true);
			this->editWater->setText(text);
			this->editWater->blockSignals(false);
			
			text = std::isnan(it->second.ice)      ? "" : QString::number(it->second.ice,      'f', DECIMAL_DIGITS);
			this->editIce->blockSignals(true);
			this->editIce->setText(text);
			this->editIce->blockSignals(false);
			
			text = std::isnan(it->second.sediment) ? "" : QString::number(it->second.sediment, 'f', DECIMAL_DIGITS);
			this->editSediment->blockSignals(true);
			this->editSediment->setText(text);
			this->editSediment->blockSignals(false);
			
			text = std::isnan(it->second.density)  ? "" : QString::number(it->second.density,  'f', DECIMAL_DIGITS);
			this->editDensity->blockSignals(true);
			this->editDensity->setText(text);
			this->editDensity->blockSignals(false);
		}
		else
		{
			this->clearAllLineEditNoSignal();
		}
		
		QLineEdit* focused = dynamic_cast<QLineEdit*>(QApplication::focusWidget());
		if(focused)
		{
			focused->selectAll();
		}
		else
		{
			this->editWater->setFocus();
			this->editWater->selectAll();
		}
	}
}


bool MapWindow::handleMapEventMousePress(MapView* mapView, QMouseEvent* event)
{
	if(this->mapTool == MapTool::Edit)
	{
		if(event->button() == Qt::MouseButton::LeftButton)
		{
			QPointF scenePoint = mapView->mapToScene(event->pos());
			if(mapView->sceneRect().contains(scenePoint))
			{
				GeoCoord coord = toGeocoord(scenePoint, mapView->sceneRect().size());
				H3Index  index = geoToH3(&coord, this->h3State->resolution);
				if(index != H3_INVALID_INDEX)
				{
					this->highlightCell(index);
				}
			}
			return true;
		}
	}
	return false;
}


bool MapWindow::handleMapEventMouseMove(MapView* mapView, QMouseEvent* event)
{
	QPointF scenePoint = mapView->mapToScene(event->pos());
	if(mapView->sceneRect().contains(scenePoint))
	{
		GeoCoord c = toGeocoord(scenePoint, mapView->sceneRect().size());
		QString message = QString("%1 : %2").arg(radsToDegs(c.lon)).arg(radsToDegs(c.lat));
		this->statusLabel->setText(message);
	}
	
	if(this->mapTool == MapTool::Edit)
	{
		if(event->buttons() & Qt::MouseButton::LeftButton)
		{
			if(mapView->sceneRect().contains(scenePoint))
			{
				GeoCoord coord = toGeocoord(scenePoint, mapView->sceneRect().size());
				H3Index  index = geoToH3(&coord, this->h3State->resolution);
				if(index != H3_INVALID_INDEX)
				{
					this->highlightCell(index);
				}
			}
			return true;
		}
	}
	return false;
}


bool MapWindow::handleMapEventMouseRelease(MapView* mapView, QMouseEvent* event)
{
	if(this->mapTool == MapTool::Edit)
	{
		if(event->button() == Qt::MouseButton::LeftButton)
		{
			return true;
		}
	}
	return false;
}


void MapWindow::setAllLineEditEnabled(bool enabled)
{
	this->editWater->setEnabled(enabled);
	this->editIce->setEnabled(enabled);
	this->editSediment->setEnabled(enabled);
	this->editDensity->setEnabled(enabled);
}


void MapWindow::clearAllLineEditNoSignal()
{
	this->editWater->blockSignals(true);
	this->editWater->clear();
	this->editWater->blockSignals(false);
	
	this->editIce->blockSignals(true);
	this->editIce->clear();
	this->editIce->blockSignals(false);
	
	this->editSediment->blockSignals(true);
	this->editSediment->clear();
	this->editSediment->blockSignals(false);
	
	this->editDensity->blockSignals(true);
	this->editDensity->clear();
	this->editDensity->blockSignals(false);
}
