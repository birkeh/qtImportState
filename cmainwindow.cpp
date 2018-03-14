#include "cmainwindow.h"
#include "ui_cmainwindow.h"

#include <QIcon>
#include <QMenu>
#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QList>
#include <QStandardItem>
#include <QScrollBar>


cMainWindow::cMainWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::cMainWindow),
	m_lpTrayIcon(0),
	m_lpTrayIconMenu(0),
	m_lpTimer(0),
	m_bMayUpdate(false)
{
	ui->setupUi(this);

	m_lpImportModel			= new QStandardItemModel(0, 1);
	ui->m_lpImportList->setModel(m_lpImportModel);

	m_lpPrepareModel			= new QStandardItemModel(0, 1);
	ui->m_lpPrepareList->setModel(m_lpPrepareModel);

	m_lpGenerateModel			= new QStandardItemModel(0, 1);
	ui->m_lpGenerateList->setModel(m_lpGenerateModel);

	createActions();
	createTrayIcon();

	connect(m_lpTrayIcon, &QSystemTrayIcon::activated, this, &cMainWindow::iconActivated);

	QIcon icon = QIcon(":/images/qtImportState.png");
	m_lpTrayIcon->setIcon(icon);
	setWindowIcon(icon);

	m_lpTrayIcon->setToolTip("qtImportState");
	m_lpTrayIcon->show();

	m_db	= QSqlDatabase::addDatabase("QMYSQL", "mysql");
	m_db.setHostName("10.69.208.60");
	m_db.setDatabaseName("reporting");
	m_db.setUserName("reporting");
	m_db.setPassword("reporting");
	if(!m_db.open())
		qDebug() << m_db.lastError().text();

	ui->m_lpMainTab->setCurrentIndex(0);

	m_bMayUpdate	= true;
	timerUpdate();
	for(int x = 0;x < 7;x++)
		ui->m_lpGenerateList->resizeColumnToContents(x);
	m_bMayUpdate	= false;

	m_lpTimer	= new QTimer(this);
	connect(m_lpTimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	m_lpTimer->start(10000);

	m_bMayUpdate	= true;
}

cMainWindow::~cMainWindow()
{
	if(m_db.isOpen())
		m_db.close();

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
		//QMessageBox::information(this, tr("Systray"), tr("The program will keep running in the system tray. To terminate the program, choose <b>Quit</b> in the context menu of the system tray entry."));
		hide();
		event->ignore();
	}
}

void cMainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
		break;
	case QSystemTrayIcon::DoubleClick:
		QWidget::showNormal();
		break;
	case QSystemTrayIcon::MiddleClick:
		break;
	default:
		;
	}
}

void cMainWindow::timerUpdate()
{
	if(!m_bMayUpdate)
		return;

	m_bMayUpdate			= false;

	updateImport();
	updatePrepare();
	updateGenerate();

	m_bMayUpdate	= true;
}

void cMainWindow::updateImport()
{
}

void cMainWindow::updatePrepare()
{
}

void cMainWindow::updateGenerate()
{
	QModelIndex		index		= ui->m_lpGenerateList->indexAt(ui->m_lpGenerateList->rect().topLeft());
	qint32			row			= index.row();
	qint32			selected	= -1;
	qint32			col[7];

	for(int x = 0;x < 7;x++)
		col[x]	= ui->m_lpGenerateList->columnWidth(x);

	index	= ui->m_lpGenerateList->currentIndex();

	if(index.isValid())
		selected	= index.row();

	m_lpGenerateModel->clear();

	QStringList	header;
	header << "nr" << "group" << "table" << "start" << "finish" << "duration" << "estimated";

	m_lpGenerateModel->setHorizontalHeaderLabels(header);

	if(!m_db.isOpen())
	{
		m_bMayUpdate	= true;
		return;
	}

	QSqlQuery*	lpQuery	= new QSqlQuery(m_db);

	lpQuery->prepare("SELECT		@rowid:=@rowid+1 AS number, "
					 "              t1.group_name AS group_name, "
					 "              t1.table_name AS table_name, "
					 "              CAST(IFNULL(DATE_FORMAT(t1.current_start, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) AS current_start, "
					 "              CAST(IFNULL(DATE_FORMAT(t1.current_finish, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) AS current_finish, "
					 "              CAST(IFNULL(t1.current_duration, '') AS CHAR) AS current_duration, "
					 "              CAST(IFNULL(DATE_FORMAT(t1.estimated, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) AS estimated "
					 "FROM		    v_imp_log_estimation AS t1, "
					 "              (SELECT @rowid:=0) AS init "
					 "WHERE		    t1.current_start >= "
					 "              ( "
					 "              	SELECT	MAX(start) "
					 "              	FROM	imp_log "
					 "              	WHERE	table_name = 'rep_UKMS_KFZ_transaction_overall_M' "
					 "              ) OR "
					 "              t1.current_start IS NULL");
	if(!lpQuery->exec())
	{
		m_bMayUpdate	= true;
		qDebug() << lpQuery->lastError().text();
		delete lpQuery;
		return;
	}

	while(lpQuery->next())
	{
		QList<QStandardItem*>	items;

		items.append(new QStandardItem(lpQuery->value("number").toString()));
		items.append(new QStandardItem(lpQuery->value("group_name").toString()));
		items.append(new QStandardItem(lpQuery->value("table_name").toString()));
		items.append(new QStandardItem(lpQuery->value("current_start").toString()));
		items.append(new QStandardItem(lpQuery->value("current_finish").toString()));
		items.append(new QStandardItem(lpQuery->value("current_duration").toString()));
		items.append(new QStandardItem(lpQuery->value("estimated").toString()));

		items[0]->setTextAlignment(Qt::AlignRight);

		m_lpGenerateModel->appendRow(items);
	}

	delete lpQuery;

	index	= m_lpGenerateModel->index(row, 0);

	ui->m_lpGenerateList->scrollTo(index, QAbstractItemView::PositionAtTop);

	if(selected != -1)
	{
		index	= m_lpGenerateModel->index(selected, 0);
		ui->m_lpGenerateList->setCurrentIndex(index);
	}

	for(int x = 0;x < 7;x++)
		ui->m_lpGenerateList->setColumnWidth(x, col[x]);
}
