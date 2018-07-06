#include "login.h"
#include "ui_login.h"
#include<QtGui>
#include <QMessageBox>
#include "client.h"
#include <QMetaType>
#include "mainwindow.h"

extern Client* chat_client;

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    qRegisterMetaType<BYTE>("BYTE");
    qRegisterMetaType<string>("string");
    qRegisterMetaType<char>("char");
    ui->setupUi(this);
    setWindowTitle("ZJU聊天室");
    ui->password->setEchoMode(QLineEdit::Password);
    connect(ui->registBtn, SIGNAL(clicked(bool)), this, SLOT(on_registBtn_clicked()),Qt::UniqueConnection);
    connect(chat_client, SIGNAL(sendData_login1(BYTE)), this, SLOT(receiveData_Login1(BYTE)));
    connect(chat_client, SIGNAL(sendData_login2(string,string,char,string)), this, SLOT(receiveData_Login2(string,string,char,string)));
}

login::~login()
{
    delete ui;
}

void login::on_loginBtn_clicked()
{
    if(ui->id->text().isEmpty() || ui->password->text().isEmpty())
    {
         QMessageBox::warning(this,tr("warning"),tr("请输入账号及密码！"),QMessageBox::Yes);
         return;
    }
    BYTE message_body[42];
    string uid = ui->id->text().toStdString();
    string psw = ui->password->text().toStdString();
    memmove(message_body, uid.c_str(), 21);
    memmove(message_body+21, psw.c_str(), 21);
    chat_client->send(Login_Request, message_body, 42);
}

void login::on_cancleBtn_clicked()
{
    close();
}


void login::on_registBtn_clicked()
{
    r = new regist(this) ;//将类指针实例化
    r->show();
}

void login::receiveData_Login1(BYTE state)
{
    QMessageBox::warning(this,tr("warning"),tr("账号或密码错误！"),QMessageBox::Yes);
    ui->id->clear();
    ui->password->clear();
}

void login::receiveData_Login2(string username, string nickname, char gender, string birthday)
{
    QString _username = QString::fromStdString(username);
    QString _nickname = QString::fromStdString(nickname);
    QString _birthday = QString::fromStdString(birthday);
    QChar _gender = QChar(gender);
    MainWindow* w = new MainWindow(this, _username, _nickname, _gender, _birthday);
    ui->id->clear();
    ui->password->clear();
    this->hide();
    w->show();
}
