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
	
	Dataset* dataset = *std::next(items.begin(), index.row());
	return QString::fromStdString(dataset->id);
}


QVariant DatasetListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	return QVariant();
}


bool DatasetListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role != Qt::ItemDataRole::EditRole)
		return false;
	
	Dataset* dataset = *std::next(items.begin(), index.row());
	dataset->id = value.toString().toStdString();
	return true;
}


Qt::ItemFlags DatasetListModel::flags(const QModelIndex& index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}


bool DatasetListModel::insertRows(int row, int count, const QModelIndex& parent)
{
	beginInsertRows(parent, row, row);
	auto position = std::next(items.begin(), row);
	items.insert(position, count, nullptr);
	endInsertRows();
	return true;
}


bool DatasetListModel::removeRows(int row, int count, const QModelIndex& parent)
{
	if(0 <= row && row < (int)items.size())
	{
		beginRemoveRows(parent, row, row);
		auto position = std::next(items.begin(), row);
		items.erase(position, std::next(position, count));
		endRemoveRows();
		return true;
	}
	return false;
}


void DatasetListModel::reset(std::list<Dataset*>&& newItems)
{
	beginResetModel();
	for(Dataset* dataset : items)
		delete dataset;
	this->items = newItems;
	endResetModel();
}


Dataset* DatasetListModel::get(int row)
{
	if(0 <= row && row < (int)items.size())
	{
		Dataset* dataset = *std::next(items.begin(), row);
		return dataset;
	}
	return nullptr;
}


Dataset* DatasetListModel::get(const QModelIndex& modelIndex)
{
	if(!modelIndex.isValid())
		return nullptr;
	if(modelIndex.model() != this)
		return nullptr;
	return get(modelIndex.row());
}


QModelIndex DatasetListModel::findIndex(Dataset* dataset)
{
	auto iter = std::find(items.begin(), items.end(), dataset);
	if(iter != items.end())
	{
		int row = std::distance(items.begin(), iter);
		QModelIndex result = createIndex(row, 0, *iter);
		return result;
	}
	return QModelIndex();
}


bool DatasetListModel::appendItem(Dataset* dataset)
{
	int row = items.size();
	
	beginInsertRows(QModelIndex(), row, row);
	items.push_back(dataset);
	endInsertRows();
	return true;
}


bool DatasetListModel::removeItem(Dataset* dataset)
{
	auto position = std::find(items.begin(), items.end(), dataset);
	if(position != items.end())
	{
		int row = std::distance(items.begin(), position);
		
		beginRemoveRows(QModelIndex(), row, row);
		items.erase(position);
		endRemoveRows();
		return true;
	}
	return false;
}
