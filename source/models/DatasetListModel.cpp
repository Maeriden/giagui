#include "DatasetListModel.hpp"

#include <utility>
#include "Dataset.hpp"


DatasetListModel::DatasetListModel(QObject* parent) : QAbstractListModel(parent)
{
	
}


DatasetListModel::~DatasetListModel()
{
	for(Dataset* dataset : items)
	{
		delete dataset;
	}
}


int DatasetListModel::rowCount() const
{
	return items.size();
}


int DatasetListModel::rowCount(const QModelIndex& parent) const
{
	return items.size();
}


QVariant DatasetListModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();
	if(role != Qt::ItemDataRole::DisplayRole && role != Qt::ItemDataRole::EditRole)
		return QVariant();
	
	Dataset* item = items[index.row()];
	switch(index.column())
	{
		case 0: return QString::fromStdString(item->id);
	}
	return QVariant();
}


QVariant DatasetListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}


bool DatasetListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role != Qt::ItemDataRole::EditRole)
		return false;
	
	Dataset* item = items[index.row()];
	switch(index.column())
	{
		case 0:  item->id = value.toString().toStdString();
	}
	return true;
}


Qt::ItemFlags DatasetListModel::flags(const QModelIndex& index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}


bool DatasetListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	beginInsertRows(QModelIndex(), row, row);
	items.insert(items.begin()+row, count, nullptr);
	endInsertRows();
	return true;
}


bool DatasetListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if(0 <= row && row < (int)items.size())
	{
		beginRemoveRows(QModelIndex(), row, row);
		items.erase(items.begin()+row, items.begin()+row+count);
		endRemoveRows();
		return true;
	}
	return false;
}


Dataset* DatasetListModel::get(const QModelIndex& modelIndex)
{
	assert(modelIndex.isValid());
	int row = modelIndex.row();
	if(0 <= row && row < (int)items.size())
	{
		Dataset* dataset = items[row];
		return dataset;
	}
	return nullptr;
}


QModelIndex DatasetListModel::findIndex(Dataset* dataset)
{
	auto iter = std::find(items.begin(), items.end(), dataset);
	if(iter != items.end())
	{
		int row = iter - items.begin();
		QModelIndex result = createIndex(row, 0, dataset);
		return result;
	}
	return createIndex(-1, 0, nullptr);
}


bool DatasetListModel::appendItem(Dataset* dataset)
{
	assert(dataset);
	bool result = insertRows(items.size(), 1, QModelIndex());
	items.back() = dataset;
	return result;
}


bool DatasetListModel::removeItem(Dataset* dataset)
{
	bool result = false;
	auto iter = std::find(items.begin(), items.end(), dataset);
	if(iter != items.end())
	{
		int row = iter - items.begin();
		assert(0 <= row && row < (int)items.size());
		result = removeRows(row, 1, QModelIndex());
	}
	return result;
}


DatasetListModel::Iterator DatasetListModel::begin()
{
	return items.begin();
}


DatasetListModel::Iterator DatasetListModel::end()
{
	return items.end();
}
