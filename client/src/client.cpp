#include "client.h"
#include "common/log.h"
#include "common/messageTransporter.h"
#include "common/protocol.h" 
#include <cstdlib>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <unistd.h>
#include "common/safe.h"

Client::Client(const char* ip, int port, const char* server_ip, int server_port)
    : cli_ip_(ip), cli_port_(port), ser_ip_(server_ip), ser_port_(server_port),
     status_(CliType::DEFAULT),msg_trans_(-1,true),running_(true)
{
    run();
}

Client::~Client()
{
    shutdown(sock_,SHUT_RDWR);
}

//用于和用户交互
void Client::menu()
{
    while(running_){
        switch (status_) {
            case CliType::DEFAULT:
                loginMenu();
                break;
            case CliType::LOGIN:
                chatMenu();
                break;
        }
    }
    log(LogLevel::INFO,"离开主菜单");
}

void Client::loginMenu()
{
    log(LogLevel::INFO,"进入登录页面");
    while(status_!=CliType::LOGIN)
    {
        //输出提示信息
        printf("请输入用户名以登录:");
        fflush(stdout);

        //读取用户输入
        Safe::input(this->name_, sizeof(this->name_));
        
        //执行登录业务
        Message msg(MsgType::LOGIN,this->name_);
        int res = loginHandler(msg);
        if(res == 0)
        {
            log(LogLevel::INFO,"登录成功,进入聊天室");
        }else if(res == -1)
        {
            log(LogLevel::WARN,"登录失败,用户名重复");
        }
    }
    log(LogLevel::INFO,"离开登录页面");
}

void Client::chatMenu()
{
    log(LogLevel::INFO,"进入聊天页面");
    //接收消息
    std::thread th_recv([this]{
        chatRecvHandler();
    });
    th_recv.detach();

    char buf[Message::kContentSize];
    while(status_==CliType::LOGIN)
    {
        printf("请输入聊天信息:");fflush(stdout);
        Safe::input(buf, sizeof(buf));
        if(strcmp(buf,"/logout") == 0)
        {
            int res = logoutHandler();
            if(res == -1)
            {
                log(LogLevel::WARN,"登出失败...");
            }else if (res == 0) {
                log(LogLevel::INFO,"登出成功...");
            }
        }else if (strcmp(buf, "/exit") == 0)
        {
            int res = exitHandler();
            if(res == -1)
            {
                log(LogLevel::WARN,"登出失败...");
            }else if (res == 0) {
                status_ = CliType::DEFAULT;
                log(LogLevel::INFO,"退出成功...");
            }
        }else {
            msg_trans_.sendMessage(Message(MsgType::CHAT,name_,buf));
        }
    }
    log(LogLevel::INFO,"离开聊天页面");
}

/*
功能:执行登录业务

向服务器发送登录请求
接收服务器响应报文
解析响应报文
返回登录是否成功
*/
int Client::loginHandler(Message&msg)
{
    msg_trans_.sendMessage(msg);
    Message response;
    msg_trans_.recvMessage(response,
        {MsgType::LOGIN_OK,MsgType::LOGIN_FAIL});
    if(response.type_==MsgType::LOGIN_OK)
    {
        status_ = CliType::LOGIN;
        return 0;
    }else if(response.type_ == MsgType::LOGIN_FAIL)
    {
        return -1;
    }
    return -2;
}

int Client::logoutHandler()
{
    //发送请求报文
    msg_trans_.sendMessage(Message(MsgType::LOGOUT,name_));
    
    //接收响应报文
    Message response;
    msg_trans_.recvMessage(response,{MsgType::LOGOUT_OK,MsgType::LOGOUT_FAIL});
    
    
    if(response.type_==MsgType::LOGOUT_OK)
    {
        //登出成功
        status_=CliType::DEFAULT;
        msg_trans_.pushMessage(Message(MsgType::QUIT));
        return 0;
    }else if(response.type_ == MsgType::LOGOUT_FAIL)
    {
        //登出失败
        return -1;
    }
    return -2;
}

void Client::chatRecvHandler()
{
    //接收服务端提示信息并打印
    Message m;
    while(status_==CliType::LOGIN)
    {
        int res = msg_trans_.recvMessage(m,{MsgType::CHAT,MsgType::INFO,MsgType::QUIT});
        if(res >0)
        {
            m.print();
        }
    }
    log(LogLevel::INFO,"chatRecvHandler end");
}

int Client::exitHandler()
{
    int res1 = logoutHandler();
    if(res1 == -1)
        return -1;
    status_ = CliType::DEFAULT;
    running_ = false;
    return 0;
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
    msg_trans_.setSock(sock_);
    //创建分支线程用于接收消息,并分发
    std::thread th([this]{
        msg_trans_.collectMessage();
    });
    th.detach();
    //进入主菜单
    menu();
    close(sock_);
}