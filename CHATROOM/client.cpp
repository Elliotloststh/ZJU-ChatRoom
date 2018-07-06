#include "client.h"
#include <QDateTime>
#include <cstring>



using std::cout;
using std::endl;

void tcp_connection_client::start(){
    tcp::endpoint ep(address_v4::from_string("10.189.62.92"), PORT);
    __socket.async_connect(ep,boost::bind(&tcp_connection_client::do_read, this));
}

void tcp_connection_client::do_read(){
    __socket.async_read_some(buffer(chunk), bind(
        &tcp_connection_client::on_read,this,
        boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
    ));
}

void tcp_connection_client::on_read(const boost::system::error_code &err, size_t size){
//    cout<<"on read"<<endl;
    if (!err || err == boost::asio::error::eof){
        ProcessData(size);
    }
    else{
        Error_Handle(err);
    }
    do_read();
}

void tcp_connection_client::ProcessData(size_t& size, size_t cursor){
    //Note: packet->Process and ProcessData all adopt referencing the size
    // so the size will be changed inside the functions.
    size_t tmp_sz = size;
    if (packet->Process(chunk + cursor, size) == state_code_t::complete){
        client->Handler(packet.get());
        packet->Clear();
        cursor += tmp_sz - size;
        //once the message is handled, the packet is out dated.
        if (size != 0){
            // means next packet is already in the chunk
            ProcessData(size, cursor);
            //recursively process all the data
        }
    }
}

inline pConn tcp_connection_client::create(Client* c, io_context &io){
    return tcp_conn_pointer(new tcp_connection_client(io, c));
}

void Client::Handler(Message_Packet* packet){
    switch(packet->protocol())
    {
        case PublicMessage_Return:
        {
            handle_public_msg(packet);
            break;
        }
        case CheckAlive:
        {
            packet->write_protocol(RespondAlive);
            *(packet->body()+1) = 'o';
            connection->send(packet);
            break;
        }
        case Register_Return:
        {
            hanlde_register(packet);
            break;
        }
        case Register_Extra_Return:
        {
            BYTE state = *(packet->body());
            emit sendData1(state);
            break;
        }
        case Login_Return:
        {
            handle_login(packet);
            break;
        }
        case FriendList_Return:
        {
            handle_friendlist(packet);
            break;
        }
        case CheckOnline_Return:
        {
            BYTE state = *(packet->body());
            emit sendData_checkonline(state);
            break;
        }
        case PrivateMessage_Return:
        {
            handle_private_msg(packet);
            break;
        }
        case AddFriend_Return:
        {
            handle_addreturn(packet);
            break;
        }
        case CheckApplication_Return:
        {
            handle_application(packet);
            break;
        }
        case AgreeApplication_Return:
        {
            handle_newfriend(packet);
            break;
        }
        case UpdateFriendList:
        {
            BYTE state = *(packet->body());
            string uid = string((reinterpret_cast<char*>(packet->body()+1)));
            emit sendData_update(state,uid);
            break;
        }
        case DeleteFriend_Return:
        {
            string uid = string((reinterpret_cast<char*>(packet->body()+1)));
            emit sendData_deletefriend(uid);
            break;
        }
        case UpdateApplication:
        {
            BYTE state = *(packet->body());
            emit sendData_update2(state);
        }
    }
}

Client::Client(io_context& io)
{
    qRegisterMetaType<string>("string");
    connection = tcp_connection_client::create(this, io);
    connection->start();

}


void Client::send(BYTE type, BYTE* Body, size_t len)
{
    auto message = new Message_Packet();
    BYTE  content_type = 0;
    BYTE extra_msg_size = 0;
    BYTE encoding = 0;
    message->write_protocol(type);
    message->write_content_type(content_type);
    message->write_extra_msg_size(extra_msg_size);
    message->write_encoding(encoding);
    message->write_content_size(len);
    QDateTime time = QDateTime::currentDateTime();
    uint32_t timeT = time.toTime_t();
    message->write_timestamp(timeT);

    message->new_body(Body, len);
    connection->send(message);
}

