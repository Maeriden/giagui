#ifndef GIAGUI_SIMULATIONCONFIGDIALOG_HPP
#define GIAGUI_SIMULATIONCONFIGDIALOG_HPP

#include <QWidget>
#include <QDialog>
#include <QtGui/QStandardItemModel>
#include <source/Containers.hpp>
#include "SimulationConfig.hpp"


class QKeyEvent;
class QLineEdit;
class QComboBox;
class QToolButton;
class QListView;
struct Dataset;
struct DatasetListModel;
struct TimesModel;
struct TimeDatasetsModel;


class SimulationConfigDialog : public QDialog
{
public:
	SimulationConfig* configuration;
	
	TimesModel*        timesModel        = nullptr;
	TimeDatasetsModel* timeDatasetsModel = nullptr;
	
	QLineEdit*    innerValueEdit     = nullptr;
	QComboBox*    innerInputComboBox = nullptr;
	QLineEdit*    outerValueEdit     = nullptr;
	QComboBox*    outerInputComboBox = nullptr;
	QLineEdit*    stepsEdit          = nullptr;
	QLineEdit*    scalingEdit        = nullptr;
	QToolButton*  timesAddButton     = nullptr;
	QToolButton*  timesRemoveButton  = nullptr;
	QListView*    timesListView      = nullptr;
	QListView*    datasetsListView   = nullptr;
	
	
	explicit SimulationConfigDialog(DatasetListModel* datasets, SimulationConfig* configuration, QWidget* parent = nullptr);
	
	bool eventFilter(QObject* object, QEvent* event) override;
	void onTimeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
	void onAddHistoryItemClicked();
	void onRemoveHistoryItemClicked();
	void onOkClicked();
};


#endif //GIAGUI_SIMULATIONCONFIGDIALOG_HPP
