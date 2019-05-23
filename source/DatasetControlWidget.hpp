#ifndef GIAGUI_DATASETCONTROLWIDGET_HPP
#define GIAGUI_DATASETCONTROLWIDGET_HPP


#include <QWidget>
#include <QValidator>

#include "Dataset.hpp"


class QLineEdit;
class QSpinBox;
struct Dataset;


class DatasetControlWidget : public QWidget
{
Q_OBJECT

public:
	explicit DatasetControlWidget(QWidget* parent = nullptr);
	
	void setDataSource(Dataset* newDataset);
	void refreshViews();
	
	
protected:
	void onResolutionSpinboxChanged(int newResolution);
	void onDefaultEditFinished();
	void onDensityEditFinished();
	void onMinValueEditFinished();
	void onMaxValueEditFinished();
	
	
protected:
	Dataset* dataset = nullptr;
	
	QIntValidator    integerValidator;
	QDoubleValidator doubleValidator;
	
	QSpinBox*  resolutionSpinBox = nullptr;
	QLineEdit* defaultLineEdit   = nullptr;
	QLineEdit* densityLineEdit   = nullptr;
	QLineEdit* minValueLineEdit  = nullptr;
	QLineEdit* maxValueLineEdit  = nullptr;
	
	// NOTE: These were an attempt at preserving the input just as the user typed it
	// They are not really used for anything useful now
	int defaultDecimals  = 6;
	int densityDecimals  = 6;
	int minValueDecimals = 6;
	int maxValueDecimals = 6;
	
	
signals:
	void resolutionChanged(Dataset* dataset, int newResolution);
	void defaultChanged(Dataset* dataset, GeoValue newDefault);
	void densityChanged(Dataset* dataset, double newDensity);
	void valueRangeChanged(Dataset* dataset, GeoValue min, GeoValue max);
};


#endif //GIAGUI_DATASETCONTROLWIDGET_HPP
