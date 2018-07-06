//与服务器的sokect连接共用这个基类，可以实现跨块的数据传输

#ifndef CONNECTION_BASE_H
#define CONNECTION_BASE_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <array>
#include "Error_handle.hpp"
#include "Message.hpp"
#include <memory.h>

using std::string;
using boost::asio::ip::tcp;
using namespace boost::asio;
using boost::bind;
using boost::system::error_code;

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


class tcp_connection_base: public boost::enable_shared_from_this<tcp_connection_base> {
    public:

        tcp::socket& socket();
        virtual void start() = 0;
        void send(Message_Packet*);

    protected:
        tcp_connection_base(io_context &io);
        void on_write(const boost::system::error_code &err, size_t size);
        void do_write();

    protected:
        tcp::socket __socket;
        BYTE chunk[CHUNK_SIZE];
        Block_Queue bq;
        boost::shared_ptr<Message_Packet> packet;
};


#endif // CONNECTION_BASE_H
