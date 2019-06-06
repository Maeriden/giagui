#ifndef GIAGUI_DATASETCREATEDIALOG_HPP
#define GIAGUI_DATASETCREATEDIALOG_HPP

#include <QDialog>


class QLineEdit;
class QCheckBox;
class QPushButton;


class DatasetCreateDialog : public QDialog
{
public:
	QString  datasetName       = QString();
	bool     datasetHasDensity = true;
	bool     datasetIsInteger  = false;
	
	
	QLineEdit*   nameEdit;
	QCheckBox*   densityCheckBox;
	QCheckBox*   integerCheckBox;
	QPushButton* okButton;
	QPushButton* cancelButton;
	
	
	explicit DatasetCreateDialog(QWidget* parent = nullptr);
	
	bool nameIsValid(const QString& datasetName);
	void onNameTextChanged(const QString& text);
	void onOkClicked();
};


#endif //GIAGUI_DATASETCREATEDIALOG_HPP
