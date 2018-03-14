#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H


#include <QSystemTrayIcon>
#include <QDialog>


namespace Ui {
class cMainWindow;
}

class cMainWindow : public QDialog
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
	void				messageClicked();

private:
	Ui::cMainWindow*	ui;
	QSystemTrayIcon*	m_lpTrayIcon;
	QMenu*				m_lpTrayIconMenu;

	QAction*			m_lpMinimizeAction;
	QAction*			m_lpMaximizeAction;
	QAction*			m_lpRestoreAction;
	QAction*			m_lpQuitAction;

	void				createActions();
	void				createTrayIcon();
};

#endif // CMAINWINDOW_H
