#include "SimulationConfigDialog.hpp"

#include <QKeyEvent>
#include <QValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QAction>
#include <QGroupBox>
#include <QTableView>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QSortFilterProxyModel>
#include <QDebug>


#include "models/GeoHistoryModel.hpp"


struct OptionalIntValidator : public QIntValidator
{
	inline OptionalIntValidator(int bottom, int top, QObject* parent = nullptr) : QIntValidator(bottom, top, parent) {}
	inline QValidator::State validate(QString &input, int& pos) const override
	{
		if(input.isEmpty())
			return QValidator::State::Acceptable;
		return QIntValidator::validate(input, pos);
//		bool ok;
//		int number = input.toInt(&ok, 10);
//		if(ok)
//			if(bottom() <= number && number <= top())
//				return QValidator::State::Acceptable;
//		return QValidator::State::Invalid;
	}
};




SimulationConfigDialog::SimulationConfigDialog(QWidget* parent) : QDialog(parent)
{
	historyModel = new GeoHistoryModel(this);
	
	QGridLayout* mainLayout = new QGridLayout(this);
	int mainLayoutRow = 0;
	{
		QGroupBox* groupBox = new QGroupBox(this);
		groupBox->setTitle(tr("Inner Mesh"));
		groupBox->setFlat(true);
		mainLayout->addWidget(groupBox, mainLayoutRow, 0, 1, 2);
		
		QHBoxLayout* layout = new QHBoxLayout(groupBox);
		
		innerValueEdit = new QLineEdit(groupBox);
		innerValueEdit->setPlaceholderText(tr("Value"));
		innerValueEdit->setValidator(new OptionalIntValidator(1, 0x7FFFFFFF, innerValueEdit));
		layout->addWidget(innerValueEdit, 20);
		
		innerInputEdit = new QLineEdit(groupBox);
		innerInputEdit->setPlaceholderText(tr("File"));
		layout->addWidget(innerInputEdit, 80);
		
#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
		QAction* fileDialogAction = new QAction(innerInputEdit);
		fileDialogAction->setIcon(QIcon::fromTheme("document-open"));
		QObject::connect(fileDialogAction, &QAction::triggered, this, &SimulationConfigDialog::onFileDialogActionTriggered);
		innerInputEdit->addAction(fileDialogAction, QLineEdit::TrailingPosition);
#endif
	}
	mainLayoutRow += 1;
	{
		QGroupBox* groupBox = new QGroupBox(this);
		groupBox->setTitle(tr("Outer Mesh"));
		groupBox->setFlat(true);
		mainLayout->addWidget(groupBox, mainLayoutRow, 0, 1, 2);
		
		QHBoxLayout* layout = new QHBoxLayout(groupBox);
		
		outerValueEdit = new QLineEdit(this);
		outerValueEdit->setPlaceholderText(tr("Value"));
		outerValueEdit->setValidator(new OptionalIntValidator(1, 0x7FFFFFFF, outerValueEdit));
		layout->addWidget(outerValueEdit, 20);
		
		outerInputEdit = new QLineEdit(this);
		outerInputEdit->setPlaceholderText(tr("File"));
		layout->addWidget(outerInputEdit, 80);

#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
		QAction* fileDialogAction = new QAction(outerInputEdit);
		fileDialogAction->setIcon(QIcon::fromTheme("document-open"));
		QObject::connect(fileDialogAction, &QAction::triggered, this, &SimulationConfigDialog::onFileDialogActionTriggered);
		outerInputEdit->addAction(fileDialogAction, QLineEdit::TrailingPosition);
#endif
	}
	mainLayoutRow += 1;
	{
		QLabel* label = new QLabel(this);
		label->setText(tr("Steps"));
		mainLayout->addWidget(label, mainLayoutRow, 0, 1, 1);
		
		stepsEdit = new QLineEdit(this);
		stepsEdit->setValidator(new QIntValidator(1, 0x7FFFFFFF, stepsEdit));
		stepsEdit->setText(QString::number(1));
		mainLayout->addWidget(stepsEdit, mainLayoutRow, 1, 1, 1);
	}
	mainLayoutRow += 1;
	{
		QLabel* label = new QLabel(this);
		label->setText(tr("Load Scaling"));
		mainLayout->addWidget(label, mainLayoutRow, 0, 1, 1);
		
		scalingEdit = new QLineEdit(this);
		scalingEdit->setValidator(new QDoubleValidator(scalingEdit));
		scalingEdit->setText(QString::number(1.0, 'f', 1));
		mainLayout->addWidget(scalingEdit, mainLayoutRow, 1, 1, 1);
	}
	mainLayoutRow += 1;
	{
		QGroupBox* groupBox = new QGroupBox(this);
		groupBox->setTitle(tr("Load History"));
		groupBox->setFlat(true);
		mainLayout->addWidget(groupBox, mainLayoutRow, 0, 1, 2);
		
		QHBoxLayout* historyBoxLayout = new QHBoxLayout(groupBox);
		
		{
			QSortFilterProxyModel* sortedHistoryModel = new QSortFilterProxyModel(this);
			sortedHistoryModel->setSourceModel(historyModel);
			sortedHistoryModel->setDynamicSortFilter(true);
			
			historyTable = new QTableView(this);
			historyTable->setModel(sortedHistoryModel);
			historyTable->horizontalHeader()->setStretchLastSection(true);
			historyTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
			historyTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
			historyTable->setSortingEnabled(true);
			historyTable->sortByColumn(0, Qt::SortOrder::AscendingOrder);
			QObject::connect(historyTable->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SimulationConfigDialog::onSelectionChanged);
			historyBoxLayout->addWidget(historyTable);
		}
		{
			QVBoxLayout* buttonsLayout = new QVBoxLayout();
			historyBoxLayout->addLayout(buttonsLayout);
			
			{
				historyAddButton = new QToolButton(this);
				historyAddButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
				QObject::connect(historyAddButton, &QToolButton::clicked, this, &SimulationConfigDialog::onAddHistoryItemClicked);
				buttonsLayout->addWidget(historyAddButton);
			}
			{
				historyRemoveButton = new QToolButton(this);
				historyRemoveButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-remove")));
				historyRemoveButton->setEnabled(historyModel->rowCount() > 0);
				QObject::connect(historyRemoveButton, &QToolButton::clicked, this, &SimulationConfigDialog::onRemoveHistoryItemClicked);
				buttonsLayout->addWidget(historyRemoveButton);
				
			}
			buttonsLayout->addStretch(1);
		}
	}
	mainLayoutRow += 1;
	{
		QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
		mainLayout->addWidget(buttonBox, mainLayoutRow, 0, 1, 2);
		
		QPushButton* okButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Save);
		QObject::connect(okButton, &QPushButton::clicked, this, &SimulationConfigDialog::onSaveClicked);
		
		QPushButton* cancelButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
		QObject::connect(cancelButton, &QPushButton::clicked, this, &SimulationConfigDialog::reject);
	}
}


