#include "database.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    db_init("sabot.db");

    w.show();
    return a.exec();
}
