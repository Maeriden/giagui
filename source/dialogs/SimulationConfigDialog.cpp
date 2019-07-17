#include "SimulationConfigDialog.hpp"

#include <cassert>
#include <utility>

#include <QKeyEvent>
#include <QValidator>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QToolButton>
#include <QAction>
#include <QGroupBox>
#include <QListView>
#include <QTableView>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QDebug>

#include <source/Dataset.hpp>
#include "models/DatasetListModel.hpp"
#include "models/GeoHistoryModel.hpp"


using HistoryEntry = SimulationConfig::Load::HistoryEntry;


// Allows not setting a value
struct OptionalIntValidator : public QIntValidator
{
	inline OptionalIntValidator(int bottom, int top, QObject* parent = nullptr) : QIntValidator(bottom, top, parent) {}
	inline QValidator::State validate(QString &input, int& pos) const override
	{
		if(input.isEmpty())
			return QValidator::State::Acceptable;
		return QIntValidator::validate(input, pos);
	}
};


// Creates a blank item mapped to not setting a value
struct MeshInputProxyModel : public QAbstractListModel
{
	std::list<Dataset*> items;
	
	
	explicit MeshInputProxyModel(DatasetListModel* source, QObject* parent = nullptr) : QAbstractListModel(parent)
	{
		for(Dataset* dataset : *source)
			if(dataset->isInteger)
				items.push_back(dataset);
	}
	
	int rowCount(const QModelIndex& parent) const override
	{
		int result = 1 + items.size();
		return result;
	}
	
	QVariant data(const QModelIndex& index, int role) const override
	{
		if(role == Qt::DisplayRole || role == Qt::EditRole)
		{
			if(index.row() > 0)
			{
				Dataset* dataset = *std::next(items.begin(), index.row() - 1);
				return QString::fromStdString(dataset->id);
			}
		}

		// UserRole is used to get the actual dataset instead of its name
		if(role == Qt::UserRole)
		{
			if(index.row() > 0)
			{
				Dataset* dataset = *std::next(items.begin(), index.row() - 1);
				return QVariant::fromValue(dataset);
			}
			return QVariant::fromValue((Dataset*)nullptr);
		}
		
		return QVariant();
	}
	
	
	using Iterator = decltype(items)::iterator;
	inline Iterator begin() { return items.begin(); }
	inline Iterator end()   { return items.end();   }
};


struct TimesModel : public QAbstractListModel
{
	std::list<HistoryEntry> items;
	
	
	explicit TimesModel(std::list<HistoryEntry> times, QObject* parent = nullptr) : QAbstractListModel(parent)
	{
		this->items = std::move(times);
	}
	
	int rowCount()                          const          { return items.size(); }
	int rowCount(const QModelIndex& parent) const override { return rowCount();   }
	
	QVariant data(const QModelIndex& index, int role) const override
	{
		if(index.row() < 0 || items.size() <= (size_t)index.row())
			return QVariant();
		if(role != Qt::DisplayRole && role != Qt::EditRole)
			return QVariant();
		
		return std::next(items.begin(), index.row())->time;
	}
	
	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if(index.row() < 0 || items.size() <= (size_t)index.row())
			return false;
		if(role != Qt::ItemDataRole::EditRole)
			return false;
		
		bool ok;
		double time = value.toDouble(&ok);
		if(!ok)
			return false;
		std::next(items.begin(), index.row())->time = time; 
		return true;
	}
	
	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
	}
	
	bool insertRows(int row, int count, const QModelIndex& parent) override
	{
		beginInsertRows(parent, row, row);
		auto position = std::next(items.begin(), row);
		items.insert(position, count, HistoryEntry{0.0, HashSet<Dataset*>()});
		endInsertRows();
		return true;
	}
	
	bool removeRows(int row, int count, const QModelIndex& parent) override
	{
		if(row < 0 || items.size() <= (size_t)row)
			return false;
		beginRemoveRows(parent, row, row);
		auto position = std::next(items.begin(), row);
		items.erase(position, std::next(position, count));
		endRemoveRows();
		return true;
	}
	
	HistoryEntry* appendRow()
	{
		int row = items.size();
		
		beginInsertRows(QModelIndex(), row, row);
		items.push_back(HistoryEntry{0.0, HashSet<Dataset*>()});
		endInsertRows();
		return &items.back();
	}
	
	HistoryEntry* get(const QModelIndex& index)
	{
		if(!index.isValid())
			return nullptr;
		if(index.model() != this)
			return nullptr;
		auto iter = std::next(items.begin(), index.row());
		return &(*iter);
	}
	
	
	using Iterator = decltype(items)::iterator;
	inline Iterator begin() { return items.begin(); }
	inline Iterator end()   { return items.end();   }
};


struct TimeDatasetsModel : QAbstractListModel
{
	std::list<Dataset*>      items;
	std::list<HistoryEntry>& times;
	HistoryEntry*            selectedTime = nullptr;
	
	
	explicit TimeDatasetsModel(std::list<Dataset*> items, std::list<HistoryEntry>& times, QObject* parent = nullptr) : QAbstractListModel(parent),
		items(std::move(items)),
		times(times)
	{}
	
