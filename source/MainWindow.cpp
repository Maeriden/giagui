#include "MainWindow.hpp"



/*class NumberValidator : public QDoubleValidator
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
};*/


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
	this->setupUi();
}


void MainWindow::setupUi()
{
	if (this->objectName().isEmpty())
		this->setObjectName(QString::fromUtf8("MainWindow"));
	this->resize(480, 180);
	
	this->centralwidget = new QWidget(this);
	this->centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
	this->setCentralWidget(this->centralwidget);
	this->formLayout = new QFormLayout(this->centralwidget);
	this->formLayout->setObjectName(QString::fromUtf8("formLayout"));
	
	this->menubar = new QMenuBar(this);
	this->menubar->setObjectName(QString::fromUtf8("menubar"));
	this->menubar->setGeometry(QRect(0, 0, 480, 20));
	this->setMenuBar(this->menubar);
	
	this->menuFile = new QMenu(tr("File"), this->menubar);
	this->menuFile->setObjectName(QString::fromUtf8("menuFile"));
	{
		this->actionOpen = new QAction(QIcon::fromTheme(QString::fromUtf8("document-open")), tr("Open..."), this);
		this->actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
		this->actionOpen->setShortcuts(QKeySequence::StandardKey::Open);
		connect(this->actionOpen, &QAction::triggered, this, &MainWindow::onActionOpenFile);
		
		this->actionSave = new QAction(QIcon::fromTheme(QString::fromUtf8("document-save")), tr("Save"), this);
		this->actionSave->setObjectName(QString::fromUtf8("actionSave"));
		this->actionSave->setShortcuts(QKeySequence::StandardKey::Save);
		connect(this->actionSave, &QAction::triggered, this, &MainWindow::onActionSaveFile);
		
		this->actionSaveAs = new QAction(QIcon::fromTheme(QString::fromUtf8("document-save-as")), tr("Save As..."), this);
		this->actionSaveAs->setObjectName(QString::fromUtf8("actionSaveAs"));
		this->actionSaveAs->setShortcuts(QKeySequence::StandardKey::SaveAs);
		connect(this->actionSaveAs, &QAction::triggered, this, &MainWindow::onActionSaveFileAs);
		
		this->actionQuit = new QAction(QIcon::fromTheme(QString::fromUtf8("application-exit")), tr("Quit"), this);
		this->actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
		this->actionQuit->setShortcuts(QKeySequence::StandardKey::Quit);
		this->actionQuit->setMenuRole(QAction::QuitRole);
		connect(this->actionQuit, &QAction::triggered, this, &MainWindow::close);
		
		this->menubar->addAction(this->menuFile->menuAction());
		this->menuFile->addAction(this->actionOpen);
		this->menuFile->addAction(this->actionSave);
		this->menuFile->addAction(this->actionSaveAs);
		this->menuFile->addSeparator();
		this->menuFile->addAction(this->actionQuit);
	}
	
	this->menuTools = new QMenu(tr("Tools"), this->menubar);
	this->menuTools->setObjectName(QString::fromUtf8("menuTools"));
	{
		this->actionEditor = new QAction(tr("Open Mesh Editor..."), this);
		this->actionEditor->setObjectName(QString::fromUtf8("actionEditor"));
		connect(this->actionEditor, &QAction::triggered, this, &MainWindow::onActionOpenEditor);
		
		this->menubar->addAction(this->menuTools->menuAction());
		this->menuTools->addAction(this->actionEditor);
	}
	
	{
		this->labelPower = new QLabel(tr("Mesh Power"), this->centralwidget);
		this->labelPower->setObjectName(QString::fromUtf8("labelPower"));
		
		this->editPower = new QLineEdit(this->centralwidget);
		this->editPower->setObjectName(QString::fromUtf8("editPower"));
		//this->editPower->setValidator(new NumberValidator(3));
		this->editPower->setValidator(new DoubleValidator(3));
		
		connect(this->editPower, &QLineEdit::textEdited, this, &MainWindow::onTextEditedMeshPower);
		
		this->formLayout->setWidget(0, QFormLayout::LabelRole, labelPower);
		this->formLayout->setWidget(0, QFormLayout::FieldRole, editPower);
	}
	{
		this->labelOutput = new QLabel(tr("Mesh Output"), this->centralwidget);
		this->labelOutput->setObjectName(QString::fromUtf8("labelOutput"));
		
		this->editOutput = new QLineEdit(this->centralwidget);
		this->editOutput->setObjectName(QString::fromUtf8("editOutput"));
		connect(this->editOutput, &QLineEdit::textEdited, this, &MainWindow::onTextEditedMeshOutput);
		
		this->formLayout->setWidget(1, QFormLayout::LabelRole, labelOutput);
		this->formLayout->setWidget(1, QFormLayout::FieldRole, editOutput);
	}
	{
		this->labelInnerValue = new QLabel(tr("Mesh Inner Value"), this->centralwidget);
		this->labelInnerValue->setObjectName(QString::fromUtf8("labelInnerValue"));
		
		this->editInnerValue = new QLineEdit(this->centralwidget);
		this->editInnerValue->setObjectName(QString::fromUtf8("editInnerValue"));
		//this->editInnerValue->setValidator(new NumberValidator(3));
		this->editInnerValue->setValidator(new DoubleValidator(3));
		connect(this->editInnerValue, &QLineEdit::textEdited, this, &MainWindow::onTextEditedMeshInnerValue);
		
		this->formLayout->setWidget(2, QFormLayout::LabelRole, labelInnerValue);
		this->formLayout->setWidget(2, QFormLayout::FieldRole, editInnerValue);
	}
	{
		this->labelOuterValue = new QLabel(tr("Mesh Outer Value"), this->centralwidget);
		this->labelOuterValue->setObjectName(QString::fromUtf8("labelOuterValue"));
		
		this->editOuterValue = new QLineEdit(this->centralwidget);
		this->editOuterValue->setObjectName(QString::fromUtf8("editOuterValue"));
		//this->editOuterValue->setValidator(new NumberValidator(3));
		this->editInnerValue->setValidator(new DoubleValidator(3));
		connect(this->editOuterValue, &QLineEdit::textEdited, this, &MainWindow::onTextEditedMeshOuterValue);
		
		this->formLayout->setWidget(3, QFormLayout::LabelRole, labelOuterValue);
		this->formLayout->setWidget(3, QFormLayout::FieldRole, editOuterValue);
	}
	{
		this->labelOuterInput = new QLabel(tr("Mesh Outer Input"), this->centralwidget);
		this->labelOuterInput->setObjectName(QString::fromUtf8("labelOuterInput"));
		
		this->editOuterInput = new QLineEdit(this->centralwidget);
		this->editOuterInput->setObjectName(QString::fromUtf8("editOuterInput"));
		connect(this->editOuterInput, &QLineEdit::textEdited, this, &MainWindow::onTextEditedMeshOuterInput);
		
		this->formLayout->setWidget(4, QFormLayout::LabelRole, labelOuterInput);
		this->formLayout->setWidget(4, QFormLayout::FieldRole, editOuterInput);
	}
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	bool confirmed = !isWindowModified();
	if(!confirmed)
	{
		QString title    = QString();
		QString question = tr("There are unsaved changes. Close anyway?");
		int reply = QMessageBox::question(this, title, question);
		confirmed = reply == QMessageBox::Yes;
	}
	
	if(confirmed)
	{
		event->accept();
	}
	else
	{
		event->ignore();
	}
}


