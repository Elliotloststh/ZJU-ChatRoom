#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <iostream>
#include <QMetaType>
#include <Qdebug>
#include <QFileDialog>
#include <QMessageBox>

extern boost::asio::io_context io;
extern Client* chat_client;

#define ONLINE 'A'
#define STEALTH 'B'
#define OFFLINE 'C'

MainWindow::MainWindow(QWidget *parent,QString user_name,QString _nickname, QChar _gender, QString _birthday) :
    QMainWindow(parent),ui(new Ui::MainWindow), user(user_name),
    nickname(_nickname), gender(_gender), birthday(_birthday)
{
    qRegisterMetaType<string>("string");
    qRegisterMetaType<QVariant>("QVariant");
    ui->setupUi(this);
    ui->textEdit->setAcceptRichText(false);
    ui->User_name->setText(nickname);
    ui->userid->setText(user);
    ui->nickname->setText(nickname);
    QString tmp;
    if(gender == 'M')
        tmp = "男";
    else
        tmp = "女";
    ui->gender->setText(tmp);
    ui->birthday->setText(birthday);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, "#f9f2f5");
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    setWindowTitle("ZJU聊天室");
    connect(chat_client, SIGNAL(sendData(string,string,string)), this, SLOT(receiveData_Public(string,string,string)));
    connect(chat_client, SIGNAL(sendData_private(string,string,string,string)), this, SLOT(receiveData_Private(string,string,string,string)));
    connect(chat_client, SIGNAL(sendData_addfriend(QVariant)), this, SLOT(receiveData_addfriend(QVariant)));
    connect(chat_client, SIGNAL(sendData_checkonline(BYTE)), this, SLOT(receiveData_checkonline(BYTE)));
    connect(chat_client, SIGNAL(sendData_addrequest(BYTE)), this, SLOT(receiveData_addrequest(BYTE)));
    connect(chat_client, SIGNAL(sendData_addapplication(string)), this, SLOT(receiveData_addapplication(string)));
    connect(chat_client, SIGNAL(sendData_newfriend(BYTE)), this, SLOT(receiveData_newfriend(BYTE)));
    connect(chat_client, SIGNAL(sendData_deletefriend(string)), this, SLOT(receiveData_deletefriend(string)));
    connect(chat_client, SIGNAL(sendData_update(BYTE,string)), this, SLOT(receiveData_update(BYTE,string)));
    connect(chat_client, SIGNAL(sendData_update2(BYTE)), this, SLOT(receiveData_update2(BYTE)));
    this->setFixedSize( this->width (),this->height ());
    this->setFixedSize( this->width (),this->height ());
    ui->tabWidget->setCurrentIndex(0);
