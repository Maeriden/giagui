#include "DatasetControlWidget.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>

#include "Dataset.hpp"
#include "MapUtils.hpp"


DatasetControlWidget::DatasetControlWidget(QWidget* parent) : QWidget(parent)
{
	integerValidator.setRange(0, std::numeric_limits<int>::max());
	doubleValidator.setRange(0, DOUBLE_MAX, UI_DOUBLE_PRECISION);
	
	QFormLayout* groupLayout = new QFormLayout(this);
	groupLayout->setContentsMargins(6, 3, 0, 6);
//	groupLayout->setFormAlignment(Qt::AlignTop);
	
	{	QLabel* label = new QLabel(this);
		label->setText(tr("Resolution"));
		
		resolutionSpinBox = new QSpinBox(this);
		resolutionSpinBox->setMaximumWidth(110);
		resolutionSpinBox->setAlignment(Qt::AlignRight | Qt::AlignBaseline);
//		resolutionSpinBox->setFocusPolicy(Qt::FocusPolicy::NoFocus);
		resolutionSpinBox->setRange(0, MAX_SUPPORTED_RESOLUTION);
		resolutionSpinBox->setEnabled(false);
		QObject::connect(resolutionSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &DatasetControlWidget::onResolutionSpinboxChanged);
		
		groupLayout->addRow(label, resolutionSpinBox);
	}
	
	{	QLabel* label = new QLabel(this);
		label->setText(tr("Default"));
		
		defaultLineEdit = new QLineEdit(this);
		defaultLineEdit->setMaximumWidth(110);
		defaultLineEdit->setAlignment(Qt::AlignRight | Qt::AlignBaseline);
//		defaultLineEdit->setFocusPolicy(Qt::FocusPolicy::NoFocus);
//		defaultLineEdit->setValidator(&doubleValidator);
		defaultLineEdit->setEnabled(false);
		QObject::connect(defaultLineEdit, &QLineEdit::editingFinished, this, &DatasetControlWidget::onDefaultEditFinished);
		
		groupLayout->addRow(label, defaultLineEdit);
	}
	
	{	QLabel* label = new QLabel(this);
		label->setText(tr("Density"));
		
		densityLineEdit = new QLineEdit(this);
		densityLineEdit->setMaximumWidth(110);
		densityLineEdit->setAlignment(Qt::AlignRight | Qt::AlignBaseline);
//		densityLineEdit->setFocusPolicy(Qt::FocusPolicy::NoFocus);
//		densityLineEdit->setValidator(&doubleValidator);
		densityLineEdit->setPlaceholderText(tr("kg/m³"));
		densityLineEdit->setEnabled(false);
		QObject::connect(densityLineEdit, &QLineEdit::editingFinished, this, &DatasetControlWidget::onDensityEditFinished);
		
		groupLayout->addRow(label, densityLineEdit);
	}
	
	{	QLabel* label = new QLabel(this);
		label->setText(tr("Minimum"));
		
		minValueLineEdit = new QLineEdit(this);
		minValueLineEdit->setMaximumWidth(110);
		minValueLineEdit->setAlignment(Qt::AlignRight | Qt::AlignBaseline);
//		minValueLineEdit->setFocusPolicy(Qt::FocusPolicy::NoFocus);
//		minValueLineEdit->setValidator(&doubleValidator);
		minValueLineEdit->setEnabled(false);
		QObject::connect(minValueLineEdit, &QLineEdit::editingFinished, this, &DatasetControlWidget::onMinValueEditFinished);
		
		groupLayout->addRow(label, minValueLineEdit);
	}
	
	{	QLabel* label = new QLabel(this);
		label->setText(tr("Maximum"));
		
		maxValueLineEdit = new QLineEdit(this);
		maxValueLineEdit->setMaximumWidth(110);
		maxValueLineEdit->setAlignment(Qt::AlignRight | Qt::AlignBaseline);
//		maxValueLineEdit->setFocusPolicy(Qt::FocusPolicy::NoFocus);
//		maxValueLineEdit->setValidator(&doubleValidator);
		maxValueLineEdit->setEnabled(false);
		QObject::connect(maxValueLineEdit, &QLineEdit::editingFinished, this, &DatasetControlWidget::onMaxValueEditFinished);
		
		groupLayout->addRow(label, maxValueLineEdit);
	}
}


void DatasetControlWidget::setDataSource(Dataset* dataset)
{
	if(this->dataset == dataset)
		return;
	this->dataset = dataset;
	refreshViews();
}


