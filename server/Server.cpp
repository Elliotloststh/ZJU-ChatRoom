#include "Message.hpp"
#include "Server.hpp"
#include "Error_handle.hpp"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <array>
#include <vector>
#include "defs.hpp"
#include <cmath>
#include <algorithm>
#include <cstring>

using std::cout;
using std::endl;
using std::array;
using std::string;
using std::cerr;

inline tcp_connection_base::tcp_connection_base(io_context &io)
    : __socket(io), packet(new Message_Packet)
{

}

void tcp_connection_base::send(Message_Packet* packet){
    size_t blocks = std::ceil((Header_Size + packet->content_size())*1.0 / CHUNK_SIZE);
    auto first = new BlockType;
    memcpy(first->chunk, packet->header(), Header_Size);
    // write header
    size_t cursor,size = packet->content_size()>CHUNK_SIZE-Header_Size? 
        CHUNK_SIZE-Header_Size : packet->content_size();
    memcpy(first->chunk + Header_Size, packet->body(), size);
    // write content
    cursor = size;
    //initial cursor
    bq.push(first);
    //push first block
    for (unsigned int i = 1; i < blocks; ++i){
        auto tmp = new BlockType;
        if (cursor + CHUNK_SIZE > packet->content_size())
            size = packet->content_size() - cursor;
        else 
            size = CHUNK_SIZE;
        // calculate size
        memcpy(tmp->chunk, packet->body() + cursor, size);
        bq.push(tmp);
        cursor += size;
        //update cursor
    }
    auto &f = bq.front();
    if (blocks == 1)
        size += Header_Size;
    f->resize(size);
    // the last chunk may be unfilled
    do_write();
}

void tcp_connection_base::do_write(){
    if (bq.size() == 0)
        return ;
    // all blocks have sent
    auto &block = bq.front();
    async_write(__socket, buffer(block->chunk, block->size),bind(
        &tcp_connection_base::on_write, this, 
        placeholders::error, placeholders::bytes_transferred
    ));
}

void tcp_connection_base::on_write(const error_code &err, size_t size){
    auto block = bq.front();
    delete block;
    bq.pop();
    // free the buffer
    do_write();
}

void tcp_connection_server::do_read(){
    __socket.async_read_some(buffer(chunk), bind(
        &tcp_connection_server::on_read,this,
        placeholders::error, placeholders::bytes_transferred
    ));
}

void tcp_connection_server::start(){
    do_read();
}

