#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sanet.h"
#include <pthread.h>
#include <stdlib.h>

static void *monitor_thread(void);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    puts("hello world d");
    fflush(stdout);
    connect(ui->messageBox, SIGNAL(textChanged()), this, SLOT(test()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::test()
{
    puts("derp");
    fflush(stdout);
}


void *monitor_thread(void)
{

}
