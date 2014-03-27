#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "loginprompt.h"


namespace Ui {
    class MonitorThread;
    class PthreadParent;
    class MapCompare;
    class MainWindow;
}

class MonitorThread : public QThread
{
    Q_OBJECT
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
    void postRemoteMessage();
    void loginButtonClicked();
    void loginAccept();


signals:
    void returnPressed();
    void messageReceived();

private:
    Ui::MainWindow *ui;
    MonitorThread *monitor;
};


#endif // MAINWINDOW_H
