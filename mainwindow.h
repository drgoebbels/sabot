#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

namespace Ui {
    class MonitorThread;
    class MainWindow;
}

class MonitorThread : public QThread
{
    Q_OBJECT
private:
    void run();
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

signals:
    void returnPressed();
    void messageReceived();


private:
    Ui::MainWindow *ui;
    MonitorThread monitor;
};


#endif // MAINWINDOW_H