void tcp_connection_server::ProcessData(size_t& size, size_t cursor){
    //Note: packet->Process and ProcessData all adopt referencing the size
    // so the size will be changed inside the functions.
    size_t tmp_sz = size;
    if (packet->Process(chunk + cursor, size) == state_code_t::complete){
        server->Handler(packet.get(), shared_from_this());
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

void tcp_connection_server::Error_Handler(const boost::system::error_code &err){
    if (err == boost::asio::error::eof){
        cerr << "Remote client closed!" << endl;
        server->RemoveClient(shared_from_this());
                //update online users
    }
    else if (err == boost::asio::error::operation_aborted){
        cerr << "Operation on socket aborted!" << endl;
    }
}

void tcp_connection_server::on_read(const error_code &err, size_t size){
    cout << "On read: " << size << " error: " << err << endl; 
    if (!err){
        ProcessData(size);
    }
    else{
        Error_Handler(err);
        return ;
    }
    do_read();
}


tcp::socket& tcp_connection_base::socket(){
    return __socket;
}

pConn tcp_connection_server::create(Server* s, io_context &io){
    return tcp_conn_pointer(new tcp_connection_server(io, s));
}

Server::Server(boost::asio::io_context &io, const std::string& config):
    __acceptor(io, tcp::endpoint(tcp::v4(), PORT)), timer(io), db_conn(new DB_Connection){
    start_accept();
    timer.expires_from_now(boost::posix_time::seconds(Check_Interval));
    // one minute
    timer.async_wait(bind(&Server::check_alive, this, placeholders::error));
    db_conn->DB_init(config);
    db_conn->del("delete from OnlineUser;");
}

Server::~Server(){
    db_conn->query("delete from OnlineUser");
    delete db_conn;
}

void Server::start_accept(){
    pConn new_conn = tcp_connection_server::create(this, __acceptor.get_io_context());
    __acceptor.async_accept(new_conn->socket(), 
        bind(&Server::handle_accept, this, new_conn, 
        boost::asio::placeholders::error));
}

void Server::check_alive(const error_code &err){
    cout << "start checking" << endl;
    if (!err){
        for (auto iter = conn_list.begin(); iter != conn_list.end(); ++iter){
            auto packet = new Message_Packet();
            char s[] = "ping";
            packet->write_protocol(CheckAlive);
            packet->write_content_size(sizeof(s));
            packet->write_encoding(ASCII);
            packet->new_body((BYTE*)s, sizeof(s));
            (*iter)->send(packet);
            delete packet;
            test_conn_map.insert({iter, true});
        }
    }
    timer.expires_from_now(boost::posix_time::seconds(5));
    // set timeout be 5
    timer.async_wait(bind(&Server::remove_dead, this, placeholders::error));
}

void Server::remove_dead(const error_code &err){
    if (!err){
        for (auto &it : test_conn_map){
            cout << "Removing" << endl;
            test_conn_map.erase(it.first);
            conn_list.erase(it.first);
        }
    }

    timer.expires_from_now(boost::posix_time::seconds(Check_Interval));
    timer.async_wait(bind(&Server::check_alive, this, placeholders::error));
}

void Server::RemoveClient(pConn conn){
    db_conn->del("delete from OnlineUser where IP='" 
        + conn->socket().remote_endpoint().address().to_string()
        +"' and port =" + 
        std::to_string(conn->socket().remote_endpoint().port()) + ";");
    //update databse
    auto it = std::find(conn_list.begin(), conn_list.end(), conn);
    conn_list.erase(it);
    test_conn_map.erase(it);
}

void Server::handle_accept(pConn new_conn, const error_code &err){
    if (!err){
        new_conn->start();
        conn_list.push_back(new_conn);
        cout << new_conn->socket().remote_endpoint().address().to_string() << endl;
        cout << new_conn->socket().remote_endpoint().port() << endl;
    }
    start_accept();
    //continue to accept connection
}

void Server::Handler(Message_Packet* packet, pConn conn){
    cout << "Message from" << conn->socket().remote_endpoint().address().to_string() << endl;
    switch(packet->protocol()){
        case PublicMessage_Request:{
            packet->write_protocol(PublicMessage_Return);
            for (auto &iter : conn_list){
                iter->send(packet);
            }
        }break;
        
        case RespondAlive:{
            if (string((char*)packet->body()) == "pong"){
                test_conn_map.erase(std::find(conn_list.begin(), conn_list.end(), conn));
            }
        }break;

        case Register_Request:{
            packet->write_protocol(Register_Return);
            // first find whether userid was registered
            auto userid = string(reinterpret_cast<char*>(packet->body()));
            auto passwd = string(reinterpret_cast<char*>(packet->body()+USER_ID_Len));
            string sql = string("select userid from User where userid = ")+ "'"+  userid + "';";
            auto res = db_conn->query(sql);
            if (res.length > 0){
                packet->body()[0] = '\0';
            }else{
                db_conn->insert(string("insert into User(userid, nickname ,Password) values('")
                + userid + "','"+ userid + "','" + passwd + "');");
                packet->body()[0] = '\1';
            }
            conn->send(packet);
        }break;

        case Register_Extra_Request:{
            packet->write_protocol(Register_Extra_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body()));
            auto nickname = string(reinterpret_cast<char*>(packet->body() + USER_ID_Len));
            auto sex = static_cast<char>(*(packet->body()+(USER_ID_Len)*2));
            auto birthday = string(reinterpret_cast<char*>(packet->body() + (USER_ID_Len)*2+1));
            string sql = string("update User set nickname='") + nickname + "',sex='"
                + sex + "', birthday = '" + birthday +"' where userid = '" + userid + "';";
            auto res = db_conn->update(sql);
            if (res){
                packet->body()[0] = '\1';
            }
            else{
                packet->body()[0] = '\0';
            }
            conn->send(packet);
        }break;
        
        case Login_Request:{
            packet->write_protocol(Login_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body()));
            auto passwd = string(reinterpret_cast<char*>(packet->body() + USER_ID_Len));
            string sql = string("select uid, userid, nickname, sex, birthday from User where userid = '")+
                userid + "'and Password = '" + passwd + "';";
            auto res = db_conn->query(sql);
            if (res.length > 0){
                int size = USER_ID_Len + NICKNAME_Len + SEX_Len + BIRTHDAY_Len, cursor = 0;
                packet->write_content_size(size);
                strcpy(reinterpret_cast<char*>(packet->body() + cursor), res.rows[0][1]);
                cursor += USER_ID_Len;
                strcpy(reinterpret_cast<char*>(packet->body() + cursor), res.rows[0][2]);
                cursor += NICKNAME_Len;
                memcpy(packet->body() + cursor, res.rows[0][3], SEX_Len);
                cursor += SEX_Len;
                memcpy(packet->body() + cursor, res.rows[0][4], BIRTHDAY_Len);
                cursor += BIRTHDAY_Len;cursor += BIRTHDAY_Len;
                packet->body()[cursor] = '\0';
                db_conn->insert(string("insert into OnlineUser values(")
                    + res.rows[0][0] + ", '" + 
                    conn->socket().remote_endpoint().address().to_string()
                    + "', '" + ONLINE + "','" + 
                    std::to_string(conn->socket().remote_endpoint().port()) + "');"
                );
            }
            else{
                packet->body()[0] = '\0';
            }
            conn->send(packet);
        }break;

        case FriendList_Request:{
            packet->write_protocol(FriendList_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body()));
            string sql = string("select user2 from Relation where user1 = '")
                + userid + "';";
            auto res = db_conn->query(sql);
            if (res.length == 0){
                BYTE s ='\0';
                packet->write_content_size(1);
                packet->new_body(&s, 1);
            }
            else{
                int size = USER_ID_Len + NICKNAME_Len + SEX_Len + BIRTHDAY_Len, cursor = 0;
                packet->write_content_size(size * res.length);
                auto buffer=new char[size*res.length];
                for (unsigned int i = 0; i <  res.length; ++i){
                    sql = string("select userid, nickname, sex, birthday from User where userid ='")
                        + res.rows[i][0] + "';";
                    auto rr = db_conn->query(sql);
                    strcpy(reinterpret_cast<char*>(buffer + cursor), rr.rows[0][0]);
                    cursor += USER_ID_Len;
                    strcpy(reinterpret_cast<char*>(buffer + cursor), rr.rows[0][1]);
                    cursor += NICKNAME_Len;
                    memcpy(buffer + cursor, rr.rows[0][2], SEX_Len);
                    cursor += SEX_Len;
                    memcpy(buffer + cursor, rr.rows[0][3], BIRTHDAY_Len);
                    cursor += BIRTHDAY_Len;
                }
                packet->new_body(reinterpret_cast<BYTE*>(buffer), size*res.length);
                delete[] buffer;
            }
            conn->send(packet);
        }break;

        case CheckOnline_Request:{
            packet->write_protocol(CheckOnline_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body())); 
            string sql = string("select state from OnlineUser natural join User where userid='")
                + userid +"';";
            auto res = db_conn->query(sql); 
            if (res.length == 0){
                packet->body()[0] = OFFLINE;
            }else{
                char state = res.rows[0][0][0];
                if (state == STEALTH){
                    state = OFFLINE;
                }else{
                    state = ONLINE;
                }
                packet->body()[0] = state;
            }
            conn->send(packet);
        }break;

        case PrivateMessage_Request:{
            packet->write_protocol(PrivateMessage_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body() + USER_ID_Len));
            string sql = string("select IP, port from OnlineUser natural join User where userid='")
                + userid + "';";
            auto res = db_conn->query(sql);
            for (auto &it : conn_list){
                if ((it->socket().remote_endpoint().address().to_string() == res.rows[0][0] &&
                    std::to_string(it->socket().remote_endpoint().port()) == res.rows[0][1])){
                    it->send(packet);
                    break;
                }
            }
            conn->send(packet);
        } break;

        case AddFriend_Request:{
            packet->write_protocol(AddFriend_Return);
            auto user1 = string(reinterpret_cast<char*>(packet->body()));
            auto user2 = string(reinterpret_cast<char*>(packet->body()+USER_ID_Len));
            string sql = string("select uid from User where userid = '") + user2 + "';";
            auto r1 = db_conn->query(sql);
            sql = string("select * from application where user1='") + 
                    user1 + "' and user2 = '" + user2 +"';";
            auto r2 = db_conn->query(sql);
            if(r1.length == 0 ){
                packet->body()[0] = '\0';
            }else if (r2.length == 1){
                packet->body()[0] = '\1';   
            }else{
                sql = string("insert into application values('") +
                    user1 + "', '" + user2 +"');";
                db_conn->insert(sql);
                packet->body()[0] = '\1';
            }
            conn->send(packet);
            sql = string("select IP, port from OnlineUser natural join User where userid = '")
                + user2 + "';";
            auto res = db_conn->query(sql);
            packet->write_protocol(UpdateApplication);
            packet->write_content_size(1);
            BYTE s = '\1';
            packet->new_body(&s, 1);
            if (res.length > 0){
                for (auto &it : conn_list){
                    if (it->socket().remote_endpoint().address().to_string() == res.rows[0][0] &&
                        std::to_string(it->socket().remote_endpoint().port()) == res.rows[0][1]){
                        it->send(packet);
                        break;
                    }
                }
            }
        }break;

        case CheckApplication_Request:{
            packet->write_protocol(CheckApplication_Return);
            auto userid = string(reinterpret_cast<char*>(packet->body()));
            string sql = string("select user1 from application where user2 = '")
                + userid + "';";
            auto res = db_conn->query(sql);
            if (res.length == 0){
                packet->write_content_size(1);
                BYTE s = '\0';
                packet->new_body(&s, 1);
            }else{
                auto buffer = new char[USER_ID_Len * res.length];
                int cursor = 0;
                for (unsigned int i = 0; i < res.length; ++i){
                    strcpy(buffer + cursor, res.rows[i][0]);
                    cursor += USER_ID_Len;
                }
                packet->write_content_size(cursor);
                packet->new_body(reinterpret_cast<BYTE*>(buffer), cursor);
                delete[] buffer;
            }
            conn->send(packet);
        }break;

        case DeleteFriend_Request:{
            packet->write_protocol(UpdateFriendList);
            auto user1 = string(reinterpret_cast<char*>(packet->body()));
            auto user2 = string(reinterpret_cast<char*>(packet->body()+USER_ID_Len));
            string sql = string("delete from Relation where user1='") + 
                user1 + "' and user2 = '" + user2 + "';";
            db_conn->del(sql);
            sql = string("delete from Relation where user1='") + 
                user2 + "' and user2 = '" + user1 + "';";
            db_conn->del(sql);
            sql = string("select IP, port from OnlineUser natural join User where userid='")
                + user2 + "';";
            packet->body()[0] = '\0';
            strcpy(reinterpret_cast<char*>(packet->body()+1), user1.data());
            auto res = db_conn->query(sql);
            if (res.length > 0){
                for (auto &it : conn_list){
                    if (it->socket().remote_endpoint().address().to_string() == res.rows[0][0] &&
                        std::to_string(it->socket().remote_endpoint().port()) == res.rows[0][1]){
                        it->send(packet);
                        break;
                    }
                }
            }
            packet->write_protocol(DeleteFriend_Return);
            strcpy(reinterpret_cast<char*>(packet->body()+1), user2.data());
            conn->send(packet);
        }break;

        case AgreeApplication_Request:{
            packet->write_protocol(AgreeApplication_Return);
            auto user2 = string(reinterpret_cast<char*>(packet->body()));
            auto user1 = string(reinterpret_cast<char*>(packet->body() + USER_ID_Len));
            auto acc = static_cast<char>(packet->body()[USER_ID_Len*2]);
            string sql = string("delete from application where user2='") + 
                user2 + "' and user1 ='" + user1 +"';";
            db_conn->del(sql);
            if (acc != '\0')
            {
                sql = string("insert into Relation values('") + user1 + "', '" + user2
                    + "');";
                db_conn->insert(sql);
                sql = string("insert into Relation values('") + user2 + "', '" + user1
                    + "');";
                db_conn->insert(sql);
                packet->body()[0] = '\1';
                strcpy(reinterpret_cast<char*>(packet->body()+1), user1.data());
                packet->write_content_size(1);
                conn->send(packet);
                sql = string("select IP, port from OnlineUser natural join User where userid='")
                    + user1 + "';";
                auto res = db_conn->query(sql);
                for (auto &it : conn_list){
                    if ((it->socket().remote_endpoint().address().to_string() == res.rows[0][0] &&
                        std::to_string(it->socket().remote_endpoint().port()) == res.rows[0][1])){
                        packet->write_protocol(UpdateFriendList);
                        it->send(packet);
                        break;
                    }
                }
            }
        }break;
    }
}

