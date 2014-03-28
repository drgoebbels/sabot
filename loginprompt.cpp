#include "loginprompt.h"
#include "ui_loginprompt.h"

LoginPrompt::LoginPrompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPrompt)
{
    ui->setupUi(this);
}

QString LoginPrompt::getUsername()
{
    return ui->username->text();
}

QString LoginPrompt::getPassword()
{
    return ui->password->text();
}

QString LoginPrompt::getServer()
{
    return ui->serverList->currentText();
}

int LoginPrompt::getServerListIndex()
{
    return ui->serverList->currentIndex();
}

void LoginPrompt::reset()
{
    ui->password->clear();
}

LoginPrompt::~LoginPrompt()
{
    delete ui;
}

void LoginPrompt::on_login_clicked()
{
    this->accept();
}

void LoginPrompt::on_cancel_pressed()
{
    this->reject();
}
