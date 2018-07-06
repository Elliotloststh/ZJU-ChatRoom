#include "mainwindow.h"
#include <QApplication>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

boost::asio::io_context io;
Client* chat_client = new Client(io);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString user_name = "Elliot";

    MainWindow w(user_name);
    w.show();
    boost::thread t(bind(&io_context::run, &io));
//    t.join();
    return a.exec();
}