void Client::handle_public_msg(Message_Packet* packet)
{
    char* username = new char[21];
    memcpy(username, packet->body(), 20);
    username[20] = '\0';
    string user = string(username);
    u_int32_t timestamp = packet->timestamp();
    string time = QDateTime::fromTime_t(timestamp).toString("yyyy-MM-dd hh:mm:ss").toStdString();
    char* msg = new char[packet->content_size()+1-20];
    memcpy(msg, packet->body()+20, packet->content_size()-20);
    msg[packet->content_size()-20] = '\0';
    string message = string(msg);
    emit sendData(username, message, time);
}

void Client::hanlde_register(Message_Packet* packet)
{

    BYTE state = *(packet->body());
    emit sendData(state);
}

void Client::handle_login(Message_Packet* packet)
{
    if(*(packet->body()) == 0)
    {
        emit sendData_login1(0);
        return;
    }
    string user_name = string((reinterpret_cast<char*>(packet->body())));
    string nickname = string((reinterpret_cast<char*>(packet->body()+21)));
    char gender = *(reinterpret_cast<char*>(packet->body()+42));
    char *ch = new char[11];
    memcpy(ch, packet->body()+43, 10);
    ch[10] = '\0';
    string birthday = string(ch);
    emit sendData_login2(user_name, nickname, gender, birthday);
}

void Client::handle_friendlist(Message_Packet* packet)
{
    if(*(packet->body()) == 0)
    {
        return;
    }
    int number = packet->content_size()/53;
    int offset = 0;
    for(int i = 0; i < number; i++)
    {
        string user_name = string((reinterpret_cast<char*>(packet->body()+offset)));
        string nickname = string((reinterpret_cast<char*>(packet->body()+offset+21)));
        char _gender = *(reinterpret_cast<char*>(packet->body()+offset+42));
        string gender;
        if(_gender == 'M')
            gender = "男";
        else
            gender = "女";
        char *ch = new char[11];
        memcpy(ch, packet->body()+offset+43, 10);
        ch[10] = '\0';
        string birthday = string(ch);
        Friend one {user_name, nickname, gender, birthday};
//        cout<<one.uid<<one.nickname<<one.gender<<one.birthday<<endl;
        QVariant variant;
        variant.setValue(one);
        emit sendData_addfriend(variant);
        offset += 53;
    }
}

void Client::handle_application(Message_Packet* packet)
{
    if(*(packet->body()) == 0)
    {
        return;
    }
    int number = packet->content_size()/21;
    for(int i = 0; i < number; i++)
    {
        string userid = string((reinterpret_cast<char*>(packet->body())));
        emit sendData_addapplication(userid);
    }
}

void Client::handle_private_msg(Message_Packet* packet)
{
    u_int32_t timestamp = packet->timestamp();
    string time = QDateTime::fromTime_t(timestamp).toString("yyyy-MM-dd hh:mm:ss").toStdString();
    string username1 = string((reinterpret_cast<char*>(packet->body())));
    string username2 = string((reinterpret_cast<char*>(packet->body()+21)));
    string message = string((reinterpret_cast<char*>(packet->body()+42)));
    emit sendData_private(username1,username2, message, time);
}

void Client::handle_addreturn(Message_Packet* packet)
{
    BYTE state = *(packet->body());
    emit sendData_addrequest(state);
}

void Client::handle_newfriend(Message_Packet* packet)
{
    BYTE state = *(packet->body());
    emit sendData_newfriend(state);
}


