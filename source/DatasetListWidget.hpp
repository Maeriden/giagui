#ifndef GIAGUI_DATASETLISTWIDGET_HPP
#define GIAGUI_DATASETLISTWIDGET_HPP

#include <QListView>


class QListView;
class QPushButton;
class QToolButton;

class Dataset;
class DatasetListModel;


class DatasetListWidget : public QWidget
{
	Q_OBJECT
	
protected:
	DatasetListModel* datasets = nullptr;
	
	QListView*   listView     = nullptr;
	QToolButton* createButton = nullptr;
	QToolButton* deleteButton = nullptr;
	
	
public:
	explicit DatasetListWidget(DatasetListModel* datasets, QWidget* parent = nullptr);
	
	Dataset* selection() const;
	
	
protected:
	void keyPressEvent(QKeyEvent* event) override;
	
	void onSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
	
	void onCreateDatasetClicked();
	void onCreateDatasetDialogFinished(int resultCode);
	
	void onDeleteDatasetClicked();
	void onDeleteDatasetDialogFinished(int resultCode);
	
	
signals:
	void itemCreated(Dataset* dataset);
	void itemSelected(Dataset* dataset);
	void itemDeleted(Dataset* dataset);
};


#endif //GIAGUI_DATASETLISTWIDGET_HPP
