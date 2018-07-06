#include "regist.h"
#include "ui_regist.h"
#include <QMessageBox>
#include <QMetaType>
#include "client.h"

extern Client* chat_client;
#define Register_succeed 1
#define Register_fail 0

regist::regist(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::regist)
{
    qRegisterMetaType<BYTE>("BYTE");
    this->setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    setWindowTitle("账号注册");
    ui->password->setPlaceholderText("6~20位组合");
    ui->password->setEchoMode(QLineEdit::Password);
    ui->pswconfirm->setEchoMode(QLineEdit::Password);
    ui->password->setMaxLength(20);
    ui->pswconfirm->setMaxLength(20);
    connect(ui->nextBtn, SIGNAL(clicked(bool)), this, SLOT(on_nextBtn_clicked()),Qt::UniqueConnection);
    connect(chat_client, SIGNAL(sendData(BYTE)),this, SLOT(receiveData_Register(BYTE)));
}


regist::~regist()
{
delete ui;
}

void regist::on_nextBtn_clicked()
{
    if(ui->id->text().isEmpty() || ui->pswconfirm->text().isEmpty()|| ui->password->text().isEmpty())
    {
        QMessageBox::warning(this,tr("warning"),tr("请输入完整信息！"),QMessageBox::Yes);
        return;
    }

    if(ui->pswconfirm->text() != ui->password->text())
    {
        QMessageBox::warning(this,tr("warning"),tr("两次密码输入不一致！"),QMessageBox::Yes);
        ui->pswconfirm->clear();   //清空账号输入框
        ui->pswconfirm->setFocus();  //将光标转到账号输入框
        return;
    }

    else
    {
        BYTE message_body[42];
        string uid = ui->id->text().toStdString();
        string psw = ui->password->text().toStdString();
        memmove(message_body, uid.c_str(), 21);
        memmove(message_body+21, psw.c_str(), 21);
        user = ui->id->text();
        chat_client->send(Register_Request, message_body, 42);
        string nickname = uid;
        char gender = 'M';
        string birthday = "2000-01-01";
        BYTE msg[53];
        memmove(msg, uid.c_str(), 21);
        memmove(msg+21, nickname.c_str(), 21);
        memmove(msg+42, &gender, 1);
        memmove(msg+43, birthday.c_str(), 10);
        chat_client->send(Register_Extra_Request, msg, 53);
    }
//    if(ui->pswconfirm->text() == ui->password->text() ){
//        r2 = new regist2(this);
//        this->close();
//        r2->show();
//    }
}

void regist::receiveData_Register(BYTE response)
{
    switch(response)
    {
    case Register_fail:
        QMessageBox::warning(this,tr("warning"),tr("用户名已存在！"),QMessageBox::Yes);
        ui->id->clear();
        break;
    case Register_succeed:
        QMessageBox::information(this,tr(""),tr("注册成功！"),QMessageBox::Yes);
        r2 = new regist2(user);
        r2->show();
        this->deleteLater();
        break;
    }
}
