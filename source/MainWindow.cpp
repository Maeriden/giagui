#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include <QHBoxLayout>
#include <QGraphicsScene>
#include <QtGui/QDoubleValidator>
#include <QtSvg/QGraphicsSvgItem>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	this->h3State.resolution = 0;
	this->h3State.activeIndex = H3_INVALID_INDEX;
	this->mapTool = MapTool::Rect;
	
	this->ui->setupUi(this);
	this->setupToolbar();
	
	this->statusLabel = new QLabel();
	this->ui->statusBar->addWidget(this->statusLabel);
	
	QGraphicsScene* scene = new QGraphicsScene(this);
	scene->addItem(new QGraphicsSvgItem(":/images/world.svg"));
	this->ui->mapView->setScene(scene);
	
	// IMPORTANT: https://stackoverflow.com/questions/2445997/qgraphicsview-and-eventfilter 
	this->ui->mapView->viewport()->installEventFilter(this);
	this->ui->mapView->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	this->ui->mapView->h3State = &this->h3State;
}


MainWindow::~MainWindow()
{
	delete this->ui;
}


void MainWindow::setupToolbar()
{
	QToolBar* toolbar = this->ui->mainToolBar;
	
	{
		QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* openAction = new QAction(QIcon::fromTheme("document-open"), tr("Open"), actionGroup);
		openAction->setShortcuts(QKeySequence::StandardKey::Open);
		openAction->setStatusTip(tr("Open"));
		
		QAction* saveAction = new QAction(QIcon::fromTheme("document-save"), tr("Save"), actionGroup);
		saveAction->setShortcuts(QKeySequence::StandardKey::Save);
		saveAction->setStatusTip(tr("Save"));
		
		QAction* saveAsAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save As"), actionGroup);
		saveAsAction->setShortcuts(QKeySequence::StandardKey::SaveAs);
		saveAsAction->setStatusTip(tr("Save as"));
		
		toolbar->addActions(actionGroup->actions());
	}
	toolbar->addSeparator();
	{
		QActionGroup* actionGroup = new QActionGroup(this);
		
		QAction* rectAction = new QAction(QIcon(":/images/icon-rect.svg"), tr("Rect tool"), actionGroup);
		rectAction->setStatusTip(tr("Select area to polyfill"));
		rectAction->setCheckable(true);
		rectAction->setChecked(true);
		connect(rectAction, &QAction::triggered, this, &MainWindow::onActionTriggeredRectTool);
		
		QAction* editAction = new QAction(QIcon(":/images/icon-edit.svg"), tr("Edit tool"), actionGroup);
		editAction->setStatusTip(tr("Edit cell values"));
		editAction->setCheckable(true);
		connect(editAction, &QAction::triggered, this, &MainWindow::onActionTriggeredEditTool);
		
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
		this->editWater->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editWater->setEnabled(false);
		this->editWater->setValidator(new QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editWater);
		connect(this->editWater,    &QLineEdit::editingFinished, this, &MainWindow::onCellChangedWater);
		
		QLabel* labelI = new QLabel(tr("Ice"), group);
		layout->addWidget(labelI);
		
		this->editIce = new QLineEdit("", group);
		this->editIce->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editIce->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editIce->setEnabled(false);
		this->editIce->setValidator(new QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editIce);
		connect(this->editIce,      &QLineEdit::editingFinished, this, &MainWindow::onCellChangedIce);
		
		QLabel* labelS = new QLabel(tr("Sediment"), group);
		layout->addWidget(labelS);
		
		this->editSediment = new QLineEdit("", group);
		this->editSediment->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		this->editSediment->setAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
		this->editSediment->setEnabled(false);
		this->editSediment->setValidator(new QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, DECIMAL_DIGITS));
		layout->addWidget(this->editSediment);
		connect(this->editSediment, &QLineEdit::editingFinished, this, &MainWindow::onCellChangedSediment);
		
		toolbar->addWidget(group);
	}
	toolbar->addSeparator();
	{
		QLabel* label = new QLabel(tr("Resolution"), this);
		toolbar->addWidget(label);
		
		this->resolutionSpinbox = new QSpinBox(this);
		this->resolutionSpinbox->setMinimum(0);
		this->resolutionSpinbox->setMaximum(MAX_SUPPORTED_RESOLUTION);
		connect(this->resolutionSpinbox, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::onResolutionChanged);
		toolbar->addWidget(this->resolutionSpinbox);
	}
}


bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
	if(event->type() == QEvent::MouseButtonPress)
	{
		QWidget* viewport = (QWidget*)object;
		return this->handleMapEventMouseDown((MapView*)viewport->parentWidget(), (QMouseEvent*)event);
	}
	
	if(event->type() == QEvent::MouseMove)
	{
		QWidget* viewport = (QWidget*)object;
		return this->handleMapEventMouseMove((MapView*)viewport->parentWidget(), (QMouseEvent*)event);
	}
	
	return false;
}


