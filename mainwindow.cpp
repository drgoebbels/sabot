#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "sanet.h"
#include <pthread.h>

static void *monitor_thread(void);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


}

MainWindow::~MainWindow()
{
    delete ui;
}

void *monitor_thread(void)
{

}
