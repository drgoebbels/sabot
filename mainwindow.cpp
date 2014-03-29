#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sanet.h"

/*
 * Not Where I wanted to place this, but so far this is
 * a way to avoid C & C++ compiler conflicts.
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

void MainWindow::postRemoteMessage()
{

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
    forever {
        wait_message();

        release_message();
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

