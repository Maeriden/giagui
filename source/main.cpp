#include <QApplication>

#include "Dataset.hpp"
#include "MapWindow.hpp"


/**********************************************************************
 * For any question write to bondi.1701014@studenti.uniroma1.it
 *********************************************************************/


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationDisplayName(QApplication::tr("GIA gui"));
	
	MapWindow mainWindow;
	mainWindow.show();
	
	return QApplication::exec();
}

