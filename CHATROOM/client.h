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
#include "Server.hpp"


using namespace boost::asio;
using namespace boost::asio::ip;
using boost::system::error_code;
using namespace std;

typedef std::deque<Message_Packet*> Message_Queue;
class Client;
class tcp_connection_client: public tcp_connection_base
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

class Client : public QObject
{
    Q_OBJECT
public:
    Client(io_context& io);
    void send(BYTE type, BYTE* Body, size_t len);
    void Handler(Message_Packet* packet);
    void ProcessData(size_t& size, size_t cursor);
private slots:
    void public_msg(string username, string msg, string time);

signals:
    void sendData(string username, string msg, string time);

private:
    pConn connection;
    BYTE Read_Chunk[CHUNK_SIZE];
};

#endif // CLIENT_H