void DatasetControlWidget::refreshViews()
{
	resolutionSpinBox->blockSignals(true);
	defaultLineEdit->blockSignals(true);
	densityLineEdit->blockSignals(true);
	minValueLineEdit->blockSignals(true);
	maxValueLineEdit->blockSignals(true);
	
	if(dataset)
	{
		resolutionSpinBox->setEnabled(true);
		defaultLineEdit->setEnabled(true);
		densityLineEdit->setEnabled(dataset->hasDensity());
		minValueLineEdit->setEnabled(true);
		maxValueLineEdit->setEnabled(true);
		
		QString measureUnit = QString::fromStdString(dataset->measureUnit);
		defaultLineEdit->setPlaceholderText(measureUnit);
		minValueLineEdit->setPlaceholderText(measureUnit);
		maxValueLineEdit->setPlaceholderText(measureUnit);
		
		resolutionSpinBox->setValue(dataset->resolution);
		
		if(dataset->hasDensity())
		{
			densityLineEdit->setText(QString::number(dataset->density, 'f', densityDecimals));
		}
		else
		{
			densityLineEdit->clear();
		}
		
		if(dataset->isInteger)
		{
			defaultLineEdit->setValidator(&integerValidator);
			minValueLineEdit->setValidator(&integerValidator);
			maxValueLineEdit->setValidator(&integerValidator);
			
			defaultLineEdit->setText(QString::number(dataset->defaultValue.integer));
			minValueLineEdit->setText(QString::number(dataset->minValue.integer));
			maxValueLineEdit->setText(QString::number(dataset->maxValue.integer));
		}
		else
		{
			defaultLineEdit->setValidator(&doubleValidator);
			minValueLineEdit->setValidator(&doubleValidator);
			maxValueLineEdit->setValidator(&doubleValidator);
			
			defaultLineEdit->setText(QString::number(dataset->defaultValue.real, 'f', defaultDecimals));
			minValueLineEdit->setText(QString::number(dataset->minValue.real,    'f', minValueDecimals));
			maxValueLineEdit->setText(QString::number(dataset->maxValue.real,    'f', maxValueDecimals));
		}
	}
	else
	{
		resolutionSpinBox->clear();
		defaultLineEdit->clear();
		densityLineEdit->clear();
		minValueLineEdit->clear();
		maxValueLineEdit->clear();
		
		resolutionSpinBox->setEnabled(false);
		defaultLineEdit->setEnabled(false);
		densityLineEdit->setEnabled(false);
		minValueLineEdit->setEnabled(false);
		maxValueLineEdit->setEnabled(false);
	}
	
	resolutionSpinBox->blockSignals(false);
	defaultLineEdit->blockSignals(false);
	densityLineEdit->blockSignals(false);
	minValueLineEdit->blockSignals(false);
	maxValueLineEdit->blockSignals(false);
}


void DatasetControlWidget::onResolutionSpinboxChanged(int value)
{
	assert(dataset);
	
	if(dataset->resolution != value)
		emit resolutionChanged(dataset, value);
}


void DatasetControlWidget::onDefaultEditFinished()
{
	assert(dataset);
	QString text = defaultLineEdit->text();
	
	bool isValidNumber;
	GeoValue value = toGeoValue(text, dataset->isInteger, &isValidNumber);
	if(isValidNumber)
	{
		if(!dataset->geoValuesAreEqual(dataset->defaultValue, value))
			emit defaultChanged(dataset, value);
	}
}


void DatasetControlWidget::onDensityEditFinished()
{
	assert(dataset);
	assert(dataset->hasDensity());
	QString text = densityLineEdit->text();
	
	bool isValidNumber;
	double value = text.toDouble(&isValidNumber);
	if(isValidNumber)
	{
		if(dataset->density != value)
			emit densityChanged(dataset, value);
	}
}


void DatasetControlWidget::onMinValueEditFinished()
{
	assert(dataset);
	QString textMin = minValueLineEdit->text();
	QString textMax = maxValueLineEdit->text();
	
	// TODO: Clamp values
	bool isValidNumber;
	GeoValue minValue = toGeoValue(textMin, dataset->isInteger, &isValidNumber);
	if(isValidNumber)
	{
		GeoValue maxValue = toGeoValue(textMax, dataset->isInteger, &isValidNumber);
		if(isValidNumber)
		{
			if(dataset->isInteger)
			{
				if(maxValue.integer < minValue.integer)
				{
					maxValue.integer = minValue.integer;
					
					maxValueLineEdit->blockSignals(true);
					maxValueLineEdit->setText(QString::number(maxValue.integer));
					maxValueLineEdit->blockSignals(false);
				}
				
				if(dataset->minValue.integer != minValue.integer
				|| dataset->maxValue.integer != maxValue.integer)
					emit valueRangeChanged(dataset, minValue, maxValue);
			}
			else
			{
				if(maxValue.real < minValue.real)
				{
					maxValue.real = minValue.real;
					
					maxValueLineEdit->blockSignals(true);
					maxValueLineEdit->setText(QString::number(maxValue.real, 'f', UI_DOUBLE_PRECISION));
					maxValueLineEdit->blockSignals(false);
				}
				
				if(dataset->minValue.real != minValue.real
				|| dataset->maxValue.real != maxValue.real)
					emit valueRangeChanged(dataset, minValue, maxValue);
			}
		}
	}
}


void DatasetControlWidget::onMaxValueEditFinished()
{
	assert(dataset);
	QString textMin = minValueLineEdit->text();
	QString textMax = maxValueLineEdit->text();
	
	// TODO: Clamp values
	bool isValidNumber;
	GeoValue minValue = toGeoValue(textMin, dataset->isInteger, &isValidNumber);
	if(isValidNumber)
	{
		GeoValue maxValue = toGeoValue(textMax, dataset->isInteger, &isValidNumber);
		if(isValidNumber)
		{
			if(dataset->isInteger)
			{
				if(minValue.integer > maxValue.integer)
				{
					minValue.integer = maxValue.integer;
					
					minValueLineEdit->blockSignals(true);
					minValueLineEdit->setText(QString::number(minValue.integer));
					minValueLineEdit->blockSignals(false);
				}
				
				if(dataset->minValue.integer != minValue.integer
				|| dataset->maxValue.integer != maxValue.integer)
					emit valueRangeChanged(dataset, minValue, maxValue);
			}
			else
			{
				if(minValue.real > maxValue.real)
				{
					minValue.real = maxValue.real;
					
					minValueLineEdit->blockSignals(true);
					minValueLineEdit->setText(QString::number(minValue.real, 'f', UI_DOUBLE_PRECISION));
					minValueLineEdit->blockSignals(false);
				}
				
				if(dataset->minValue.real != minValue.real
				|| dataset->maxValue.real != maxValue.real)
					emit valueRangeChanged(dataset, minValue, maxValue);
			}
		}
	}
}
