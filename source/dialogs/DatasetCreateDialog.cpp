#include "DatasetCreateDialog.hpp"

#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>

#include "Dataset.hpp"


DatasetCreateDialog::DatasetCreateDialog(QWidget* parent) : QDialog(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	{
		QFormLayout* argsLayout = new QFormLayout();
		{
			QLabel* label = new QLabel(this);
			label->setText(tr("Name"));
			
			nameEdit = new QLineEdit(this);
			nameEdit->setText(datasetName);
			QObject::connect(nameEdit, &QLineEdit::textChanged, this, &DatasetCreateDialog::onNameTextChanged);
			
			argsLayout->addRow(label, nameEdit);
		}
		{
			QLabel* label = new QLabel(this);
			label->setText(tr("Has Density"));
			
			densityCheckBox = new QCheckBox(this);
			densityCheckBox->setChecked(datasetHasDensity);
			
			argsLayout->addRow(label, densityCheckBox);
		}
		{
			QLabel* label = new QLabel(this);
			label->setText(tr("Is Integer"));
			
			integerCheckBox = new QCheckBox(this);
			integerCheckBox->setChecked(datasetIsInteger);
			
			argsLayout->addRow(label, integerCheckBox);
		}
		
		mainLayout->addLayout(argsLayout);
	}
	{
		QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
		
		okButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Ok);
		okButton->setEnabled(nameIsValid(nameEdit->text()));
		QObject::connect(okButton,     &QPushButton::clicked, this, &DatasetCreateDialog::onOkClicked);
		
		cancelButton = buttonBox->addButton(QDialogButtonBox::StandardButton::Cancel);
		QObject::connect(cancelButton, &QPushButton::clicked, this, &DatasetCreateDialog::reject);
		
		mainLayout->addWidget(buttonBox);
	}
}


bool DatasetCreateDialog::nameIsValid(const QString& datasetName)
{
	if(datasetName.size() > 0)
		return true;
	return false;
}


void DatasetCreateDialog::onNameTextChanged(const QString& text)
{
	okButton->setEnabled(nameIsValid(text));
}


void DatasetCreateDialog::onOkClicked()
{
	datasetName       = nameEdit->text();
	datasetHasDensity = densityCheckBox->isChecked();
	datasetIsInteger  = integerCheckBox->isChecked();
	accept();
}
