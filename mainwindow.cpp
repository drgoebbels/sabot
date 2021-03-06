#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <assert.h>
#include <string.h>

#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QWheelEvent>

/*
 * Not Where I wanted to place this, but so far this is
 * a way to avoid C & C++ compiler conflicts.
 *
 */
uint16_t active;
const char *sanet_servers[][2] = {
    {"2D Central", S_2D_CENTRAL},
    {"Paper Thin City", S_PAPER_THIN},
    {"Fine Line Island", S_FINE_LINE},
    {"U of SA", S_U_OF_SA},
    {"Flat World", S_FLAT_WORLD},
    {"Planar Outpost", S_PLANAR_OUTPOST},
    {"Mobius Metropolis", S_MOBIUS_METROPOLIS},
    {"EU Amsterdam", S_AMSTERDAM},
    {"Compatibility", S_COMPATABILITY},
    {"SS Lineage", S_SS_LINEAGE}
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    lp = NULL;
    ui->setupUi(this);
    monitor = new MonitorThread((Ui::MainWindow *)this);

    connect(ui->messageBox, SIGNAL(returnPressed()), this, SLOT(sendMessage()));
    connect(ui->pmBroadcast, SIGNAL(clicked()), this, SLOT(sendBroadcast()));
    connect(ui->addLogin, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
    connect(monitor, SIGNAL(messageReceived(message_s *)), this, SLOT(postRemoteMessage(message_s *)));
    connect(monitor, SIGNAL(updateUserList(edit_users_s *)), this, SLOT(editUsers(edit_users_s *)));
    connect(monitor, SIGNAL(editGames(edit_games_s*)), this, SLOT(editGamesSlot(edit_games_s*)));
    connect(ui->chatList, SIGNAL(wheelEvent(QWheelEvent *)), this, SLOT(scrollControl(QWheelEvent *)));

    connect(ui->serverList, SIGNAL(currentIndexChanged(int)), this, SLOT(beginServerChange(int)));

    // connect(ui->serverTabs->chi, SIGNAL(triggered()), this, SLOT(openDebugLogSlot()));
    monitor->start();
}

void MainWindow::sendMessage()
{
    std::string temp1 = ui->messageBox->text().toStdString();
    const char *str = temp1.c_str();

    if(ui->privateMessage->isChecked()) {
        QTableWidget *userList = ui->tab->findChild<QTableWidget *>("userTable");
        QTableWidgetItem *item = userList->item(userList->currentRow(), 1);
        send_pmessage(conncurr, str, item->text().toStdString().c_str());
    }
    else {
        send_message(conncurr, str, ui->messageType->text().toStdString().c_str());
    }
    ui->messageBox->clear();
}

void MainWindow::sendBroadcast()
{
    std::string temp1 = ui->messageBox->text().toStdString();
    const char *str = temp1.c_str();

  //  pm_broadcast(conncurr, str);

    ui->messageBox->clear();
}

void MainWindow::postRemoteMessage(message_s *msg)
{
    user_s *sender = msg->base.user;
    QWidget* tab = ui->serverTabs->currentWidget();
    QListWidget *list;
    std::string message;

    if(msg->type == 'P') {
        if(strcmp(msg->text, ">VIP<") && strcmp(msg->text, ">UNVIP<"))
            message = " [private] ";
        else
            message = " {vip} ";
        putchar('\a');
    }
    if(msg->type == '9' || msg->type == 'P'){
        list = tab->findChild<QListWidget *>("chatList");
        if(sender->mod_level == '0') {
            message += "<";
            message += sender->name;
            message += "> ";
        }
        else {
            message += "<<";
            message += sender->name;
            message += ">> ";
        }
    }
    else {
        list = tab->findChild<QListWidget *>("miscTraffic");
        message += "* <";
        message += sender->name;
        message += ":";
        message += msg->type;
        message += "> ";
    }
    message += msg->text;
    list->addItem(message.c_str());
    list->scrollToBottom();

   /* if(list->count() >= MAX_CHATMESSAGE_WINDOW) {
        QListWidgetItem *victim = list->takeItem(0);
        delete victim;
    }*/
}

void MainWindow::loginAccept()
{
    int i, index = lp->getServerListIndex();
    const char **server = sanet_servers[index];

    std::string username = lp->getUsername().toStdString();
    std::string password = lp->getPassword().toStdString();

    login(server[1], username.c_str(), password.c_str());


    if(ui->serverTabs->count() == 1 && ui->messageBox->isReadOnly()) {
        ui->serverTabs->setTabText(0, username.c_str());
        ui->messageBox->setReadOnly(false);
        ui->serverList->setDisabled(false);
    }
    else {
        if(active & (1 << index)) {

        }
        //ui->serverTabs->addTab()
    }
    suppressLoginSignal = true;

    ui->serverList->setCurrentIndex(index);

    active |= (1 << index);
}

void MainWindow::editUsers(edit_users_s *edit)
{
    user_s *u = edit->base.user;
    QWidget* tab = ui->serverTabs->currentWidget();
    QTableWidget *userList = tab->findChild<QTableWidget *>("userTable");
    QListWidget *list = tab->findChild<QListWidget *>("miscTraffic");

    std::string user(u->name);
    std::string id(u->id);
    std::string message;

    if(edit->add)
        message += "+ <";
    else
        message += "- <";

    message += u->name;
    message += "|";
    message += u->id;
    message += "> ";

    if(u->mod_level > '0') {
        id += "    --M.";
        id += u->mod_level;
        id += "--";
    }
    if(edit->add) {
        int c = userList->rowCount();
        message += "has entered the chatroom.";
        userList->insertRow(c);
        userList->setItem(c, 0, new QTableWidgetItem(QString::fromStdString(user)));
        userList->setItem(c, 1, new QTableWidgetItem(QString::fromStdString(id)));
        userList->setColumnWidth(1, 50);
    }
    else {
        QList<QTableWidgetItem *> item = userList->findItems(QString::fromStdString(id), Qt::MatchExactly);
        message += "has left the chatroom.";
        userList->removeRow(item.first()->row());
    }
    list->addItem(message.c_str());
    list->scrollToBottom();
    free(edit);
}

void MainWindow::editGamesSlot(edit_games_s *game)
{
    gamelist_s *g, *bg;
    QListWidget *gameList = ui->tab->findChild<QListWidget *>("gameList");

    gameList->clear();
    for(bg = g = game->glist; bg; g = bg) {
        gameList->addItem(g->name);
        bg = g->next;
        free(g);
    }
    free(game);
}

void MainWindow::openDebugLogSlot()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Files (*.*)"));
    //filename.
}

