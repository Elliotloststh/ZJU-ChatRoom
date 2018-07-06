#ifndef __ERROR_HANDLE_H
#define __ERROR_HANDLE_H
#include <boost/asio.hpp>

enum state_code_t{
    success = 0,
    complete,
    incomplete,
    fail
};

void Error_Handle(const boost::system::error_code &err);

#endif