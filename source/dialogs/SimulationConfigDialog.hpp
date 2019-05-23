#ifndef GIAGUI_SIMULATIONCONFIGDIALOG_HPP
#define GIAGUI_SIMULATIONCONFIGDIALOG_HPP

#include <QWidget>
#include <QDialog>
#include "SimulationConfig.hpp"


class QKeyEvent;
class QLineEdit;
class QToolButton;
class QTableView;
class GeoHistoryModel;


class SimulationConfigDialog : public QDialog
{
Q_OBJECT
	
public:
	SimulationConfig configuration;
	QString          path;
	
public:
	explicit SimulationConfigDialog(QWidget* parent = nullptr);
	
	SimulationConfig::Load::HistoryEntry* selection() const;
	
	
protected:
	void keyPressEvent(QKeyEvent* event) override;
#if ENABLE_LINEEDIT_FILEDIALOG_ACTION
	void onMeshFileDialogActionTriggered();
	void onMeshFileDialogFinished(int resultCode);
#endif
	void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
	void onAddHistoryItemClicked();
	void onRemoveHistoryItemClicked();
	void onSaveClicked();
	void onSaveFileDialogAccepted();
	
	
protected:
	GeoHistoryModel* historyModel = nullptr;
	
	QLineEdit*    innerValueEdit = nullptr;
	QLineEdit*    innerInputEdit = nullptr;
	QLineEdit*    outerValueEdit = nullptr;
	QLineEdit*    outerInputEdit = nullptr;
	QLineEdit*    stepsEdit      = nullptr;
	QLineEdit*    scalingEdit    = nullptr;
	QTableView*   historyTable   = nullptr;
	
	QToolButton*  historyAddButton    = nullptr;
	QToolButton*  historyRemoveButton = nullptr;
};


#endif //GIAGUI_SIMULATIONCONFIGDIALOG_HPP
