#include "mainwindow.h"
#include "login.h"
#include <QApplication>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

//定义一个全局的io服务，一个cient实例

boost::asio::io_context io;
Client* chat_client = new Client(io);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    boost::thread t(bind(&io_context::run, &io));
    login log;          //首先进入登录界面
    log.show();

    return a.exec();
}
