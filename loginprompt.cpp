#include "loginprompt.h"
#include "ui_loginprompt.h"

LoginPrompt::LoginPrompt(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPrompt)
{
    ui->setupUi(this);
}

const char *LoginPrompt::getUsername()
{
    return ui->username->text().toStdString().c_str();
}

const char *LoginPrompt::getPassword()
{
    return ui->password->text().toStdString().c_str();
}

const char *LoginPrompt::getServer()
{
    printf("%d\n", ui->serverList->currentIndex());
    return ui->serverList->currentText().toStdString().c_str();
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
