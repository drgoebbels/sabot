#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sanet.h"

#include "loginprompt.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->messageBox, SIGNAL(returnPressed()), this, SLOT(postMessage()));
    connect(ui->addLogin, SIGNAL(clicked()), this, SLOT(loginButtonClicked()));
    monitor.start();
}


void MainWindow::postMessage()
{
    QListWidgetItem *item = new QListWidgetItem(ui->messageBox->text());
    ui->chatText->addItem(item);
    ui->chatText->scrollToItem(item);
    ui->messageBox->clear();
}

void MainWindow::postRemoteMessage()
{

}

void MainWindow::loginButtonClicked()
{
    LoginPrompt lp(this);
    lp.exec();
}

void MonitorThread::run()
{
    while(true) {
        wait_message();

        release_message();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

