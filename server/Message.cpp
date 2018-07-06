#include "Message.hpp"
#include <cstring>
#include <boost/shared_ptr.hpp>
#include <iostream>
using std::cout;
using std::endl;

state_code_t Message_Packet::Process(BYTE* chunk, size_t &size){
    if (h_cursor < Header_Size){
        size_t read_bytes;
        read_bytes = write_header(chunk, size);
        h_cursor += read_bytes;
        if (h_cursor == Header_Size){
            void* p = Header + Content_Length_Off;
            content_length = *static_cast<unsigned int*>(p);
            Body = boost::shared_ptr<BYTE>(new BYTE[content_length]);
            b_cursor += write_body(chunk + read_bytes, size);
        }
    }
    else{
        b_cursor += write_body(chunk, size);
    }
    if (b_cursor == content_length){
        complete = true;
    }
    return complete == true ? state_code_t::complete : state_code_t::incomplete;
}

size_t Message_Packet::write_header(BYTE* chunk, size_t &size){
    size_t write_size = size < Header_Size - h_cursor ? 
        size : Header_Size - h_cursor;
    size -= write_size;
    std::memmove(Header + h_cursor, chunk, write_size);
    return write_size;
}

size_t Message_Packet::write_body(BYTE* chunk, size_t &size){
    size_t write_size = content_length - b_cursor > size ? 
        size : content_length - b_cursor;
    size -= write_size;
    std::memmove(Body.get() + b_cursor, chunk, write_size);
    return write_size;
}

state_code_t Message_Packet::Clear(){
    Body.reset();
    complete = false;
    h_cursor = 0;
    b_cursor = 0;
    return state_code_t::success;
}

void Message_Packet::new_body(BYTE* chunk, size_t size){
    Body = boost::shared_ptr<BYTE>(new BYTE[size]);
    memcpy(Body.get(), chunk, size);
}