#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include "loginprompt.h"


namespace Ui {
    class MonitorThread;
    class MapCompare;
    class MainWindow;
}

class MonitorThread : public QThread
{
    Q_OBJECT
private:
    void run();
};

class MapCompare {
public:
    bool operator()(std::string& a, std::string& b) {return !!a.compare(b);}
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

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
    LoginPrompt *lp;
    MonitorThread monitor;
};


#endif // MAINWINDOW_H
