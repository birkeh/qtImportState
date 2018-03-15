#include "cmainwindow.h"
#include <QApplication>
#include <QSettings>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(qtImportState);

	QApplication a(argc, argv);
	QApplication::setQuitOnLastWindowClosed(false);

	QCoreApplication::setOrganizationName("WIN-DESIGN");
	QCoreApplication::setOrganizationDomain("windesign.at");
	QCoreApplication::setApplicationName("qtImportState");

	QSettings	settings;

	cMainWindow w;
	if(settings.value("main/maximized").toBool())
		w.showMaximized();
	else
	{
		qint16	x		= settings.value("main/x", -1).toInt();
		qint16	y		= settings.value("main/y", -1).toInt();
		qint16	width	= settings.value("main/width", -1).toInt();
		qint16	height	= settings.value("main/height", -1).toInt();

		w.show();

		if(x != -1 && y != -1)
			w.move(x, y);

		if(width != -1 && height != -1)
			w.resize(width, height);
	}

	return a.exec();
}
