#ifndef __SERVER_H
#define __SERVER_H

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <array>
#include "Message.hpp"
#include <memory.h>
#include <map>
#include "Database.hpp"

using std::string;
using boost::asio::ip::tcp;
using namespace boost::asio;
using boost::bind;
using boost::system::error_code;
using std::map;

constexpr auto PORT = 8080;

struct Block_t{
    size_t size;
    BYTE* chunk;
    Block_t():size(CHUNK_SIZE){
        chunk = new BYTE[CHUNK_SIZE];
    }
    
    Block_t(size_t s):size(CHUNK_SIZE){
        chunk = new BYTE[CHUNK_SIZE];
    }

    void resize(size_t s){
        if (s == size)
            return;
        else {
            size = s;
            chunk = (BYTE*)realloc(chunk, size);
        }
    }

    ~Block_t(){
        delete[] chunk;
    }
};
using BlockType = Block_t;
using Block_Queue = std::queue<BlockType*>;

class Server;

class tcp_connection_base{
    public:

        tcp::socket& socket();
        virtual void start() = 0;
        void send(Message_Packet*);

    protected:
        tcp_connection_base(io_context &io);
        void on_write(const error_code &err, size_t size);
        void do_write();

    protected:
        tcp::socket __socket;
        BYTE chunk[CHUNK_SIZE];
        Block_Queue bq;
        boost::shared_ptr<Message_Packet> packet;
};

class tcp_connection_server:public tcp_connection_base,
    public boost::enable_shared_from_this<tcp_connection_server>
{
    public:
        using tcp_conn_pointer = boost::shared_ptr<tcp_connection_server>;
        static tcp_conn_pointer create(Server*s, io_context&io);
        virtual void start();

    protected:
        tcp_connection_server(io_context&io, Server*s):server(s),
                tcp_connection_base(io){}
        void do_read();
        void on_read(const error_code &err, size_t size);
        void ProcessData(size_t& size, size_t cursor = 0);
        void Error_Handler(const boost::system::error_code &err);
    
    private:
        Server* server;
};

using pConn = tcp_connection_server::tcp_conn_pointer;
class Server{
    public:
        Server(boost::asio::io_context &io, const std::string&);
        ~Server();
        void Handler(Message_Packet*, pConn conn);
        void RemoveClient(pConn);
    private:
        void start_accept();
        void handle_accept(pConn new_conn, const error_code &err);
        void check_alive(const error_code &err);
        void remove_dead(const error_code &err);
    private:
        tcp::acceptor __acceptor;
        std::vector<pConn> conn_list;
        map<std::vector<pConn>::iterator, bool> test_conn_map;
        deadline_timer timer;
        DB_Connection *db_conn;
};

#endif