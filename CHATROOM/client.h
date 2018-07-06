#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include "Message.hpp"
#include <iostream>
#include <cstdlib>
#include <deque>
#include <string>
#include <cstring>
#include <QString>
#include <QWidget>
#include "defs.hpp"
#include "connection_base.h"
#include <QMetaType>
#include <QVariant>


using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;
using namespace std;

typedef std::deque<Message_Packet*> Message_Queue;

class Client;//前置声明
class tcp_connection_client: public tcp_connection_base     //继承base类，用于处理客户端的底层socket
{

public:
    virtual void start();
    using tcp_conn_pointer = boost::shared_ptr<tcp_connection_client>;
    static tcp_conn_pointer create(Client*c, io_context&io);
protected:
    tcp_connection_client(io_context&io, Client*c):client(c),
            tcp_connection_base(io){};
    void do_read();
    void on_read(const boost::system::error_code &err, size_t size);
    void ProcessData(size_t& size, size_t cursor = 0);
private:
    Client* client;
};

using pConn = tcp_connection_client::tcp_conn_pointer;

struct Friend                   //一个用户（好友）的结构体
{
    string uid;
    string nickname;
    string gender;
    string birthday;
};

Q_DECLARE_METATYPE(Friend)      //注册结构体，使之可以在槽函数中传递

class Client : public QObject       //客户端类，包含一个tcp_connection_client，send函数用于发送，handle函数用于处理接收的包，
{                                    //根据包类别调用不同的处理函数。并向主界面发送相应的信号
    Q_OBJECT
public:
    Client(io_context& io);     //构造，初始化socket等
//    ~Client(){delete this;};
    void send(BYTE type, BYTE* Body, size_t len);       //发送
    void Handler(Message_Packet* packet);               //处理接收的包，怎么接收看tcp_connect底层类
    void ProcessData(size_t& size, size_t cursor);      //处理字节流，将其打包为一个完整的包（即使跨块）
private slots:                                          //处理接收到的包的函数
    void handle_public_msg(Message_Packet* packet);     //公聊
    void hanlde_register(Message_Packet* packet);       //注册
    void handle_login(Message_Packet* packet);          //登录
    void handle_friendlist(Message_Packet* packet);     //申请好友列表
    void handle_application(Message_Packet* packet);    //申请好友请求列表
    void handle_private_msg(Message_Packet* packet);    //私聊
    void handle_addreturn(Message_Packet* packet);      //发送好友请求
    void handle_newfriend(Message_Packet* packet);      //同意添加好友

signals:                                                //向主界面发送的信号
    void sendData(string, string, string);
    void sendData(BYTE);
    void sendData1(BYTE);
    void sendData_login1(BYTE);
    void sendData_login2(string,string,char,string);
    void sendData_addfriend(QVariant);
    void sendData_addapplication(string);
    void sendData_deletefriend(QVariant);
    void sendData_checkonline(BYTE);
    void sendData_private(string,string, string, string);
    void sendData_addrequest(BYTE);
    void sendData_newfriend(BYTE);
    void sendData_update(BYTE,string);
    void sendData_update2(BYTE);
    void sendData_deletefriend(string);

private:
    pConn connection;
    BYTE Read_Chunk[CHUNK_SIZE];
};

#endif // CLIENT_H
