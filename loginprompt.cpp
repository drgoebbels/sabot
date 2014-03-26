#include "loginprompt.h"
#include "ui_loginprompt.h"

LoginPrompt::LoginPrompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPrompt)
{
    ui->setupUi(this);
    //ui->serverList->itemData(ui->serverList->currentIndex()).da
}

LoginPrompt::~LoginPrompt()
{
    delete ui;
}
