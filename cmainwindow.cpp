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
#include <QTime>
#include <QSettings>


#define PROCESS_ROWS	4
#define IMPORT_ROWS		6
#define PREPARE_ROWS	7
#define	GENERATE_ROWS	7


cMainWindow::cMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::cMainWindow),
	m_lpTrayIcon(0),
	m_lpTrayIconMenu(0),
	m_lpTimer(0),
	m_bMayUpdate(false)
{
	ui->setupUi(this);

	QSettings	settings;
	qint16		iSplitter1	= settings.value("main/splitter1", -1).toInt();
	qint16		iSplitter2	= settings.value("main/splitter2", -1).toInt();
	if(iSplitter1 != -1 && iSplitter2 != -1)
		ui->m_lpMainSplitter->setSizes(QList<int>() << iSplitter1 << iSplitter2);

	m_lpProcessModel	= new QStandardItemModel(0, 1);
	ui->m_lpProcessList->setModel(m_lpProcessModel);

	m_lpImportModel		= new QStandardItemModel(0, 1);
	ui->m_lpImportList->setModel(m_lpImportModel);

	m_lpPrepareModel	= new QStandardItemModel(0, 1);
	ui->m_lpPrepareList->setModel(m_lpPrepareModel);

	m_lpGenerateModel	= new QStandardItemModel(0, 1);
	ui->m_lpGenerateList->setModel(m_lpGenerateModel);

	createActions();
	createTrayIcon();

	connect(m_lpTrayIcon, &QSystemTrayIcon::activated, this, &cMainWindow::iconActivated);

	QIcon icon = QIcon(":/images/qtImportState.png");
	m_lpTrayIcon->setIcon(icon);
	setWindowIcon(icon);

	m_lpTrayIcon->setToolTip("qtImportState");
	m_lpTrayIcon->show();

	connectDB();

	ui->m_lpMainTab->setCurrentIndex(0);

	m_bMayUpdate	= true;
	timerUpdate();
	for(int x = 0;x < PROCESS_ROWS;x++)
		ui->m_lpProcessList->resizeColumnToContents(x);
	for(int x = 0;x < IMPORT_ROWS;x++)
		ui->m_lpImportList->resizeColumnToContents(x);
	for(int x = 0;x < PREPARE_ROWS;x++)
		ui->m_lpPrepareList->resizeColumnToContents(x);
	for(int x = 0;x < GENERATE_ROWS;x++)
		ui->m_lpGenerateList->resizeColumnToContents(x);
	m_bMayUpdate	= false;

	setTimer();

	m_bMayUpdate	= true;
}

cMainWindow::~cMainWindow()
{
	QSettings	settings;
	settings.setValue("main/width", QVariant::fromValue(size().width()));
	settings.setValue("main/height", QVariant::fromValue(size().height()));
	settings.setValue("main/x", QVariant::fromValue(x()));
	settings.setValue("main/y", QVariant::fromValue(y()));
	if(this->isMaximized())
		settings.setValue("main/maximized", QVariant::fromValue(true));
	else
		settings.setValue("main/maximized", QVariant::fromValue(false));

	QList<int>sizes	= ui->m_lpMainSplitter->sizes();
	settings.setValue("main/splitter1", QVariant::fromValue(sizes[0]));
	settings.setValue("main/splitter2", QVariant::fromValue(sizes[1]));

	if(m_db.isOpen())
		m_db.close();

	if(m_lpTrayIcon)
		delete m_lpTrayIcon;

	if(m_lpTrayIconMenu)
		delete m_lpTrayIconMenu;

	if(m_lpTimer)
		delete m_lpTimer;

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
	QMainWindow::setVisible(visible);
}