void MainWindow::onActionOpenFile()
{
	QString caption = tr("Import data");
	QString cwd     = QString();
	QString filter  = tr("TOML (*.toml);;All Files (*)");
	QString dialogPath = QFileDialog::getOpenFileName(this, caption, cwd, filter);
	if(dialogPath.isEmpty())
		return;
	
	const char* filePath = dialogPath.toUtf8().constData();
	SimulationData simulationData;
	int error = importSimulation(filePath, &simulationData);
	if(error == 0)
	{
		this->editPower->setText(QString::number(simulationData.power));
		this->editOutput->setText(QString::fromStdString(simulationData.output));
		this->editInnerValue->setText(QString::number(simulationData.innerValue));
		this->editOuterValue->setText(QString::number(simulationData.outerValue));
		this->editOuterInput->setText(QString::fromStdString(simulationData.outerInput));
		
		setWindowFilePath(dialogPath);
		setWindowModified(false);
	}
	else if(error == 1)
	{
		QMessageBox::information(this, tr("Error"), tr("Unable to open file"));
	}
	else if(error == 2)
	{
		QMessageBox::information(this, tr("Error"), tr("I/O error while reading data"));
	}
}


void MainWindow::onActionSaveFile()
{
	if(windowFilePath().isEmpty())
	{
		this->onActionSaveFileAs();
		return;
	}
	
	const char* filePath = windowFilePath().toUtf8().constData();
	SimulationData simulationData = {
		.power      = this->editPower->text().toDouble(),
		.output     = this->editOutput->text().toStdString(),
		.innerValue = this->editInnerValue->text().toDouble(),
		.outerValue = this->editOuterValue->text().toDouble(),
		.outerInput = this->editOuterInput->text().toStdString(),
	};
	
	int error = exportSimulation(filePath, &simulationData);
	if(error == 0)
	{
		setWindowModified(false);
	}
	if(error == 1)
	{
		QMessageBox::information(this, tr("Error"), tr("Unable to save file"));
	}
	else if(error == 2)
	{
		QMessageBox::information(this, tr("Error"), tr("I/O error while writing data"));
	}
}