//    ui->public_chat->scrollToBottom();
    // 请求好友列表

    int body_length = 21;
    BYTE message_body[body_length];
    memmove(message_body,user.toStdString().c_str(), 21); // 写_user_id
    chat_client->send(FriendList_Request, message_body, body_length);
    chat_client->send(CheckApplication_Request, message_body, body_length);
    // 初始化好友列表
    ui->listWidget->addItem(user);
    ui->request->hide();
    ui->accept_btn->hide();
    ui->reject_btn->hide();
    ui->deleteBtn->hide();
    ui->chatBtn->hide();
    connect(ui->listWidget_private, SIGNAL(itemSelectionChanged()), this, SLOT(switchfriend()));
    connect(ui->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(singleclicked(QListWidgetItem*)));
    connect(ui->listWidget_2,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(singleclicked_2(QListWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()//公聊
{
    QString instr = ui->textEdit->document()->toPlainText();
    if(!instr.isEmpty())
    {

        int body_len = instr.length();
        string str = instr.toStdString();
        const char *ch = str.data();
        const char *uid = user.toStdString().data();
        BYTE* send_msg = new BYTE[body_len+20];
        memcpy(send_msg, uid, 20);
        memcpy(send_msg+20,ch, body_len);
        chat_client->send(PublicMessage_Request, send_msg, body_len+20);
    }
    ui->textEdit->clear();
}


// 私聊发送
void MainWindow::on_pushButton_private_clicked()
{
    QString instr = ui->textEdit_private->document()->toPlainText();
    if(!instr.isEmpty())
    {
        int body_length = 21 + 21 + instr.length() + 1;
        BYTE message_body[body_length];
        string _user_id = user.toStdString();
        int index = ui->listWidget_private->currentRow();
        string _user_id2 = chatlist[index];
        string message = instr.toStdString();
        memmove(message_body, _user_id.c_str(), 21); // 写_user_id
        memmove(message_body + 21, _user_id2.c_str(), 21); // 写_user_id2
        memmove(message_body + 21 + 21, message.c_str() ,message.length()+1); // 写messqge
        //chat_client->send(content_type, message_body, body_length);
        chat_client->send(PrivateMessage_Request, message_body, body_length);
    }
    ui->textEdit_private->clear();
}

// 公聊聊天记录查询
void MainWindow::on_record_clicked()
{
    record = new chat_record(this);
    record->show();
}

// 私聊聊天记录查询
void MainWindow::on_record_private_clicked()
{
    record = new chat_record(this);
    record->show();
    //chat_client->send(content_type, message_body, body_length);
}

/*
void MainWindow::on_listWidget_private_itemSelectionChanged()
{
    QString user_id2 = ui->listWidget_private->currentItem()->text();
    ui->textBrowser_private->append(user_id2);
    QTextBrowser *t = new QTextBrowser;
    t->setObjectName("textbrowser" + user_id2);
    //t->show();
    //ui->verticalLayout->addWidget(t);
}*/

// 更改状态
void MainWindow::on_checkBox_stateChanged(int state)
{
    BYTE new_state;
    if (state == Qt::Checked) // "选中"
        new_state = STEALTH;
    else
        new_state = ONLINE;
    int body_length = 21 + 1;
    BYTE message_body[body_length];
    memcpy(message_body, user.toStdString().c_str(), 21); // 写_user_id
    memcpy(message_body + 21, &new_state, 1); // 写_state
    chat_client->send(ChangeState_Request, message_body, body_length);
}

// 私聊聊天切换
void MainWindow::switchfriend()
{
    int current_index = ui->listWidget_private->currentRow();
    ui->stackedWidget_private->setCurrentIndex(current_index+1);
}

// 学姐

void MainWindow::singleclicked(QListWidgetItem* item)   //实现item的单击操作
{
    int index = ui->listWidget->currentRow();
    if(index==0)
    {
        ui->userid->setText(user);
        ui->nickname->setText(nickname);
        QString tmp;
        if(gender == 'M')
            tmp = "男";
        else
            tmp = "女";
        ui->gender->setText(tmp);
        ui->birthday->setText(birthday);
        ui->deleteBtn->hide();
        ui->chatBtn->hide();
        ui->addf->show();
        ui->addfriend->show();
        ui->confBtn->show();
    }
    else
    {
        ui->userid->setText(QString::fromStdString(friendlist[index-1].uid));
        ui->nickname->setText(QString::fromStdString(friendlist[index-1].nickname));
        ui->gender->setText(QString::fromStdString(friendlist[index-1].gender));
        ui->birthday->setText(QString::fromStdString(friendlist[index-1].birthday));
        ui->deleteBtn->show();
        ui->chatBtn->show();
        ui->addf->hide();
        ui->addfriend->hide();
        ui->confBtn->hide();
    }

}

void MainWindow::singleclicked_2(QListWidgetItem *item)
{
    if(ui->request->isHidden())
    {
        ui->request->show();
        ui->accept_btn->show();
        ui->reject_btn->show();
    }
    string sentence ="用户 " + item->text().toStdString()+" 请求添加你为好友";
    ui->request->setText(QString::fromStdString(sentence));
}


void MainWindow::on_deleteBtn_clicked()     //删除按钮
{
    bool en = false;
    switch( QMessageBox::warning(this, "提示",QString::fromLocal8Bit("确定删除该好友吗?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No | QMessageBox::Escape))
    {
        case QMessageBox::Yes:
        {
            en = true;
            break;
        }
        case QMessageBox::No:
        {
            break;
        }
    }
    if(en)
    {
        int body_length = 21+21;
        string user_1 = user.toStdString();
        int index = ui->listWidget->currentRow()-1;
        string user_2 = friendlist[index].uid;
        BYTE message_body[body_length];
        memmove(message_body, user_1.c_str(), 21); // 写_user_id
        memmove(message_body + 21, user_2.c_str(), 21); // 写_user_id2
        chat_client->send(DeleteFriend_Request, message_body, body_length);

    }
}

void MainWindow::on_chatBtn_clicked()      //私聊按钮
{ 
    int no = ui->listWidget->currentRow()-1;
    int body_length = 21;
    BYTE message_body[body_length];
    memcpy(message_body, friendlist[no].uid.c_str(), 21);
    chat_client->send(CheckOnline_Request, message_body, 21);
}

void MainWindow::on_confBtn_clicked()
{
    int body_length = 21+21;
    string user_1 = user.toStdString();
    string user_2 = ui->addfriend->text().toStdString();
    if(user_2==user_1)
    {
        QMessageBox::information(this,tr(""),tr("不能添加自己为好友"),QMessageBox::Yes);
        return;
    }
    for(int i = 0;i<friendlist.size();i++)
    {
        if(friendlist[i].uid == user_2)
        {
            QMessageBox::information(this,tr(""),tr("你们已经是好友了"),QMessageBox::Yes);
            return;
        }
    }
    BYTE message_body[body_length];
    memmove(message_body, user_1.c_str(), 21); // 写_user_id
    memmove(message_body + 21, user_2.c_str(), 21); // 写_user_id2
    chat_client->send(AddFriend_Request, message_body, body_length);

}

void MainWindow::on_accept_btn_clicked()
{
    int body_length = 21 + 21 + 1;
    string user_1 = user.toStdString();
    string user_2 = ui->listWidget_2->currentItem()->text().toStdString();
    BYTE message_body[body_length];
    memmove(message_body, user_1.c_str(), 21); // 写_user_id
    memmove(message_body + 21, user_2.c_str(), 21); // 写_user_id2
    message_body[body_length-1] = 1;
    chat_client->send(AgreeApplication_Request, message_body, body_length);
}

void MainWindow::on_reject_btn_clicked()
{
    int body_length = 21 + 21 + 1;
    string user_1 = user.toStdString();
    string user_2 = ui->listWidget_2->currentItem()->text().toStdString();
    BYTE message_body[body_length];
    memmove(message_body, user_1.c_str(), 21); // 写_user_id
    memmove(message_body + 21, user_2.c_str(), 21); // 写_user_id2
    message_body[body_length-1] = 0;
    int index = ui->listWidget_2->currentRow();
    applicationlist.erase(applicationlist.begin()+index);
    ui->listWidget_2->takeItem(index);
    ui->listWidget_2->setCurrentRow(-1);
    ui->request->hide();
    ui->accept_btn->hide();
    ui->reject_btn->hide();
    chat_client->send(AgreeApplication_Request, message_body, body_length);
}

void MainWindow::on_open_file_clicked()
{
    QString path = QFileDialog::getOpenFileName(this,"标题",".","Images (*.png *.xpm *.jpg *.txt)");
    if(path.isEmpty())
        return;
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite)){
        return;
    }
    else{
        // 私聊发图/文件 用
        QByteArray tmp = file.QIODevice::readAll(); // 读取所有字节
        qDebug()<< tmp;
        QString user_id2 = ui->listWidget_private->currentItem()->text(); // 得到当前好友的id
        //ui->textBrowser_private->append(user_id2);
        //ui->textBrowser_private->append(user);
        int content_type = 22;
        int body_length = 20 + 20 + file.size();
        string _user_id = user.toStdString();
        string _user_id2 = user_id2.toStdString();
        BYTE message_body[12 + body_length];
        memset(message_body, 0, 12 + body_length);
        memmove(message_body, _user_id.c_str(), _user_id.length()); // 写_user_id
        memmove(message_body + 20, _user_id2.c_str(), _user_id2.length()); // 写_user_id2
        memmove(message_body + 20 + 20, tmp.data(), file.size());
        cout << file.size() << endl;
        //chat_client->send(content_type, message_body, body_length);*/
    }
    file.close();
}

//----------for slot-----------//
void MainWindow::receiveData_Public(string username, string msg, string time)
{
    string s = username + "    " + time;
    QString ss = QString::fromStdString(s);
//    string m = " • "+msg;
    string m = msg+'\n';
    QString mm = QString::fromStdString(m);
    QListWidgetItem *newItem1 = new QListWidgetItem;
    newItem1->setText(ss);
    newItem1->setTextColor(QColor(82,139, 139, 255));
    QListWidgetItem *newItem2 = new QListWidgetItem;
    newItem2->setText(mm);

    ui->public_chat->addItem(newItem1);
    ui->public_chat->addItem(newItem2);     //获取传递过来的数据
}

void MainWindow::receiveData_addfriend(QVariant variant)
{
    Friend f = variant.value<Friend>();
    friendlist.push_back(f);
    ui->listWidget->addItem(QString::fromStdString(f.nickname));
}

void MainWindow::receiveData_addapplication(string userid)
{
    applicationlist.push_back(userid);
    ui->listWidget_2->addItem(QString::fromStdString(userid));
}

void MainWindow::receiveData_checkonline(BYTE response)
{
    switch(response)
    {
    case(OFFLINE):
    {
        QMessageBox::warning(this,tr("warning"),tr("对方不在线！"),QMessageBox::Yes);
        break;
    }
    case(ONLINE):
    {
        QString str = ui->listWidget->currentItem()->text();
        QListWidgetItem*item = new QListWidgetItem;     //建立一新的项目
        item->setText(str);
        QListWidget *pListWidget = new QListWidget;
        ui->stackedWidget_private->addWidget(pListWidget);
        int flag = 0;
        int i;
        int index = ui->listWidget->currentRow()-1;
        string one = friendlist[index].uid;
        for(i = 0; i<chatlist.size(); i++)//遍历所有的ITEM
        {
            if(chatlist[i]==one){
                flag = 1;
                break;
            }
        }

        if(flag==0){
            ui->listWidget_private->addItem(item);
            chatlist.push_back(one);
            ui->tabWidget->setCurrentIndex(1);   //跳转到私聊界面
            ui->listWidget_private->setCurrentItem(item);
            int c = ui->stackedWidget_private->count();
            ui->stackedWidget_private->setCurrentIndex(c-1);
        }
        else{
            ui->tabWidget->setCurrentIndex(1);   //跳转到私聊界面
            ui->listWidget_private->setCurrentRow(i);
            ui->stackedWidget_private->setCurrentIndex(i+1);
        }

    }

    }
}

void MainWindow::receiveData_Private(string username1, string username2, string msg, string time)
{
    int index;
    if(username1!=user.toStdString())
    {

        int flag = 0;
        int i;
        for(i = 0; i<chatlist.size(); i++)//遍历所有的ITEM
        {
            if(chatlist[i]==username1){
                index = i;
                flag = 1;
                break;
            }
        }
        if(flag==0)
        {
            QListWidget *pListWidget = new QListWidget;
            ui->stackedWidget_private->addWidget(pListWidget);
            QListWidgetItem*item = new QListWidgetItem;
            int index1;
            for(int k = 0;k<friendlist.size();k++)
            {
                if(friendlist[k].uid==username1)
                {
                    index1 = k;
                    break;
                }

            }
            chatlist.push_back(username1);
            item->setText(QString::fromStdString(username1));
            ui->listWidget_private->addItem(item);
            ui->listWidget_private->setCurrentItem(item);
            index = ui->stackedWidget_private->count()-2;
        }
        auto pchat = qobject_cast<QListWidget*>(ui->stackedWidget_private->widget(index+1));
        string s = username1 + "    " + time;
        QListWidgetItem *newItem1 = new QListWidgetItem;
        newItem1->setText(QString::fromStdString(s));
        newItem1->setTextColor(QColor(82,139, 139, 255));
        QListWidgetItem *newItem2 = new QListWidgetItem;
        newItem2->setText(QString::fromStdString(msg+"\n"));
        pchat->addItem(newItem1);
        pchat->addItem(newItem2);
//        ui->stackedWidget_private->setCurrentWidget(pchat);
    }
    else
    {
        for(int i = 0; i<chatlist.size(); i++)//遍历所有的ITEM
        {
            if(chatlist[i]==username2){
                index = i;
                break;
            }
        }
//        ui->stackedWidget_private->children()
//        QList<QListWidget*> plist = ui->stackedWidget_private->findChildren<QListWidget*>();
//        QListWidget* pchat = plist.at(plist.size()-1-index);
        auto pchat = qobject_cast<QListWidget*>(ui->stackedWidget_private->widget(index+1));


//        cout<<plist.size()-1-index<<endl;
        string s = username1 + "    " + time;
        QListWidgetItem *newItem1 = new QListWidgetItem;
        newItem1->setText(QString::fromStdString(s));
        newItem1->setTextColor(QColor(82,139, 139, 255));
        QListWidgetItem *newItem2 = new QListWidgetItem;
        newItem2->setText(QString::fromStdString(msg+"\n"));
        pchat->addItem(newItem1);
        pchat->addItem(newItem2);
//        ui->stackedWidget_private->setCurrentWidget(pchat);
    }



}

void MainWindow::on_tabWidget_friend_currentChanged(int index)
{
    if(index==0)
    {
        ui->request->hide();
        ui->accept_btn->hide();
        ui->reject_btn->hide();

        ui->f_userid->show();
        ui->userid->show();
        ui->f_nickname->show();
        ui->nickname->show();
        ui->f_gender->show();
        ui->gender->show();
        ui->f_birthday->show();
        ui->birthday->show();
        if(ui->listWidget->currentRow()==0 || ui->listWidget->currentRow()==-1)
        {
            ui->addf->show();
            ui->addfriend->show();
            ui->confBtn->show();
        }
        else
        {
            ui->chatBtn->show();
            ui->deleteBtn->show();
        }


    }
    else
    {
        ui->f_userid->hide();
        ui->userid->hide();
        ui->f_nickname->hide();
        ui->nickname->hide();
        ui->f_gender->hide();
        ui->gender->hide();
        ui->f_birthday->hide();
        ui->birthday->hide();
        ui->addf->hide();
        ui->addfriend->hide();
        ui->chatBtn->hide();
        ui->confBtn->hide();
        ui->deleteBtn->hide();
        if(ui->listWidget_2->currentRow()!=-1)
        {
            ui->request->show();
            ui->accept_btn->show();
            ui->reject_btn->show();
        }

    }

}

void MainWindow::receiveData_addrequest(BYTE state)
{
    if(state==0)
        QMessageBox::warning(this,tr("warning"),tr("账号不存在！"),QMessageBox::Yes);
    else
        QMessageBox::information(this,tr(""),tr("好友请求已发送"),QMessageBox::Yes);
    ui->addfriend->clear();
}

void MainWindow::receiveData_newfriend(BYTE state)
{
    friendlist.clear();
    ui->listWidget->clear();
    ui->listWidget->addItem(user);
    int body_length = 21;
    BYTE message_body[body_length];
    memmove(message_body,user.toStdString().c_str(), 21); // 写_user_id
    int index = ui->listWidget_2->currentRow();
    applicationlist.erase(applicationlist.begin()+index);
    ui->listWidget_2->takeItem(index);
    ui->listWidget_2->setCurrentRow(-1);
    ui->request->hide();
    ui->accept_btn->hide();
    ui->reject_btn->hide();
    chat_client->send(FriendList_Request, message_body, body_length);
}

void MainWindow::receiveData_update(BYTE state, string userid)
{
    if(state==0)
    {
        cout<<userid<<endl;
        if(ui->listWidget_private->count()>0)
        {
            int i = 0;
            for(i = 0; i< ui->listWidget_private->count();i++)
            {
                if(chatlist[i] == userid)
                {
                    break;
                }
            }
            if(i!=ui->listWidget_private->count())
            {
                chatlist.erase(chatlist.begin()+i);
                ui->listWidget_private->takeItem(i);
                auto pchat = ui->stackedWidget_private->widget(i+1);
                ui->stackedWidget_private->removeWidget(pchat);
                pchat->deleteLater();
            }
        }
    }
    friendlist.clear();
    ui->listWidget->clear();
    ui->listWidget->addItem(user);
    int body_length = 21;
    BYTE message_body[body_length];
    memmove(message_body,user.toStdString().c_str(), 21); // 写_user_id

    chat_client->send(FriendList_Request, message_body, body_length);
}

void MainWindow::receiveData_deletefriend(string userid)
{
    cout<<userid<<endl;
    if(ui->listWidget_private->count()>0)
    {
        int i = 0;
        for(i = 0; i< ui->listWidget_private->count();i++)
        {
            if(chatlist[i] == userid)
            {
                break;
            }
        }
        if(i!=ui->listWidget_private->count())
        {
            chatlist.erase(chatlist.begin()+i);
            ui->listWidget_private->takeItem(i);
            auto pchat = ui->stackedWidget_private->widget(i+1);
            ui->stackedWidget_private->removeWidget(pchat);
            pchat->deleteLater();
        }
    }


    friendlist.clear();
    ui->listWidget->clear();
    ui->listWidget->addItem(user);
    int body_length = 21;
    BYTE message_body[body_length];
    memmove(message_body,user.toStdString().c_str(), 21); // 写_user_id

    ui->listWidget->setCurrentRow(0);
    chat_client->send(FriendList_Request, message_body, body_length);
    ui->addf->show();
    ui->addfriend->show();
    ui->confBtn->show();
    ui->deleteBtn->hide();
    ui->chatBtn->hide();
}

void MainWindow::receiveData_update2(BYTE state)
{
    applicationlist.clear();
    ui->listWidget_2->clear();

    int body_length = 21;
    BYTE message_body[body_length];
    memmove(message_body,user.toStdString().c_str(), 21); // 写_user_id
    chat_client->send(CheckApplication_Request, message_body, body_length);
}
