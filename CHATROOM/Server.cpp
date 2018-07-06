#include "Message.hpp"
#include "Server.hpp"
#include "Error_handle.hpp"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <array>
#include "defs.hpp"
#include <cmath>
using std::cout;
using std::endl;
using std::array;

tcp_connection_base::tcp_connection_base(io_context &io)
    : __socket(io), packet(new Message_Packet)
{

}

void tcp_connection_base::send(Message_Packet* packet){
    size_t blocks = std::ceil((Header_Size + packet->content_size())*1.0 / CHUNK_SIZE);
    BlockType tmp;
    memcpy(tmp.chunk, packet->header(), Header_Size);
    // write header
    size_t cursor,size = packet->content_size()>CHUNK_SIZE-Header_Size?
        CHUNK_SIZE-Header_Size : packet->content_size();
    memcpy(tmp.chunk + Header_Size, packet->body(), size);
    // write content
    cursor = size;
    //initial cursor
    bq.push(tmp);
    //push first block
    for (unsigned int i = 1; i < blocks; ++i){
        if (size + CHUNK_SIZE > packet->content_size())
            size = packet->content_size() - size;
        else
            size = CHUNK_SIZE;
        // calculate size
        memcpy(tmp.chunk + cursor + Header_Size, packet->body() + cursor, size);
        bq.push(tmp);
        cursor += size;
        //update cursor
    }
    auto &f = bq.front();
    if (blocks == 1)
        size += Header_Size;
    f.resize(size);
    // the last chunk may be unfilled
    do_write();
}

void tcp_connection_base::do_write(){
    if (bq.size() == 0)
        return ;
    // all blocks have sent
    auto block = bq.front();
    bq.pop();
    async_write(__socket, buffer(block.chunk, block.size),bind(
        &tcp_connection_base::on_write, this,
        placeholders::error, placeholders::bytes_transferred
    ));
}

void tcp_connection_base::on_write(const error_code &err, size_t size){
    do_write();
}



tcp::socket& tcp_connection_base::socket(){
    return __socket;
}


