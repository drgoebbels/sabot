#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "sanet.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->messageBox, SIGNAL(returnPressed()), this, SLOT(postMessage()));

    pthread_create(&sanet_listen, NULL, (void *(*)(void *))sanet_listen, NULL);

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

MainWindow::~MainWindow()
{
    delete ui;
}

void sanet_listen(void)
{
    while(1) {

    }
}
