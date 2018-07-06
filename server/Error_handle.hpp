#ifndef __ERROR_HANDLE_H
#define __ERROR_HANDLE_H
#include <boost/asio.hpp>
#include "Server.hpp"

void Error_Handle(const boost::system::error_code &err);

#endif