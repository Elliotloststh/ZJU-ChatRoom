#include "Server.hpp"
#include <iostream>

int main(){
    try
    {
        boost::asio::io_context io_context;
        Server server(io_context, "DB_config.json");
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}