	void setSelectedTime(HistoryEntry* selectedTime)
	{
		if(this->selectedTime != selectedTime)
		{
			beginResetModel();
			this->selectedTime = selectedTime;
			endResetModel();
		}
	}
	
	HistoryEntry* getOwner(Dataset* dataset) const
	{
		for(HistoryEntry& entry : times)
		{
			if(entry.datasets.count(dataset) > 0)
				return &entry;
		}
		return nullptr;
	}
	
	int rowCount()                          const          { return items.size(); }
	int rowCount(const QModelIndex& parent) const override { return rowCount();      }
	
	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
//		return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
		Qt::ItemFlags flags = Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
		
		// Do not enable item if no time is selected
		if(!selectedTime)
			return flags;
		
		// Do not enable item if it is owned by another time entry
		Dataset*      dataset = *std::next(items.begin(), index.row());
		HistoryEntry* owner   = getOwner(dataset);
		if(owner && owner != selectedTime)
			return flags;
		
		// Enable item if it does not have an owner or the owner is the currently selected time entry
		return flags | Qt::ItemIsEnabled;
	}
	
	QVariant data(const QModelIndex& index, int role) const override
	{
		if(role == Qt::ItemDataRole::DisplayRole)
		{
			Dataset* dataset = *std::next(items.begin(), index.row());
			return QString::fromStdString(dataset->id);
		}
		
		// This fragment decides whether an item shows a checkbox and, if it does, if it's checked or unchecked
		// Items always show a checkbox
		// - if item has an owner show checked
		// - if item has no owner show uncheckd
		// If the owner is different from the selected time entry the item will be disabled, but this is handled by the flags() funciton
		if(role == Qt::ItemDataRole::CheckStateRole)
		{
//			if(!selectedTime)
//				return Qt::CheckState::Unchecked;
			
			Dataset*      dataset = *std::next(items.begin(), index.row());
			HistoryEntry* owner   = getOwner(dataset);
			return owner ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
		}
		return QVariant();
	}
	
	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if(role != Qt::ItemDataRole::CheckStateRole)
			return false;
		
		// We can't set ownership if we don't know the owner
		if(!selectedTime)
			return false;
		
		Dataset* dataset = *std::next(items.begin(), index.row());
		bool isChecked = value.toBool(); 
		if(isChecked)
		{
			assert(getOwner(dataset) == nullptr);
			selectedTime->datasets.insert(dataset);
		}
		else
		{
			assert(getOwner(dataset) == selectedTime);
			selectedTime->datasets.erase(dataset);
		}
		return true;
	}
	
	void refresh()
	{
		beginResetModel();
		endResetModel();
	}
};


