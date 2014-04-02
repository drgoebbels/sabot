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

    connect(ui->messageBox, SIGNAL(returnPressed()), this, SLOT(postMessage()));
    connect(ui->addLogin, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
    connect(monitor, SIGNAL(messageReceived(connect_inst_s *)), this, SLOT(postRemoteMessage(connect_inst_s *)));
    connect(monitor, SIGNAL(updateUserList(user_s *, bool)), this, SLOT(addUser(user_s *, bool)));

    monitor->start();
}

void MainWindow::postMessage()
{
    /*QListWidgetItem *item = new QListWidgetItem(ui->messageBox->text());
    ui->userTabs->currentIndex();
    ui->userTabs->tab
    ui->chatText->scrollToItem(item);
    ui->messageBox->clear();*/
}

void MainWindow::postRemoteMessage(connect_inst_s *conn)
{
    chat_packet_s *msg = conn->chat.tail;
    user_s *sender = msg->user;
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

void MainWindow::addUser(user_s *u, bool add)
{
    QWidget* tab = ui->serverTabs->currentWidget();
    QListWidget *userList = tab->findChild<QListWidget *>("userList");
    std::string user(" ");
    user.append(u->id);
    user.append(" -> ");
    user.append(u->name);
    if(u->mod_level > '0')
        user.append(": M");


    if(add) {
        userList->addItem(user.c_str());
    }
    else {
        printf("%s disconnected\n", u->name);

        QList<QListWidgetItem *> childList = userList->findItems(user.c_str(), Qt::MatchExactly);
        if(childList.size() > 0)
            delete childList.first();
    }

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
    user_event_queue_s *node;
    connect_inst_s *conn, *backup;

    forever {
        wait_message();

        //check for messages
        conn = connlist;
        while(conn) {
            msg_lock(conn);
            if(conn->chat.tail && !conn->chat.tail->is_consumed) {
                emit messageReceived(conn);
                conn->chat.tail->is_consumed = true;
            }
            if(conn->uqueue.head) {
                while((node = udequeue(conn))) {
                    emit updateUserList(node->uptr, node->add);
                    free(node);
                }
                conn->uqueue.head = NULL;
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