void MainWindow::onActionSaveFileAs()
{
	QString caption = tr("Export data");
	QString cwd     = QString();
	QString filter  = tr("TOML (*.toml);;All Files (*)");
	// NOTE: using QFileDialog::getSaveFileName() does not automatically append the extension
	QFileDialog saveDialog(this, caption, cwd, filter);
	saveDialog.setAcceptMode(QFileDialog::AcceptSave);
	saveDialog.setDefaultSuffix(QString::fromUtf8("toml"));
	int result = saveDialog.exec();
	if(result != QDialog::Accepted)
		return;
	QString dialogPath = saveDialog.selectedFiles().first();
	
	setWindowFilePath(dialogPath);
	this->onActionSaveFile();
}


void MainWindow::onActionOpenEditor()
{
	if(this->mapWindow)
	{
		assert(this->mapWindow->isVisible());
		return;
	}
	this->mapWindow = new MapWindow();
	this->mapWindow->setAttribute(Qt::WA_DeleteOnClose, true);
	this->mapWindow->setWindowModality(Qt::ApplicationModal);
	connect(this->mapWindow, &QMainWindow::destroyed, this, &MainWindow::onDestroyedMapWindow);
	this->mapWindow->show();
}


void MainWindow::onTextEditedMeshPower(const QString& text)
{
	if(!windowFilePath().isEmpty())
		setWindowModified(true);
}


void MainWindow::onTextEditedMeshOutput(const QString& text)
{
	if(!windowFilePath().isEmpty())
		setWindowModified(true);
}


void MainWindow::onTextEditedMeshInnerValue(const QString& text)
{
	if(!windowFilePath().isEmpty())
		setWindowModified(true);
}


void MainWindow::onTextEditedMeshOuterValue(const QString& text)
{
	if(!windowFilePath().isEmpty())
		setWindowModified(true);
}


void MainWindow::onTextEditedMeshOuterInput(const QString& text)
{
	if(!windowFilePath().isEmpty())
		setWindowModified(true);
}


void MainWindow::onDestroyedMapWindow(QObject* widget)
{
	assert(widget == this->mapWindow);
	this->mapWindow = nullptr;
}





int importSimulation(const char* filePath, SimulationData* data)
{
	try
	{
		std::shared_ptr<cpptoml::table> root = cpptoml::parse_file(filePath);
		data->power      = root->get_qualified_as<double>("mesh.power").value_or(0.0);
		data->output     = root->get_qualified_as<std::string>("mesh.output").value_or("");
		data->innerValue = root->get_qualified_as<double>("mesh.inner.value").value_or(0.0);
		data->outerValue = root->get_qualified_as<double>("mesh.outer.value").value_or(0.0);
		data->outerInput = root->get_qualified_as<std::string>("mesh.outer.input").value_or("");
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