SimulationConfigDialog::SimulationConfigDialog(DatasetListModel* datasetsModel, SimulationConfig* configuration, QWidget* parent) : QDialog(parent),
	configuration(configuration)
{
	assert(configuration);
	
	timesModel        = new TimesModel(configuration->load.history, this);
	timeDatasetsModel = new TimeDatasetsModel(datasetsModel->items, timesModel->items, this);
	
	
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
		
		innerInputComboBox = new QComboBox(groupBox);
		innerInputComboBox->setModel(new MeshInputProxyModel(datasetsModel, this));
		layout->addWidget(innerInputComboBox, 80);
		
#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
		QAction* fileDialogAction = new QAction(innerInputComboBox);
		fileDialogAction->setIcon(QIcon::fromTheme("document-open"));
		QObject::connect(fileDialogAction, &QAction::triggered, this, &SimulationConfigDialog::onFileDialogActionTriggered);
		innerInputComboBox->addAction(fileDialogAction, QLineEdit::TrailingPosition);
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
		
		outerInputComboBox = new QComboBox(groupBox);
		outerInputComboBox->setModel(new MeshInputProxyModel(datasetsModel, this));
		layout->addWidget(outerInputComboBox, 80);

#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
		QAction* fileDialogAction = new QAction(outerInputComboBox);
		fileDialogAction->setIcon(QIcon::fromTheme("document-open"));
		QObject::connect(fileDialogAction, &QAction::triggered, this, &SimulationConfigDialog::onFileDialogActionTriggered);
		outerInputComboBox->addAction(fileDialogAction, QLineEdit::TrailingPosition);
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
		
		QVBoxLayout* historyBoxLayout = new QVBoxLayout(groupBox);
		
		// Add/remove buttons
		{
			QHBoxLayout* buttonsLayout = new QHBoxLayout();
			historyBoxLayout->addLayout(buttonsLayout);
			
			{
				timesAddButton = new QToolButton(this);
				timesAddButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-add")));
				QObject::connect(timesAddButton, &QToolButton::clicked, this, &SimulationConfigDialog::onAddHistoryItemClicked);
				buttonsLayout->addWidget(timesAddButton);
			}
			{
				timesRemoveButton = new QToolButton(this);
				timesRemoveButton->setIcon(QIcon::fromTheme(QString::fromUtf8("list-remove")));
				timesRemoveButton->setEnabled(timesModel->rowCount() > 0);
				QObject::connect(timesRemoveButton, &QToolButton::clicked, this, &SimulationConfigDialog::onRemoveHistoryItemClicked);
				buttonsLayout->addWidget(timesRemoveButton);
				
			}
			buttonsLayout->addStretch(1);
		}
		
		// Times and datasetsModel lists
		{
			QHBoxLayout* listsLayout = new QHBoxLayout();
			historyBoxLayout->addLayout(listsLayout);
			
			
			timesListView = new QListView(this);
			timesListView->setModel(timesModel);
			timesListView->setSelectionMode(QAbstractItemView::SingleSelection);
			timesListView->installEventFilter(this);
			QObject::connect(timesListView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SimulationConfigDialog::onTimeSelectionChanged);
			listsLayout->addWidget(timesListView);
			
			datasetsListView = new QListView(this);
			datasetsListView->setModel(timeDatasetsModel);
			listsLayout->addWidget(datasetsListView);
		}
	}
	mainLayoutRow += 1;
	{
		QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
		mainLayout->addWidget(buttonBox, mainLayoutRow, 0, 1, 2);
		
		QPushButton* okButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Ok);
		QObject::connect(okButton, &QPushButton::clicked, this, &SimulationConfigDialog::onOkClicked);
		
		QPushButton* cancelButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
		QObject::connect(cancelButton, &QPushButton::clicked, this, &SimulationConfigDialog::reject);
	}
	
	if(configuration->mesh.inner.value.has_value())
	{
		QString text = QString::number(configuration->mesh.inner.value.value());
		innerValueEdit->setText(text);
	}
	
	if(configuration->mesh.inner.input)
	{
		QString text = QString::fromStdString(configuration->mesh.inner.input->id);
		innerInputComboBox->setCurrentText(text);
	}
	
	if(configuration->mesh.outer.value.has_value())
	{
		innerValueEdit->setText(QString::number(configuration->mesh.outer.value.value()));
	}
	
	if(configuration->mesh.outer.input)
	{
		QString text = QString::fromStdString(configuration->mesh.outer.input->id);
		outerInputComboBox->setCurrentText(text);
	}
	
	
	QModelIndex   selectedTimeIndex = timesListView->currentIndex();
	HistoryEntry* selectedTime      = timesModel->get(selectedTimeIndex);
	timeDatasetsModel->setSelectedTime(selectedTime);
}


bool SimulationConfigDialog::eventFilter(QObject* object, QEvent* event)
{
	if(object != timesListView)
		return false;
	if(event->type() != QEvent::KeyPress)
		return false;
	
	QKeyEvent* keyPressEvent = static_cast<QKeyEvent*>(event);
	if(keyPressEvent->key() != Qt::Key_Delete)
		return false;
	onRemoveHistoryItemClicked();
	return true;
}


void SimulationConfigDialog::onTimeSelectionChanged(const QModelIndex& current, const QModelIndex& previous)
{
	timesRemoveButton->setEnabled(current.isValid());
	
	HistoryEntry* selectedTime = timesModel->get(current);
	timeDatasetsModel->setSelectedTime(selectedTime);
}


void SimulationConfigDialog::onAddHistoryItemClicked()
{
	timesModel->appendRow();
}


void SimulationConfigDialog::onRemoveHistoryItemClicked()
{
	QModelIndex selectedTimeIndex = timesListView->currentIndex();
	if(selectedTimeIndex.isValid())
	{
		timesModel->removeRow(selectedTimeIndex.row());
		timeDatasetsModel->refresh();
	}
}


void SimulationConfigDialog::onOkClicked()
{
	configuration->mesh.inner.value.reset();
	if(innerValueEdit->text().size() > 0)
	{
		bool ok;
		int value = innerValueEdit->text().toInt(&ok);
		if(ok)
			configuration->mesh.inner.value = value;
	}
	
	
	configuration->mesh.inner.input = innerInputComboBox->currentData(Qt::UserRole).value<Dataset*>(); 
	
	
	configuration->mesh.outer.value.reset();
	if(outerValueEdit->text().size() > 0)
	{
		bool ok;
		int value = outerValueEdit->text().toInt(&ok);
		if(ok)
			configuration->mesh.outer.value = value;
	}
	
	
	configuration->mesh.outer.input = outerInputComboBox->currentData(Qt::UserRole).value<Dataset*>();
	
	
	configuration->time.steps = 1;
	if(stepsEdit->text().size() > 0)
	{
		bool ok;
		int value = stepsEdit->text().toInt(&ok);
		if(ok)
			configuration->time.steps = value; 
	}
	
	
	configuration->load.scaling = 1.0;
	if(scalingEdit->text().size() > 0)
	{
		bool ok;
		double value = scalingEdit->text().toDouble(&ok);
		if(ok)
			configuration->load.scaling = value;
	}
	
	
	configuration->load.history.clear();
	for(HistoryEntry& entry : *timesModel)
	{
		configuration->load.history.push_back(entry);
	}
	
	accept();
}
