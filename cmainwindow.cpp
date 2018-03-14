#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>


cMainWindow::cMainWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::cMainWindow)
{
	ui->setupUi(this);

	createActions();
	createTrayIcon();

	connect(m_lpTrayIcon, &QSystemTrayIcon::messageClicked, this, &cMainWindow::messageClicked);
	connect(m_lpTrayIcon, &QSystemTrayIcon::activated, this, &cMainWindow::iconActivated);

	m_lpTrayIcon->setToolTip("BLA");
	m_lpTrayIcon->show();
}

cMainWindow::~cMainWindow()
{
	delete ui;
}

void cMainWindow::createActions()
{
	m_lpMinimizeAction = new QAction(tr("Mi&nimize"), this);
	connect(m_lpMinimizeAction, &QAction::triggered, this, &QWidget::hide);

	m_lpMaximizeAction = new QAction(tr("Ma&ximize"), this);
	connect(m_lpMaximizeAction, &QAction::triggered, this, &QWidget::showMaximized);

	m_lpRestoreAction = new QAction(tr("&Restore"), this);
	connect(m_lpRestoreAction, &QAction::triggered, this, &QWidget::showNormal);

	m_lpQuitAction = new QAction(tr("&Quit"), this);
	connect(m_lpQuitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void cMainWindow::createTrayIcon()
{
	m_lpTrayIconMenu = new QMenu(this);
	m_lpTrayIconMenu->addAction(m_lpMinimizeAction);
	m_lpTrayIconMenu->addAction(m_lpMaximizeAction);
	m_lpTrayIconMenu->addAction(m_lpRestoreAction);
	m_lpTrayIconMenu->addSeparator();
	m_lpTrayIconMenu->addAction(m_lpQuitAction);

	m_lpTrayIcon = new QSystemTrayIcon(this);
	m_lpTrayIcon->setContextMenu(m_lpTrayIconMenu);
}

void cMainWindow::setVisible(bool visible)
{
	m_lpMinimizeAction->setEnabled(visible);
	m_lpMaximizeAction->setEnabled(!isMaximized());
	m_lpRestoreAction->setEnabled(isMaximized() || !visible);
	QDialog::setVisible(visible);
}

void cMainWindow::closeEvent(QCloseEvent* event)
{
	if(m_lpTrayIcon->isVisible())
	{
		QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. To terminate the program, choose <b>Quit</b> in the context menu of the system tray entry."));
		hide();
		event->ignore();
	}
}

void cMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
//		iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1) % iconComboBox->count());
		break;
	case QSystemTrayIcon::MiddleClick:
//		showMessage();
		break;
	default:
		;
	}
}

void cMainWindow::messageClicked()
{
	QMessageBox::information(0, tr("Systray"),
							 tr("Sorry, I already gave what help I could.\n"
								"Maybe you should try asking a human?"));
}
