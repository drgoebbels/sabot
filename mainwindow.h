#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "loginprompt.h"
#include "sanet.h"

namespace Ui {
    class MonitorThread;
    class PthreadParent;
    class MapCompare;
    class MainWindow;
}

class MonitorThread : public QThread
{
    Q_OBJECT
signals:
    void messageReceived(connect_inst_s *conn);
    void updateUserList(user_s *u, bool add);

public:
    MonitorThread(Ui::MainWindow *parent);
private:
    void run();

    Ui::MainWindow *parent;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    LoginPrompt *lp;

public slots:
    void postMessage();
    void postRemoteMessage(connect_inst_s *conn);
    void loginButtonClicked();
    void loginAccept();
    void addUser(user_s *u, bool add);

signals:
    void returnPressed();

private:
    Ui::MainWindow *ui;
    MonitorThread *monitor;
};


#endif // MAINWINDOW_H
