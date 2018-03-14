#include "cmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(qtImportState);

	QApplication a(argc, argv);
	QApplication::setQuitOnLastWindowClosed(false);

	cMainWindow w;
	w.show();

	return a.exec();
}
