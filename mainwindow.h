#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pthread.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void postMessage();
    void postRemoteMessage();

signals:
    void returnPressed();
    void messageReceived();


private:
    pthread_t sanet_thread;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
