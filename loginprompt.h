#ifndef LOGINPROMPT_H
#define LOGINPROMPT_H

#include <QDialog>

namespace Ui {
class LoginPrompt;
}

class LoginPrompt : public QDialog
{
    Q_OBJECT

public:
    explicit LoginPrompt(QWidget *parent = 0);
    ~LoginPrompt();

    QString getUsername();
    QString getPassword();
    QString getServer();
    int getServerListIndex();
    void reset();

private slots:
    void on_login_clicked();

    void on_cancel_pressed();

private:
    Ui::LoginPrompt *ui;
};

#endif // LOGINPROMPT_H
