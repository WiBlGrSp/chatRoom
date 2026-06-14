#include "client.h"
#include "common/log.h"
#include "common/messageTransporter.h"
#include "common/protocol.h" 
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include "common/safe.h"

Client::Client(const char* ip, int port, const char* server_ip, int server_port)
    : cli_ip_(ip), cli_port_(port), ser_ip_(server_ip), ser_port_(server_port), status_(CliType::DEFAULT)
{
    run();
}

Client::~Client()
{
}

void Client::login()
{
    //读取用户名
    printf("请输入用户名以登录:");
    fflush(stdout);
    Safe::input(this->name_, sizeof(this->name_));

    if (MessageTransporter(sock_).sendMessage(Message(MsgType::LOGIN, this->name_)) == -1)
    {
        log(LogLevel::ERROR, "login error");
    }
    status_ = CliType::LOGIN;
}

void Client::logout()
{
    if (MessageTransporter(sock_).sendMessage(Message(MsgType::LOGOUT, this->name_)) == -1)
    {
        log(LogLevel::ERROR, "logout error");
    }
    status_ = CliType::LOGOUT;
}


//用于发送消息
void Client::messageHandler()
{
    char buf[Message::kContentSize];
    MessageTransporter msg_trans(this->sock_);
    //自动登录
    login();
    while(true)
    {
        //读取终端输入
        Safe::input(buf, sizeof(buf));
        //根据输入类型,分类处理
        if (strcasecmp(buf, "login") == 0)
        {
            if (status_ == CliType::DEFAULT || status_ == CliType::LOGOUT)
                login();
        } else if (strcasecmp(buf, "logout") == 0)
        {
            if (status_ == CliType::LOGIN)
                logout();
        } else if (strcasecmp(buf, "exit") == 0)
        {
            if (status_ == CliType::LOGIN)
                logout();
            break;
        } else if (status_ == CliType::LOGIN)
        {
            msg_trans.sendMessage(Message(MsgType::CHAT, this->name_, buf));
        }
    }
}

void Client::run()
{
    //初始化套接字
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_ == -1)
        log(LogLevel::ERROR, "create error");
    else
        log(LogLevel::INFO, "create success");
    
    //1.1设置端口快速重用
    int opt = 1;
    setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    //发送连接请求
    struct sockaddr_in addr_ser;
    memset(&addr_ser, 0, sizeof(addr_ser));
    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons(ser_port_);
    addr_ser.sin_addr.s_addr = inet_addr(ser_ip_);

    if (connect(sock_, (struct sockaddr*)(&addr_ser), sizeof(addr_ser)) == -1)
    {
        log(LogLevel::ERROR, "connect error");
        return;
    }
    else
        log(LogLevel::INFO, "connect success");

    //创建分支线程用于发送消息
    std::thread th([this]{
        messageHandler();
    });
    th.detach();

    //接收服务端消息并打印
    MessageTransporter msg_trans(sock_);
    Message m;
    while(true)
    {
        int res = msg_trans.recvMessage(m);
        if (res == -1)
            log(LogLevel::ERROR, "recv error");
        else if (res == 0)
        {
            log(LogLevel::INFO, "服务器已下线");
            break;
        }
        else
        {
            m.print();
        }
    }   
}