/*
void Client::handle_connect(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cout << "read socket connect success" << std::endl;
        async_read(socket_r, buffer(read_msg.Header, 12), //读取数据报头
                    boost::bind(&Client::handle_read_header, this, boost::asio::placeholders::error));

    }
    else
    {
        std::cout << "read socket connect fail" << std::endl;
    }

}

void Client::handle_read_header(const boost::system::error_code& error)
{
    if (!error && read_msg.content_length!=0) //分别处理数据报头和数据部分
    {
        std::cout << "in the !error" << std::endl;
        async_read(socket_r, buffer(read_msg.Body.get(), read_msg.content_length),
                   boost::bind(&Client::handle_read_body, this, boost::asio::placeholders::error));//读取数据包数据
    }
    else
    {
        do_close();
    }
}

void Client::handle_read_body(const boost::system::error_code& error)
{
    if (!error)
    {
        BYTE type = read_msg.protocol();
        if(type == 20)
        {
            QString one;

        }

        async_read(socket_r, buffer(read_msg.Header, Header_Size), //在这里读取下一个数据包头
                    boost::bind(&Client::handle_read_header, this, boost::asio::placeholders::error)); //完成一次读操作（处理完一个数据包）  进行下一次读操作
    }
    else
    {
        do_close();
    }
}



void Client::test(const boost::system::error_code& error)
{
    if(!error)
        std::cout << "write socket connect success" << std::endl;
    else
        std::cout << "write socket connect fail" << std::endl;
}


void Client::write(Message_Packet * msg, const string& addr,unsigned port)
{
//    cout<<"1"<<endl;
    tcp::endpoint ep(address_v4::from_string(addr), port);
    socket_w.async_connect(ep,boost::bind(&Client::test, this,
                                          boost::asio::placeholders::error));

    socket_w.get_io_service().post(boost::bind(&Client::do_write, this, msg, addr, port));
}

void Client::do_write(Message_Packet * msg, const string& addr,unsigned port)
{
//    cout<<"2"<<endl;
    bool write_in_progress = !write_msg_queue.empty(); //空的话变量为false
    write_msg_queue.push_back(msg); //把要写的数据push至写队列
    if (!write_in_progress)//队列初始为空 push一个msg后就有一个元素了
    {
        BYTE* tmp=new BYTE[msg->size()];
        memcpy(tmp, msg->Header, 12);
        memcpy(tmp+12, msg->Body.get(),msg->size()-12);
        async_write(socket_w,buffer(tmp, msg->size()),
                    bind(&Client::handle_write, this, boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred,
                         addr, port));
    }
}

void Client::handle_write(const boost::system::error_code& error,size_t bytes_transferred, const string& addr,unsigned port)
{
//    cout<<"3"<<endl;
    if (!error)
    {
        cout<<"send "<<bytes_transferred<<" BYTE"<<endl;
        write_msg_queue.pop_front();//刚才处理完一个数据 所以要pop一个
        if (!write_msg_queue.empty())
        {
            BYTE* tmp=new BYTE[write_msg_queue.front()->size()];
            memcpy(tmp, write_msg_queue.front()->Header, 12);
            memcpy(tmp+12, write_msg_queue.front()->Body.get(),write_msg_queue.front()->size()-12);

            async_write(socket_w,buffer(tmp, write_msg_queue.front()->size()),
                    bind(&Client::handle_write, this, boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred, addr, port)); //循环处理剩余的消息
        }
        else
        {
            do_close();
//            tcp::endpoint ep(address_v4::from_string(addr), port);
//            socket_w.async_connect(ep,boost::bind(&Client::test, this,
//                                                  boost::asio::placeholders::error));
        }
    }
    else
    {
        do_close();
    }
}

void Client::close()
{
    socket_w.get_io_service().post(boost::bind(&Client::do_close, this)); //这个close函数是客户端要主动终止时调用  do_close函数是从服务器端
    //socket_r.get_io_service().post(boost::bind(&Client::do_close, this)); //读数据失败时调用
}

void Client::do_close()
{
    socket_w.close();
    //socket_r.close();
}
*/
