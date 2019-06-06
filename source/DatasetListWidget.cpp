#include "DatasetListWidget.hpp"

#include <QKeyEvent>
#include <QListWidget>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QMessageBox>

#include "Dataset.hpp"
#include "models/DatasetListModel.hpp"
#include <dialogs/DatasetCreateDialog.hpp>


DatasetListWidget::DatasetListWidget(DatasetListModel* datasets, QWidget* parent) : QWidget(parent)
{
	assert(datasets);
	this->datasets = datasets;
	
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(6, 6, 0, 3);
	layout->setSpacing(0);
	
	{
		QHBoxLayout* headerLayout = new QHBoxLayout();
		headerLayout->setSpacing(2);
		headerLayout->setMargin(0);
		
		layout->addLayout(headerLayout, 0);
		layout->setStretchFactor(headerLayout, 0);
		layout->setAlignment(headerLayout, Qt::AlignTop);
		
		createButton = new QToolButton(this);
		createButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
		createButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		QObject::connect(createButton, &QPushButton::clicked, this, &DatasetListWidget::onCreateDatasetClicked);
		headerLayout->addWidget(createButton);
		
		deleteButton = new QToolButton(this);
		deleteButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-remove")));
		deleteButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		QObject::connect(deleteButton, &QPushButton::clicked, this, &DatasetListWidget::onDeleteDatasetClicked);
		headerLayout->addWidget(deleteButton);
		
		headerLayout->addStretch(1);
	}
	
	
	listView = new QListView(this);
	listView->setModel(datasets);
	listView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	listView->setMaximumSize(180, 180);
	listView->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
//	QObject::connect(listView, &QListView::clicked, this, &DatasetListWidget::onListItemClicked);
	QObject::connect(listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &DatasetListWidget::onSelectionChanged);
	layout->addWidget(listView);
	
	layout->setStretchFactor(listView, 0);
	layout->setAlignment(listView, Qt::AlignTop);
	
	layout->addStretch(1);
}


Dataset* DatasetListWidget::selection() const
{
	Dataset* dataset = nullptr;
	QModelIndex selectionIndex = listView->currentIndex();
	if(selectionIndex.isValid())
		dataset = datasets->get(selectionIndex);
	return dataset;
}


void DatasetListWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Delete)
	{
		event->accept();
		onDeleteDatasetClicked();
	}
	event->ignore();
}


void DatasetListWidget::onSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
	assert(current != previous);
	Dataset* currentDataset  = datasets->get(current);
	Dataset* previousDataset = datasets->get(previous);
	deleteButton->setEnabled(currentDataset != nullptr);
	emit itemSelected(currentDataset, previousDataset);
}


void DatasetListWidget::onCreateDatasetClicked()
{
	DatasetCreateDialog* dialog = new DatasetCreateDialog(this);
	dialog->setWindowTitle(tr("Create dataset"));
	dialog->setAttribute(Qt::WA_DeleteOnClose, true);
	QObject::connect(dialog, &DatasetCreateDialog::finished, this, &DatasetListWidget::onCreateDatasetDialogFinished);
	dialog->open();
}


void DatasetListWidget::onCreateDatasetDialogFinished(int resultCode)
{
	if(resultCode != QDialog::Accepted)
		return;
	
	DatasetCreateDialog* dialog = static_cast<DatasetCreateDialog*>(sender());
	DatasetID_t datasetId         = dialog->datasetName.toStdString();
	bool        datasetHasDensity = dialog->datasetHasDensity;
	bool        datasetIsInteger  = dialog->datasetIsInteger;
	assert(datasetId.size() > 0);
	
	Dataset* dataset = nullptr;
	try
	{
		dataset = new Dataset(datasetId, datasetHasDensity, datasetIsInteger);
	}
	catch(std::bad_alloc& ex)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Memory allocation error"));
//		dialog->setInformativeText(tr("Operation was aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		return;
	}
	
	if(datasets->appendItem(dataset))
	{
		QModelIndex modelIndex = datasets->findIndex(dataset);
		assert(modelIndex.row() != -1);
		emit itemCreated(dataset);
	}
	else
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Dataset creation failed"));
//		dialog->setInformativeText(tr("Operation was aborted"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
		delete dataset;
	}
}


void DatasetListWidget::onDeleteDatasetClicked()
{
	Dataset* dataset = selection();
	if(dataset)
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Delete dataset"));
		dialog->setText(tr("Deleting %1").arg(QString::fromStdString(dataset->id)));
		dialog->setInformativeText(tr("Are you sure?"));
		dialog->setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
		dialog->setDefaultButton(QMessageBox::StandardButton::Cancel);
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->setProperty("dataset", QVariant::fromValue(dataset));
		QObject::connect(dialog, &QDialog::finished, this, &DatasetListWidget::onDeleteDatasetDialogFinished);
		dialog->open();
	}
}


void DatasetListWidget::onDeleteDatasetDialogFinished(int resultCode)
{
	if(resultCode != QMessageBox::Yes)
		return;
	
	Dataset* dataset = sender()->property("dataset").value<Dataset*>();
	assert(dataset);
	if(datasets->removeItem(dataset))
	{
		emit itemDeleted(dataset);
		delete dataset;
	}
	else
	{
		QMessageBox* dialog = new QMessageBox(this);
		dialog->setWindowTitle(tr("Error"));
		dialog->setText(tr("Dataset deletion failed"));
		dialog->setInformativeText(tr("Dataset was not found among the current datasetsModel"));
		dialog->setAttribute(Qt::WA_DeleteOnClose, true);
		dialog->open();
	}
}
