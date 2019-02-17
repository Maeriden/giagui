#include "MainWindow.hpp"
#include "MapWindow.hpp"
#include <QFileDialog>
#include <QMessageBox>


int exportSimulation(const char* filePath, SimulationData* data);
int importSimulation(const char* filePath, SimulationData* data);


class NumberValidator : public QDoubleValidator
{
public:
	explicit NumberValidator(int decimals, QObject* parent = nullptr) : QDoubleValidator(-DOUBLE_MAX, DOUBLE_MAX, decimals, parent) {}
	
	// Make empty strings a valid "number" (to represent NaN values)
	State validate(QString& input, int& pos) const override
	{
		if(input.isEmpty())
			return QValidator::Acceptable;
		return QDoubleValidator::validate(input, pos);
	}
};


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
	mapWindow(nullptr),
	simulationData(new SimulationData{}),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	this->setupUi();
}


void MainWindow::setupUi()
{
	connect(ui->actionOpen,   &QAction::triggered, this, &MainWindow::onActionOpenFile);
	connect(ui->actionSave,   &QAction::triggered, this, &MainWindow::onActionSaveFile);
	connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::onActionSaveFileAs);
	connect(ui->actionEditor, &QAction::triggered, this, &MainWindow::onActionOpenEditor);
	
	ui->editPower->setValidator(new NumberValidator(3));
	ui->editInnerValue->setValidator(new NumberValidator(3));
	ui->editOuterValue->setValidator(new NumberValidator(3));
	
	connect(ui->editPower,      &QLineEdit::editingFinished, this, &MainWindow::onEditingFinishedPower);
	connect(ui->editOutput,     &QLineEdit::editingFinished, this, &MainWindow::onEditingFinishedOutput);
	connect(ui->editInnerValue, &QLineEdit::editingFinished, this, &MainWindow::onEditingFinishedInnerValue);
	connect(ui->editOuterValue, &QLineEdit::editingFinished, this, &MainWindow::onEditingFinishedOuterValue);
	connect(ui->editOuterInput, &QLineEdit::editingFinished, this, &MainWindow::onEditingFinishedOuterInput);
}


void MainWindow::onActionOpenFile()
{
	QString caption = trUtf8("Import data");
	QString cwd     = QString();
	QString filter  = trUtf8("TOML (*.toml);;All Files (*)");
	QString filePath = QFileDialog::getOpenFileName(this, caption, cwd, filter);
	if(filePath.isEmpty())
		return;
	
	SimulationData simulationData = {};
	int error = importSimulation(filePath.toUtf8().constData(), &simulationData);
	if(error == 0)
	{
		this->exportPath = filePath;
		
		ui->editPower->setText(QString::number(simulationData.power));
		ui->editOutput->setText(simulationData.output);
		ui->editInnerValue->setText(QString::number(simulationData.innerValue));
		ui->editOuterValue->setText(QString::number(simulationData.outerValue));
		ui->editOuterInput->setText(simulationData.outerInput);
	}
}


void MainWindow::onActionSaveFile()
{
	if(this->exportPath.isEmpty())
	{
		this->onActionSaveFileAs();
		return;
	}
	
	int error = exportSimulation(this->exportPath.toUtf8().constData(), this->simulationData);
	if(error == 0)
	{
		setWindowModified(false);
	}
	if(error == 1)
	{
		QMessageBox::information(this, trUtf8("Error"), trUtf8("Unable to save file"));
	}
	else if(error == 2)
	{
		QMessageBox::information(this, trUtf8("Error"), trUtf8("I/O error while writing data"));
	}
}


void MainWindow::onActionSaveFileAs()
{
	QString caption = trUtf8("Export data");
	QString cwd     = QString();
	QString filter  = trUtf8("TOML (*.toml);;All Files (*)");
	// NOTE: using QFileDialog::getSaveFileName() does not automatically append the extension
	QFileDialog saveDialog(this, caption, cwd, filter);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.setDefaultSuffix(QString::fromUtf8("toml"));
	int result = saveDialog.exec();
	if(result != QDialog::Accepted)
		return;
	QString filePath = saveDialog.selectedFiles().first();
	
	this->exportPath = filePath;
	this->setWindowTitle(tr("GIA gui - %1").arg(this->exportPath));
	this->onActionSaveFile();
}


void MainWindow::onActionOpenEditor()
{
	if(this->mapWindow)
	{
		assert(this->mapWindow->isVisible());
		return;
	}
	this->mapWindow = new MapWindow(this);
	this->mapWindow->setAttribute(Qt::WA_DeleteOnClose, true);
	connect(this->mapWindow, &QMainWindow::destroyed, this, &MainWindow::onDestroyedMapWindow);
	this->mapWindow->show();
}


void MainWindow::onEditingFinishedPower()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(sender());
	this->simulationData->power = lineEdit->text().toDouble();
}


void MainWindow::onEditingFinishedOutput()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(sender());
	this->simulationData->output = lineEdit->text().toLocal8Bit().constData();
}


void MainWindow::onEditingFinishedInnerValue()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(sender());
	this->simulationData->innerValue = lineEdit->text().toDouble();
}


void MainWindow::onEditingFinishedOuterValue()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(sender());
	this->simulationData->outerValue = lineEdit->text().toDouble();
}


void MainWindow::onEditingFinishedOuterInput()
{
	QLineEdit* lineEdit = static_cast<QLineEdit*>(sender());
	this->simulationData->outerInput = lineEdit->text().toLocal8Bit().constData();
}


void MainWindow::onDestroyedMapWindow(QObject* widget)
{
	assert(widget == this->mapWindow);
	this->mapWindow = nullptr;
}


#include <cpptoml.h>
#include <fstream>


int importSimulation(const char* filePath, SimulationData* data)
{
	try
	{
		std::shared_ptr<cpptoml::table> root = cpptoml::parse_file(filePath);
		data->power  = root->get_qualified_as<double>("mesh.power").value_or(0.0);
		data->output = root->get_qualified_as<std::string>("mesh.output").value_or("").c_str();
		data->innerValue = root->get_qualified_as<double>("mesh.inner.value").value_or(0.0);
		data->outerValue = root->get_qualified_as<double>("mesh.outer.value").value_or(0.0);
		data->outerInput = root->get_qualified_as<std::string>("mesh.outer.input").value_or("").c_str();
	}
	catch(cpptoml::parse_exception& ex)
	{
		return 2;
	}
	return 0;
}


int exportSimulation(const char* filePath, SimulationData* data)
{
	std::ofstream stream(filePath);
	if(!stream.is_open())
		return 1;
	
	try
	{
		stream << "[mesh]"       << std::endl;
		stream << "power = "     << data->power      <<        std::endl;
		stream << "output = '"   << data->output     << "'" << std::endl;
		stream << std::endl;
		stream << "[mesh.inner]" << std::endl;
		stream << "value = "     << data->innerValue <<        std::endl;
		stream << std::endl;
		stream << "[mesh.outer]" << std::endl;
		stream << "value = "     << data->outerValue <<        std::endl;
		stream << "input = '"    << data->outerInput << "'" << std::endl;
	}
	catch(std::ofstream::failure& ex)
	{
		return 2;
	}
	return 0;
}
