#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <assert.h>

#include <QListWidget>
#include <QListWidgetItem>
#include <QTableWidget>
#include <QTableWidgetItem>

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
        send_message(conncurr, str);
    }
    ui->messageBox->clear();
}

void MainWindow::sendBroadcast()
{
    std::string temp1 = ui->messageBox->text().toStdString();
    const char *str = temp1.c_str();

    pm_broadcast(conncurr, str);

    ui->messageBox->clear();
}

void MainWindow::postRemoteMessage(message_s *msg)
{
    user_s *sender = msg->base.user;
    QWidget* tab = ui->serverTabs->currentWidget();
    QListWidget *chatList = tab->findChild<QListWidget *>("chatList");
    std::string message;

    if(msg->type == 'P') {
        message.append(" [private] ");
        putchar('\a');
    }
    if(msg->type == '9' || msg->type == 'P'){
        if(sender->mod_level == '0') {
            message.append("<");
            message.append(sender->name);
            message.append("> ");
        }
        else {
            message.append("<<");
            message.append(sender->name);
            message.append(">> ");
        }
    }
    else {
        message.append("<~");
        message.append(sender->name);
        message.append("~>");
    }
    message.append(msg->text);
    chatList->addItem(message.c_str());
    chatList->scrollToBottom();
}

void MainWindow::loginAccept()
{
    int index = lp->getServerListIndex();
    const char **server = sanet_servers[index];

    std::string username = lp->getUsername().toStdString();
    std::string password = lp->getPassword().toStdString();

    login(server[1], username.c_str(), password.c_str());

    if(ui->serverTabs->count() == 1 && ui->messageBox->isReadOnly()) {
        ui->serverTabs->setTabText(0, server[0]);
        ui->messageBox->setReadOnly(false);
    }
    else {
        if(active & (1 << index)) {

        }
        //ui->serverTabs->addTab()
    }
    active |= (1 << index);
}

void MainWindow::editUsers(edit_users_s *edit)
{
    user_s *u = edit->base.user;
    QWidget* tab = ui->serverTabs->currentWidget();
    QTableWidget *userList = tab->findChild<QTableWidget *>("userTable");
    std::string user(u->name);
    std::string id(u->id);

    if(u->mod_level > '0')
        id.append("     M");
    if(edit->add) {
        int c = userList->rowCount();
        userList->insertRow(c);
        userList->setItem(c, 0, new QTableWidgetItem(QString::fromStdString(user)));
        userList->setItem(c, 1, new QTableWidgetItem(QString::fromStdString(id)));
        userList->setColumnWidth(1, 50);
    }
    else {
        QList<QTableWidgetItem *> item = userList->findItems(QString::fromStdString(id), Qt::MatchExactly);
        userList->removeRow(item.first()->row());
    }
}

void MainWindow::editGamesSlot(edit_games_s *game)
{
    gamelist_s *g, *bg;
    QListWidget *gameList = ui->tab->findChild<QListWidget *>("gameList");

    gameList->clear();
    for(g = game->glist; g; g = bg) {
        gameList->addItem(g->name);
        bg = g->next;
        free(g);
    }
    free(game);
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
            /*if(conn->chat.tail && !conn->chat.tail->is_consumed) {
                conn->chat.tail->is_consumed = true;
            }
            if(conn->uqueue.head) {
                while((node = udequeue(conn))) {
                    emit updateUserList(node->uptr, node->add);
                    free(node);
                }
                conn->uqueue.head = NULL;
            }*/

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

