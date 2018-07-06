#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include"regist.h"
#include "Message.hpp"
using namespace std;

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();

private slots:
    void on_loginBtn_clicked();

    void on_cancleBtn_clicked();

    void on_registBtn_clicked();

    void receiveData_Login1(BYTE state);

    void receiveData_Login2(string username, string nickname, char gender, string birthday);

private:
    Ui::login *ui;
    regist *r;
};

#endif // LOGIN_H