void cMainWindow::closeEvent(QCloseEvent* event)
{
	if(m_lpTrayIcon->isVisible())
	{
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

void cMainWindow::connectDB()
{
	if(m_db.isOpen())
		return;

	QSettings	settings;
	QString		szHostName		= settings.value("database/hostName", "10.69.208.60").toString();
	QString		szDatabaseName	= settings.value("database/databaseName", "reporting").toString();
	QString		szUserName		= settings.value("database.userName", "reporting").toString();
	QString		szPassword		= settings.value("database/password", "reporting").toString();

	m_db	= QSqlDatabase::addDatabase("QMYSQL", "mysql");
	m_db.setHostName(szHostName);
	m_db.setDatabaseName(szDatabaseName);
	m_db.setUserName(szUserName);
	m_db.setPassword(szPassword);
	if(!m_db.open())
		qDebug() << m_db.lastError().text();
}

void cMainWindow::setTimer()
{
	QSettings	settings;
	qint32		iTimer	= settings.value("timer/timeout", 10000).toInt();

	if(!m_lpTimer)
	{
		m_lpTimer	= new QTimer(this);
		connect(m_lpTimer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
		m_lpTimer->start(iTimer);
	}
	else
	{
		m_lpTimer->stop();
		m_lpTimer->start(iTimer);
	}
}

void cMainWindow::timerUpdate()
{
	if(!m_bMayUpdate)
		return;

	m_bMayUpdate			= false;
	QSettings		settings;
	qint32			iTimer	= settings.value("timer/timeout", 10000).toInt();

	connectDB();
	updateProcessList();
	updateImport();
	updatePrepare();
	updateGenerate();

	ui->m_lpStatusBar->showMessage(QString("Last update: %1, next update: %2").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss")).arg(QDateTime::currentDateTime().addMSecs(iTimer).toString("dd.MM.yyyy HH:mm:ss")));

	m_bMayUpdate	= true;
}

void cMainWindow::updateProcessList()
{
	QSettings		settings;
	QModelIndex		index		= ui->m_lpProcessList->indexAt(ui->m_lpProcessList->rect().topLeft());
	qint32			row			= index.row();
	qint32			selected	= -1;
	qint32			col[PROCESS_ROWS];
	QString			szUser		= settings.value("process/user", "reporting").toString();
	QString			szHost		= settings.value("process/host", "localhost").toString();
	QString			szDB		= settings.value("process/db", "reporting").toString();
	QString			szCommand	= settings.value("process/command", "Query").toString();

	for(int x = 0;x < PROCESS_ROWS;x++)
		col[x]	= ui->m_lpProcessList->columnWidth(x);

	index	= ui->m_lpProcessList->currentIndex();

	if(index.isValid())
		selected	= index.row();

	m_lpProcessModel->clear();

	QStringList	header;
	header << "command" << "time" << "state" << "info";

	m_lpProcessModel->setHorizontalHeaderLabels(header);

	if(!m_db.isOpen())
	{
		m_bMayUpdate	= true;
		return;
	}

	QSqlQuery*	lpQuery	= new QSqlQuery(m_db);

	lpQuery->prepare(QString("SELECT ID, USER, HOST, DB, COMMAND, TIME, STATE, INFO FROM INFORMATION_SCHEMA.PROCESSLIST WHERE db = '%1';").arg(szDB));
	if(!lpQuery->exec())
	{
		m_bMayUpdate	= true;
		qDebug() << lpQuery->lastError().text();
		delete lpQuery;
		return;
	}

	while(lpQuery->next())
	{
		if(lpQuery->value("DB").toString() != szUser)
			continue;
		if(lpQuery->value("HOST").toString() != szHost)
			continue;
		if(lpQuery->value("DB").toString() != szDB)
			continue;
		if(lpQuery->value("COMMAND").toString() != szCommand)
			continue;
		if(lpQuery->value("INFO").toString().startsWith("SELECT ID, USER, HOST, DB, COMMAND, TIME, STATE, INFO FROM INFORMATION_SCHEMA.PROCESSLIST", Qt::CaseInsensitive))
			continue;

		QList<QStandardItem*>	items;
		QString					szInfo	= lpQuery->value("INFO").toString();

		if(szInfo.contains("\n"))
			szInfo	= szInfo.left(szInfo.indexOf("\n")) + " ...";

		qint32	iSecond	= lpQuery->value("TIME").toInt();
		qint32	iHour	= iSecond/3600;
		iSecond			-= iHour*3600;
		qint32	iMinute	= iSecond/60;
		iSecond			-= iMinute*60;

		QTime time(iHour, iMinute, iSecond);
		items.append(new QStandardItem(lpQuery->value("COMMAND").toString()));
		items.append(new QStandardItem(time.toString()));
		items.append(new QStandardItem(lpQuery->value("STATE").toString()));
		items.append(new QStandardItem(szInfo));

		items[1]->setTextAlignment(Qt::AlignRight);

		m_lpProcessModel->appendRow(items);
	}

	delete lpQuery;

	index	= m_lpProcessModel->index(row, 0);

	ui->m_lpProcessList->scrollTo(index, QAbstractItemView::PositionAtTop);

	if(selected != -1)
	{
		index	= m_lpProcessModel->index(selected, 0);
		ui->m_lpProcessList->setCurrentIndex(index);
	}

	for(int x = 0;x < PROCESS_ROWS;x++)
		ui->m_lpProcessList->setColumnWidth(x, col[x]);
}

void cMainWindow::updateImport()
{
	QModelIndex		index		= ui->m_lpImportList->indexAt(ui->m_lpImportList->rect().topLeft());
	qint32			row			= index.row();
	qint32			selected	= -1;
	qint32			col[IMPORT_ROWS];

	for(int x = 0;x < IMPORT_ROWS;x++)
		col[x]	= ui->m_lpImportList->columnWidth(x);

	index	= ui->m_lpImportList->currentIndex();

	if(index.isValid())
		selected	= index.row();

	m_lpImportModel->clear();

	QStringList	header;
	header << "nr" << "group" << "table" << "start" << "finish" << "duration";

	m_lpImportModel->setHorizontalHeaderLabels(header);

	if(!m_db.isOpen())
	{
		m_bMayUpdate	= true;
		return;
	}

	QSqlQuery*	lpQuery	= new QSqlQuery(m_db);

	lpQuery->prepare("SELECT	@rowid:=@rowid+1 AS number, "
					 "          t1.group_name, "
					 "          t1.table_name, "
					 "          CAST(IFNULL(DATE_FORMAT(t1.start, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) current_start, "
					 "          CAST(IFNULL(DATE_FORMAT(t1.finish, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) current_finish, "
					 "          CAST(IFNULL(t1.duration, '') AS CHAR) current_duration "
					 "FROM		v_imp_log t1, "
					 "          (SELECT @rowid:=0) AS init "
					 "WHERE		( "
					 "              t1.start >= "
					 "              ( "
					 "					SELECT	MAX(start) "
					 "					FROM	imp_log "
					 "					WHERE	group_name = 'import_task' "
					 "				) OR "
					 "				t1.start IS NULL "
					 "			) AND "
					 "			t1.group_name='import' "
					 "ORDER BY	@rowid:=@rowid+1; ");
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

		items[0]->setTextAlignment(Qt::AlignRight);

		m_lpImportModel->appendRow(items);
	}

	delete lpQuery;

	index	= m_lpImportModel->index(row, 0);

	ui->m_lpImportList->scrollTo(index, QAbstractItemView::PositionAtTop);

	if(selected != -1)
	{
		index	= m_lpImportModel->index(selected, 0);
		ui->m_lpImportList->setCurrentIndex(index);
	}

	for(int x = 0;x < IMPORT_ROWS;x++)
		ui->m_lpImportList->setColumnWidth(x, col[x]);
}

void cMainWindow::updatePrepare()
{
	QModelIndex		index		= ui->m_lpPrepareList->indexAt(ui->m_lpPrepareList->rect().topLeft());
	qint32			row			= index.row();
	qint32			selected	= -1;
	qint32			col[PREPARE_ROWS];

	for(int x = 0;x < PREPARE_ROWS;x++)
		col[x]	= ui->m_lpPrepareList->columnWidth(x);

	index	= ui->m_lpPrepareList->currentIndex();

	if(index.isValid())
		selected	= index.row();

	m_lpPrepareModel->clear();

	QStringList	header;
	header << "nr" << "group" << "table" << "start" << "finish" << "duration" << "estimated";

	m_lpPrepareModel->setHorizontalHeaderLabels(header);

	if(!m_db.isOpen())
	{
		m_bMayUpdate	= true;
		return;
	}

	QSqlQuery*	lpQuery	= new QSqlQuery(m_db);

	lpQuery->prepare("SELECT	@rowid:=@rowid+1 AS number, "
					 "			t1.group_name, "
					 "			t1.table_name, "
					 "			CAST(IFNULL(DATE_FORMAT(t1.current_start, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) current_start, "
					 "			CAST(IFNULL(DATE_FORMAT(t1.current_finish, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) current_finish, "
					 "			CAST(IFNULL(t1.current_duration, '') AS CHAR) current_duration, "
					 "			CAST(IFNULL(DATE_FORMAT(t1.estimated, '%Y-%m-%d %H:%i:%s'), '') AS CHAR) estimated "
					 "FROM		v_imp_log_detail_estimation t1, "
					 "			(SELECT @rowid:=0) AS init "
					 "WHERE		t1.current_start >= "
					 "			( "
					 "				SELECT	MAX(start) "
					 "				FROM	imp_log "
					 "				WHERE	table_name = 'errorstate' AND "
					 "						group_name = 'UKMS Start' "
					 "			) OR "
					 "			t1.current_start IS NULL "
					 "ORDER BY	@rowid:=@rowid+1; ");
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

		m_lpPrepareModel->appendRow(items);
	}

	delete lpQuery;

	index	= m_lpPrepareModel->index(row, 0);

	ui->m_lpPrepareList->scrollTo(index, QAbstractItemView::PositionAtTop);

	if(selected != -1)
	{
		index	= m_lpPrepareModel->index(selected, 0);
		ui->m_lpPrepareList->setCurrentIndex(index);
	}

	for(int x = 0;x < PREPARE_ROWS;x++)
		ui->m_lpPrepareList->setColumnWidth(x, col[x]);
}

void cMainWindow::updateGenerate()
{
	QModelIndex		index		= ui->m_lpGenerateList->indexAt(ui->m_lpGenerateList->rect().topLeft());
	qint32			row			= index.row();
	qint32			selected	= -1;
	qint32			col[GENERATE_ROWS];

	for(int x = 0;x < GENERATE_ROWS;x++)
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

	for(int x = 0;x < GENERATE_ROWS;x++)
		ui->m_lpGenerateList->setColumnWidth(x, col[x]);
}
