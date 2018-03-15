#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#include <QSystemTrayIcon>
#include <QMainWindow>

#include <QSqlDatabase>
#include <QStandardItemModel>

#include <QTimer>


namespace Ui {
class cMainWindow;
}

class cMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit cMainWindow(QWidget *parent = 0);
	~cMainWindow();

	void				setVisible(bool visible) override;

protected:
	void				closeEvent(QCloseEvent* event) override;

private slots:
	void				iconActivated(QSystemTrayIcon::ActivationReason reason);
	void				timerUpdate();

	void				updateProcessList();
	void				updateImport();
	void				updatePrepare();
	void				updateGenerate();

private:
	Ui::cMainWindow*	ui;
	QSystemTrayIcon*	m_lpTrayIcon;
	QMenu*				m_lpTrayIconMenu;
	QSqlDatabase		m_db;
	QTimer*				m_lpTimer;
	bool				m_bMayUpdate;

	QStandardItemModel*	m_lpProcessModel;
	QStandardItemModel*	m_lpImportModel;
	QStandardItemModel*	m_lpPrepareModel;
	QStandardItemModel*	m_lpGenerateModel;

	QAction*			m_lpMinimizeAction;
	QAction*			m_lpMaximizeAction;
	QAction*			m_lpRestoreAction;
	QAction*			m_lpQuitAction;

	void				createActions();
	void				createTrayIcon();

	void				connectDB();
	void				setTimer();
};

#endif // CMAINWINDOW_H
