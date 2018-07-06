#include <iostream>
#include "Message.hpp"
#include <boost/asio.hpp>
#include <cstring>

using namespace std;

#define PORT 8080
#define local_ip "127.0.0.1"
using namespace boost::asio;

void send_test(){
    io_service io;
    ip::tcp::endpoint ep(ip::address::from_string(local_ip), PORT);
    ip::tcp::socket socket(io);
    socket.connect(ep);
    void *p;
    const char* s = "Hello World!";
    BYTE test_message[12+5002+12+13];
    test_message[0] = 'a';
    test_message[1] = 'b';
    test_message[2] = 'c';
    test_message[3] = 'd';
    p = test_message + 4;
    *static_cast<unsigned int*>(p) = 5002;
    //memmove(test_message+12, s, 12);
    memset(test_message+12, 's', 5000);
    test_message[12+5000] = 'J';
    test_message[12+5001] = 'K';
    test_message[12+5002+0] = 'x';
    test_message[12+5002+1] = 'y';
    test_message[12+5002+2] = 'z';
    test_message[12+5002+3] = 'm';
    p = test_message + 12 + 5002 + 4;
    *static_cast<unsigned int*>(p) = 13;
    memcpy(static_cast<BYTE*>(p)+8, s, 13);
    boost::system::error_code err;
    socket.write_some(buffer(test_message));
    sleep(3);
    socket.write_some(buffer(test_message));
    socket.close();
}

int main(){
    send_test();
}