void MainWindow::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		event->accept();
		this->h3State.activeIndex = H3_INVALID_INDEX;
		
		this->editWater->setEnabled(false);
		this->editIce->setEnabled(false);
		this->editSediment->setEnabled(false);
		
		this->editWater->setText("");
		this->editIce->setText("");
		this->editSediment->setText("");
	}
	else
	{
		event->ignore();
	}
}


void MainWindow::onActionTriggeredRectTool()
{
	this->mapTool = MapTool::Rect;
	
	this->editWater->setEnabled(false);
	this->editIce->setEnabled(false);
	this->editSediment->setEnabled(false);
	
	this->editWater->setText("");
	this->editIce->setText("");
	this->editSediment->setText("");
}


void MainWindow::onActionTriggeredEditTool()
{
	this->mapTool = MapTool::Edit;
}


void MainWindow::onResolutionChanged(int resolution)
{
	this->h3State.resolution = resolution;
	
	// TODO: Transfer old resolution cell data to new reolution cell data
	this->ui->mapView->resetPolyfill();
	this->ui->mapView->scene()->invalidate();
}


void MainWindow::onCellChangedWater()
{
	if(this->h3State.activeIndex == H3_INVALID_INDEX)
		return;
	bool   valid;
	double value = this->editWater->text().toDouble(&valid);
	if(valid)
	{
		this->h3State.cellsData[this->h3State.activeIndex].water = value;
		this->ui->mapView->scene()->invalidate();
		
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editWater->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MainWindow::onCellChangedIce()
{
	if(this->h3State.activeIndex == H3_INVALID_INDEX)
		return;
	bool   valid;
	double value = this->editIce->text().toDouble(&valid);
	if(valid)
	{
		this->h3State.cellsData[this->h3State.activeIndex].ice = value;
		this->ui->mapView->scene()->invalidate();
		
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editIce->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


void MainWindow::onCellChangedSediment()
{
	if(this->h3State.activeIndex == H3_INVALID_INDEX)
		return;
	bool   valid;
	double value = this->editSediment->text().toDouble(&valid);
	if(valid)
	{
		this->h3State.cellsData[this->h3State.activeIndex].sediment = value;
		this->ui->mapView->scene()->invalidate();
		
		// Reset text to actual stored value (in case of conversion weirdness)
		this->editSediment->setText(QString::number(value, 'f', DECIMAL_DIGITS));
	}
}


bool MainWindow::handleMapEventMouseMove(MapView* mapView, QMouseEvent* event)
{
	QPointF scenePoint = mapView->mapToScene(event->pos());
	if(mapView->sceneRect().contains(scenePoint))
	{
		GeoCoord c = toGeocoord(scenePoint, mapView->sceneRect().size());
		QString message = QString("%1 : %2").arg(radsToDegs(c.lon)).arg(radsToDegs(c.lat));
		this->statusLabel->setText(message);
	}
	return false;
}


bool MainWindow::handleMapEventMouseDown(MapView* mapView, QMouseEvent* event)
{
	if(this->mapTool != MapTool::Edit)
		return false;
	if(event->button() != Qt::MouseButton::LeftButton)
		return false;
	
	QPointF scenePoint = mapView->mapToScene(event->pos());
	if(mapView->sceneRect().contains(scenePoint))
	{
		GeoCoord coord = toGeocoord(scenePoint, mapView->sceneRect().size());
		H3Index  index = geoToH3(&coord, this->h3State.resolution);
		if(index != H3_INVALID_INDEX)
		{
			if(this->h3State.activeIndex != index)
			{
				this->h3State.activeIndex = index;
				
				this->editWater->setEnabled(true);
				this->editIce->setEnabled(true);
				this->editSediment->setEnabled(true);
				
				auto it = this->h3State.cellsData.find(this->h3State.activeIndex);
				if(it != this->h3State.cellsData.end())
				{
					this->editWater->setText(QString::number(it->second.water, 'f', DECIMAL_DIGITS));
					this->editIce->setText(QString::number(it->second.ice, 'f', DECIMAL_DIGITS));
					this->editSediment->setText(QString::number(it->second.sediment, 'f', DECIMAL_DIGITS));
				}
				else
				{
					this->editWater->setText("");
					this->editIce->setText("");
					this->editSediment->setText("");
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
		else
		{
			this->editWater->setEnabled(false);
			this->editIce->setEnabled(false);
			this->editSediment->setEnabled(false);
			
			this->editWater->setText("");
			this->editIce->setText("");
			this->editSediment->setText("");
		}
	}
	return true;
}
