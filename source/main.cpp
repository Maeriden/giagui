#include "MainWindow.hpp"
#include <QApplication>

// Polyfill issue
// https://github.com/uber/h3-js/issues/24

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.show();

	return QApplication::exec();
}

