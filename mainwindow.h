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
    void messageReceived(message_s *conn);
    void updateUserList(edit_users_s *);
    void editGames(edit_games_s *);

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
    void sendMessage();
    void sendBroadcast();
    void postRemoteMessage(message_s *conn);
    void loginButtonClicked();
    void loginAccept();
    void editUsers(edit_users_s *);
    void editGamesSlot(edit_games_s *);
    void openDebugLogSlot();
    void scrollControl(QWheelEvent *);

signals:


private:
    Ui::MainWindow *ui;
    MonitorThread *monitor;
};


#endif // MAINWINDOW_H
