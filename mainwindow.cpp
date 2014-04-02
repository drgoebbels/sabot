#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidget>
#include <QListWidgetItem>

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
    connect(ui->addLogin, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
    connect(monitor, SIGNAL(messageReceived(message_s *)), this, SLOT(postRemoteMessage(message_s *)));
    connect(monitor, SIGNAL(updateUserList(edit_users_s *)), this, SLOT(editUsers(edit_users_s *)));

    monitor->start();
}

void MainWindow::sendMessage()
{
    const char *str = ui->messageBox->text().toStdString().c_str();

    ui->messageBox->clear();
    send_message(conncurr, str);
}

void MainWindow::postRemoteMessage(message_s *msg)
{
    user_s *sender = msg->base.user;
    QWidget* tab = ui->serverTabs->currentWidget();
    QListWidget *chatList = tab->findChild<QListWidget *>("chatList");
    std::string message;

    if(msg->type == 'P') {
        message.append(" [private] ");
    }
    message.append("<");
    message.append(sender->name);
    message.append(":");
    message.append(sender->id);
    message.append("> ");
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
    QListWidget *userList = tab->findChild<QListWidget *>("userList");
    QListWidget *chatList = tab->findChild<QListWidget *>("chatList");
    std::string user(" ");
    std::string message(u->name);
    user.append(u->id);
    user.append(" -> ");
    user.append(u->name);

    if(u->mod_level > '0')
        user.append(": M");
    if(edit->add) {
        message.append(" joined lobby.");
        userList->addItem(user.c_str());
        chatList->addItem(message.c_str());
    }
    else {
        message.append(" left lobby.");
        QList<QListWidgetItem *> childList = userList->findItems(user.c_str(), Qt::MatchExactly);
        if(childList.size() > 0)
            delete childList.first();
        chatList->addItem(message.c_str());
    }
    chatList->scrollToBottom();
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

