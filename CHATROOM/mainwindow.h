//主界面类，大致流程是界面操作，调用chat_client的send函数发送包，chat_client收到包，向界面发送信号，界面进行相应的处理
//如自动刷新好友列表操作，是服务端发送一个刷新信号，client接收之后，发槽函数给主界面，主界面重新发送一个请求好友列表的操作，client发送，服务端接收到，发送一个好友列表
//client接收到，传给主界面，主界面加载这个列表
//如请求进行私聊是主界面调用client发送一个检查对方是否在线信号，服务端返回是或否，client接收到后给主界面进行相应处理



#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "client.h"
#include <vector>
#include "chat_record.h"

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent,QString user_name,QString _nickname, QChar _gender, QString _birthday);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_private_clicked();

    void on_record_private_clicked();

    void on_record_clicked();

    //void on_listWidget_private_itemSelectionChanged();

    void on_checkBox_stateChanged(int state);

    //void on_pushButton_2_clicked();

    void on_open_file_clicked();

private:
    Ui::MainWindow *ui;
    QString user;                   //当前用户信息
    QString nickname;
    QChar gender;
    QString birthday;
    chat_record *record;

    vector<Friend> friendlist;              //好友列表
    vector<string> chatlist;                //私聊窗口列表，存储id（因为界面上是nickname
    vector<string> applicationlist;         //好友请求列表

private slots:
    void switchfriend();
    // 学姐


    void singleclicked(QListWidgetItem *item);

    void singleclicked_2(QListWidgetItem *item);

    void on_deleteBtn_clicked();

    void on_chatBtn_clicked();
    void on_confBtn_clicked();

    void receiveData_Public(string username, string msg, string time); //收到公聊消息

    void receiveData_addfriend(QVariant variant);       //增加一个好友（好友列表通过调用多次实现）

    void receiveData_addapplication(string userid);     //增加一个好友请求（同上）

    void receiveData_checkonline(BYTE response);        //判断对方是否在线，在则可以进行私聊

    void receiveData_Private(string username1,string username2, string msg, string time);       //收到私聊消息

    void receiveData_addrequest(BYTE state);        //发送好友请求

    void receiveData_newfriend(BYTE state);         //同意添加好友

    void receiveData_deletefriend(string userid);   //删除好友后自己收到的信号，发给对方的只是一个刷新好友列表的信

    void receiveData_update(BYTE state, string userid); //更新好友列表

    void receiveData_update2(BYTE state);           //更新申请列表

    void on_tabWidget_friend_currentChanged(int index);
    void on_accept_btn_clicked();
    void on_reject_btn_clicked();
};

#endif // MAINWINDOW_H
