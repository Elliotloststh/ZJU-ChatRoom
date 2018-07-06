#include "regist2.h"
#include "ui_regist2.h"
#include "client.h"
#include <QMessageBox>
#include <QMetaType>

extern Client* chat_client;
#define Register_succeed 1
#define Register_fail 0

regist2::regist2(QString user_name) :
    user(user_name),
    ui(new Ui::regist2)
{
    qRegisterMetaType<BYTE>("BYTE");
    this->setWindowModality(Qt::ApplicationModal);
    ui->setupUi(this);
    ui->nickname->setMaxLength(20);
    setWindowTitle("资料完善");
    ui->nickname->setPlaceholderText("不填默认为账号名");
    ui->birthday->setCalendarPopup(true);
    connect(chat_client, SIGNAL(sendData1(BYTE)),this, SLOT(receiveDate_Register1(BYTE)));
}

regist2::~regist2()
{
    delete ui;
}

void regist2::on_conBtn_clicked()
{
    int body_len = 21+21+1+10;
    BYTE message_body[body_len];
    string nickname;

    string uid = user.toStdString();
    if(ui->nickname->text().isEmpty())
        nickname = user.toStdString();
    else
        nickname = ui->nickname->text().toStdString();
    char gender;
    if(ui->gender->currentText() == "男")
        gender = 'M';
    else
        gender = 'F';
    string birthday = ui->birthday->date().toString("yyyy-MM-dd").toStdString();
//    cout<< uid <<endl;
//    cout<< nickname <<endl;
//    cout<< gender <<endl;
//    cout<< birthday <<endl;
    memmove(message_body, uid.c_str(), 21);
    memmove(message_body+21, nickname.c_str(), 21);
    memmove(message_body+42, &gender, 1);
    memmove(message_body+43, birthday.c_str(), 10);
    chat_client->send(Register_Extra_Request, message_body, body_len);


}

void regist2::receiveDate_Register1(BYTE response)
{
    switch(response)
    {
    case Register_fail:
        QMessageBox::warning(this,tr("warning"),tr("提交失败！"),QMessageBox::Yes);
        break;
    case Register_succeed:
        QMessageBox::information(this,tr(""),tr("提交成功！"),QMessageBox::Yes);
        this->deleteLater();
        break;
    }
}

