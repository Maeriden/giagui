#include "MainWindow.hpp"
#include <QApplication>
#include "map.hpp"


H3State globalH3State;


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationDisplayName(QApplication::tr("GIA gui"));

	MainWindow w;
	w.show();

	return QApplication::exec();
}