void SimulationConfigDialog::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Delete)
	{
		int itemIndex = historyTable->currentIndex().row();
		if(itemIndex != -1)
		{
			event->accept();
			GeoHistoryModel* model = static_cast<GeoHistoryModel*>(historyTable->model());
			model->removeRow(itemIndex);
		}
	}
	event->ignore();
}


#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
void SimulationConfigDialog::onMeshFileDialogActionTriggered()
{
	assert(dynamic_cast<QLineEdit*>(sender()->parent()) != nullptr);
	
	QWidget*     target = static_cast<QWidget*>(sender()->parent());
	QFileDialog* dialog = new QFileDialog(target);
	QObject::connect(dialog, &QFileDialog::finished, this, &SimulationConfigDialog::onMeshFileDialogFinished);
	dialog->show();
}


void SimulationConfigDialog::onMeshFileDialogFinished(int resultCode)
{
	sender()->deleteLater();
	if(resultCode != QDialog::Accepted)
		return;
	
	QFileDialog* dialog = static_cast<QFileDialog*>(sender());
	
	assert(dialog->selectedFiles().size() == 1);
	assert(dialog->selectedFiles().first().size() > 0);
	QString path = dialog->selectedFiles().first();
	
	assert(dynamic_cast<QLineEdit*>(dialog->parent()) != nullptr);
	QLineEdit* target = static_cast<QLineEdit*>(dialog->parent());
	target->setText(path);
}
#endif


SimulationConfig::Load::HistoryEntry* SimulationConfigDialog::selection() const
{
	SimulationConfig::Load::HistoryEntry* entry = nullptr;
	QModelIndex selectionIndex = historyTable->currentIndex();
	if(selectionIndex.isValid())
		entry = historyModel->get(selectionIndex);
	return entry;
}


void SimulationConfigDialog::onSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
	historyRemoveButton->setEnabled(current.isValid());
}


void SimulationConfigDialog::onAddHistoryItemClicked()
{
	SimulationConfig::Load::HistoryEntry entry = {0.0, ""};
	historyModel->appendItem(&entry);
}


void SimulationConfigDialog::onRemoveHistoryItemClicked()
{
	int itemIndex = historyTable->currentIndex().row();
	if(itemIndex != -1)
	{
		GeoHistoryModel* model = static_cast<GeoHistoryModel*>(historyTable->model());
		model->removeRow(itemIndex);
	}
}


void SimulationConfigDialog::onSaveClicked()
{
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setWindowTitle(tr("Save Simulation"));
	dialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	QObject::connect(dialog, &QDialog::accepted, this, &SimulationConfigDialog::onSaveFileDialogAccepted);
	dialog->open();
}


void SimulationConfigDialog::onSaveFileDialogAccepted()
{
	QFileDialog* dialog = static_cast<QFileDialog*>(sender());
	path = dialog->selectedFiles().first();
	
	if(innerValueEdit->text().size() > 0)
		configuration.mesh.inner.value = innerValueEdit->text().toInt();
	if(innerInputEdit->text().size() > 0)
		configuration.mesh.inner.input = innerInputEdit->text().toStdString();
	if(outerValueEdit->text().size() > 0)
		configuration.mesh.outer.value = outerValueEdit->text().toInt();
	if(outerInputEdit->text().size() > 0)
		configuration.mesh.outer.input = outerInputEdit->text().toStdString();
	configuration.time.steps = stepsEdit->text().toInt();
	configuration.load.scaling = scalingEdit->text().toFloat();
	for(const GeoHistoryModel::HistoryEntry& item : historyModel->items)
		configuration.load.history.push_back(item);
	
	accept();
}
