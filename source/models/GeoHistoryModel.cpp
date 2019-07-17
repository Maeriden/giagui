#include "GeoHistoryModel.hpp"

#include <cassert>


GeoHistoryModel::GeoHistoryModel(QObject* parent) : QAbstractTableModel(parent)
{
	
}


int GeoHistoryModel::rowCount() const
{
	return items.size();
}


int GeoHistoryModel::columnCount() const
{
	return 2;
}


int GeoHistoryModel::rowCount(const QModelIndex& parent) const
{
	return items.size();
}


int GeoHistoryModel::columnCount(const QModelIndex& parent) const
{
	return 2;
}


QVariant GeoHistoryModel::data(const QModelIndex& index, int role) const
{
	if(!index.isValid())
		return QVariant();
	if(role != Qt::ItemDataRole::DisplayRole && role != Qt::ItemDataRole::EditRole)
		return QVariant();
	
	const HistoryEntry& item = items[index.row()];
	switch(index.column())
	{
		case 0:  return item.time;
		case 1:  return QString::fromStdString(item.filename);
		default: return QVariant();
	}
}


QVariant GeoHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation != Qt::Orientation::Horizontal)
		return QVariant();
	if(role != Qt::ItemDataRole::DisplayRole)
		return QVariant();
	switch(section)
	{
		case 0:  return tr("Time");
		case 1:  return tr("File Path");
		default: return QVariant();
	}
}


bool GeoHistoryModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if(role != Qt::ItemDataRole::EditRole)
		return false;
	HistoryEntry& item = items[index.row()];
	switch(index.column())
	{
		case 0: item.time     = value.toFloat();                break;
		case 1: item.filename = value.toString().toStdString(); break;
	}
	return true;
}


Qt::ItemFlags GeoHistoryModel::flags(const QModelIndex& index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}


bool GeoHistoryModel::insertRows(int row, int count, const QModelIndex& parent)
{
	beginInsertRows(QModelIndex(), row, row);
	items.insert(items.begin()+row, HistoryEntry());
	endInsertRows();
	return true;
}


bool GeoHistoryModel::removeRows(int row, int count, const QModelIndex& parent)
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


GeoHistoryModel::HistoryEntry* GeoHistoryModel::get(const QModelIndex& modelIndex)
{
	assert(modelIndex.isValid());
	int row = modelIndex.row();
	if(0 <= row && row < (int)items.size())
	{
		HistoryEntry* entry = &items[row];
		return entry;
	}
	return nullptr;
}


QModelIndex GeoHistoryModel::findIndex(HistoryEntry* entry)
{
	struct {
		HistoryEntry& entry;
		bool operator()(const HistoryEntry& other) { return other.time == entry.time && other.filename == entry.filename; }
	} predicate = {*entry};
	
	auto iter = std::find_if(items.begin(), items.end(), predicate);
	if(iter != items.end())
	{
		int row = iter - items.begin();
		QModelIndex result = createIndex(row, 0, entry);
		return result;
	}
	return createIndex(-1, 0, nullptr);
}


bool GeoHistoryModel::appendItem(HistoryEntry* entry)
{
	assert(entry);
	bool result = insertRows(items.size(), 1, QModelIndex());
	items.back() = *entry;
	return result;
}


bool GeoHistoryModel::removeItem(HistoryEntry* entry)
{
	bool result = false;
	auto iter = std::find_if(items.begin(), items.end(), [&a = *entry](const HistoryEntry& b){
		return a.time == b.time && a.filename == b.filename;
	});
	if(iter != items.end())
	{
		int row = iter - items.begin();
		assert(0 <= row && row < (int)items.size());
		result = removeRows(row, 1, QModelIndex());
	}
	return result;
}


GeoHistoryModel::Iterator GeoHistoryModel::begin()
{
	return items.begin();
}


GeoHistoryModel::Iterator GeoHistoryModel::end()
{
	return items.end();
}
