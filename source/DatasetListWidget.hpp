#ifndef GIAGUI_DATASETLISTWIDGET_HPP
#define GIAGUI_DATASETLISTWIDGET_HPP

#include <QListView>


class QListView;
class QPushButton;
class QToolButton;

struct Dataset;
struct DatasetListModel;


class DatasetListWidget : public QWidget
{
	Q_OBJECT
public:
	DatasetListModel* datasets = nullptr;
	
	QListView*   listView     = nullptr;
	QToolButton* createButton = nullptr;
	QToolButton* deleteButton = nullptr;
	
	
	explicit DatasetListWidget(DatasetListModel* datasets, QWidget* parent = nullptr);
	
	Dataset* selection() const;
	
	
	void keyPressEvent(QKeyEvent* event) override;
	
	void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
	
	void onCreateDatasetClicked();
	void onCreateDatasetDialogFinished(int resultCode);
	
	void onDeleteDatasetClicked();
	void onDeleteDatasetDialogFinished(int resultCode);
	
	
signals:
	void itemCreated(Dataset* dataset);
	void itemSelected(Dataset* current, Dataset* previous);
	void itemDeleted(Dataset* dataset);
};


#endif //GIAGUI_DATASETLISTWIDGET_HPP
