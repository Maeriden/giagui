#ifndef GIAGUI_DATASETCREATEDIALOG_HPP
#define GIAGUI_DATASETCREATEDIALOG_HPP

#include <QDialog>


class QLineEdit;
class QCheckBox;
class QPushButton;
class DatasetCollection;


class DatasetCreateDialog : public QDialog
{
	Q_OBJECT
	
public:
	QString  datasetName       = QString();
	bool     datasetHasDensity = true;
	bool     datasetIsInteger  = false;
	
	
protected:
	QLineEdit* nameEdit;
	QCheckBox* densityCheckBox;
	QCheckBox* integerCheckBox;
	
	QPushButton* okButton;
	QPushButton* cancelButton;
	
	
public:
	explicit DatasetCreateDialog(QWidget* parent = nullptr);
	
	
protected:
	bool nameIsValid(const QString& datasetName);
	void onNameTextChanged(const QString& text);
	void onOkClicked();
};


#endif //GIAGUI_DATASETCREATEDIALOG_HPP
