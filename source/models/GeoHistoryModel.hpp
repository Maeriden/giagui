#ifndef GIAGUI_GEOHISTORYMODEL_HPP
#define GIAGUI_GEOHISTORYMODEL_HPP


#include <QAbstractTableModel>
#include "SimulationConfig.hpp"


struct GeoHistoryModel : public QAbstractTableModel
{
public:
	using HistoryEntry = SimulationConfig::Load::HistoryEntry;
	std::vector<HistoryEntry> items;
	using Iterator = decltype(items)::iterator;
	
	explicit GeoHistoryModel(QObject* parent = nullptr);
	
	int           rowCount() const;
	int           columnCount() const;
	int           rowCount(const QModelIndex& parent) const override;
	int           columnCount(const QModelIndex& parent) const override;
	QVariant      data(const QModelIndex& index, int role) const override;
	QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
	bool          setData(const QModelIndex& index, const QVariant& value, int role) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;
	bool          insertRows(int row, int count, const QModelIndex& parent) override;
	bool          removeRows(int row, int count, const QModelIndex& parent) override;
	
	HistoryEntry* get(const QModelIndex& modelIndex);
	QModelIndex   findIndex(HistoryEntry* item);
	bool          appendItem(HistoryEntry* dataset);
	bool          removeItem(HistoryEntry* dataset);
	
	Iterator begin();
	Iterator end();
};

#endif //GIAGUI_GEOHISTORYMODEL_HPP