void MainWindow::loginButtonClicked()
{
    if(!lp) {
        lp = new LoginPrompt(this);
        connect(lp, SIGNAL(accepted()), this, SLOT(loginAccept()));
    }
    lp->show();
    lp->raise();
    lp->activateWindow();
}

void MainWindow::beginServerChange(int index)
{
    int result, i;
    const char *last;
    std::string message;
    QWidget* tab;
    QListWidget *chatList;

    if(!suppressLoginSignal) {
        ui->userTable->clear();
        ui->gameList->clear();

        last = conncurr->server;
        message = "####################-";
        tab = ui->serverTabs->currentWidget();
        chatList = tab->findChild<QListWidget *>("chatList");

        result = change_server(connlist, sanet_servers[index][1]);
        fflush(stdout);
        if(result < 0) {
            message += "Failed to Connect to-";
            message += sanet_servers[index][0];
            message += "-####################";
            chatList->addItem(message.c_str());
            chatList->scrollToBottom();
            result = change_server(connlist, last);
            for(i = 0; strcmp(sanet_servers[i][1], last); i++);

            if(result < 0) {
                message.clear();
                message += "####################-Failed to Return to ";
                message += sanet_servers[i][0];
            }
            else {
                message.clear();
                message += "####################-Returned to ";
                message += sanet_servers[i][0];
            }
        }
        else {
            message += "Successfully Connected to ";
            message += sanet_servers[index][0];
        }
        message += "-####################";
        chatList->addItem(message.c_str());
        chatList->scrollToBottom();
    }
    else {
        suppressLoginSignal = false;
    }
}

void userOptions(QListWidgetItem *item)
{
}

void MainWindow::scrollControl(QWheelEvent *e)
{

}

MonitorThread::MonitorThread(Ui::MainWindow *parent)
{
    this->parent = parent;
}

void MonitorThread::run()
{
    int i = 0;
    monitor.inuse = 1;
    chat_event_s *event;
    connect_inst_s *conn, *backup;

    forever {
        wait_message();

        //check for messages
        conn = connlist;
        while(conn) {
            msg_lock(conn);

            while((event = event_dequeue(conn))) {
                switch(event->type) {
                    case EVENT_CHAT_MSG:
                        emit messageReceived((message_s *)event);
                        break;
                    case EVENT_EDIT_USERS:
                        emit updateUserList((edit_users_s *)event);
                        break;
                    case EVENT_EDIT_GAMES:
                        emit editGames((edit_games_s *)event);
                        break;
                    default:
                        break;
                }
            }

            backup = conn;
            conn = conn->next;
            msg_unlock(backup);
        }
        release_message();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void signal_complete(void)
{

}
