#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaType>

extern boost::asio::io_context io;
extern Client* chat_client;


MainWindow::MainWindow(QString user_name) :
    ui(new Ui::MainWindow), user(user_name)
{
    qRegisterMetaType<string>("string");
    ui->setupUi(this);
    ui->textEdit->setAcceptRichText(false);
    ui->label_2->setText(user);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, "#f9f2f5");
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    setWindowTitle("ZJU聊天室");
    connect(chat_client, &Client::sendData, this, &MainWindow::receiveData_Public);
    this->setFixedSize( this->width (),this->height ());
    ui->listWidget_2->addItem("Jack");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString instr = ui->textEdit->document()->toPlainText();
//    boost::shared_ptr< boost::asio::io_service::work > work(
//            new boost::asio::io_service::work( io_w )
//            );
    if(!instr.isEmpty())
    {

        int body_len = instr.length();
        string str = instr.toStdString();
        cout<<str<<" "<<body_len<<endl;
        const char *ch = str.data();
        const char *uid = user.toStdString().data();
        BYTE* send_msg = new BYTE[body_len+20];
        memcpy(send_msg, uid, 20);
        memcpy(send_msg+20,ch, body_len);
        cout<<send_msg<<endl;
        chat_client->send(PublicMessage_Request, send_msg, body_len+20);



    }
//    io_context::work work(io_w);
//    work.reset();
//    io_w.stop();
    ui->textEdit->clear();
}

void MainWindow::receiveData_Public(string username, string msg, string time)
{
    string s = username + "    " + time;
    QString ss = QString::fromStdString(s);
    string m = "  "+msg;
    QString mm = QString::fromStdString(m);
    ui->public_chat->addItem(ss);
    ui->public_chat->addItem(mm);     //获取传递过来的数据
}
