#include"../include/client.h"
#include"../include/log.h"
#include"../include/protocol.h" 
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<thread>

Client::Client(const char* ip, int port, const char* server_ip, int server_port)
    : cli_ip(ip), cli_port(port), ser_ip(server_ip), ser_port(server_port),status(cliType::DEFAULT)
{
    run();
}

Client::~Client()
{
}
void Client::login()
{

    //读取用户名
    char buf[BUFSIZE];
    printf("请输入用户名以登录:");
    fflush(stdout);
    bzero(this->name,sizeof(this->name));
    fgets(this->name,sizeof(this->name),stdin);
    this->name[strlen(name)-1]='\0';
    
    //组装消息
    msg m;
    strncpy(m.name,this->name,USER_NAME_LENGTH);
    m.type = msg_type::LOGIN; 

    m.serialize(buf,BUFSIZE);


    if(send(sock,buf,BUFSIZE,0)==-1)
    {
        log(LogLevel::ERROR,"login error");
    }
    status = cliType::LOGIN;
}
void Client::logout()
{
    msg m;
    char buf[BUFSIZE];

    m.type = msg_type::LOGOUT; 
    strncpy(m.name, this->name,USER_NAME_LENGTH);
    bzero(m.content,CONTENT_LENGTH);

    m.serialize(buf, BUFSIZE);
    if(send(sock,buf,BUFSIZE,0)==-1)
    {
        log(LogLevel::ERROR,"logout error");
    }
    status = cliType::LOGOUT;
}


//用于发送消息
void Client::messageHandler()
{
    char buf[BUFSIZE];
    //自动登录
    login();
    while(true)
    {
        //读取终端输入
        fgets(buf,BUFSIZE,stdin);
        buf[strlen(buf)-1] = '\0';

        //根据输入类型,分类处理
        if(strcasecmp(buf,"login")==0)
        {
            if(status == cliType::DEFAULT || status==cliType::LOGOUT)
                login();
        }else if(strcasecmp(buf,"logout")==0)
        {
            if(status == cliType::LOGIN)
                logout();
        }else if(strcasecmp(buf,"exit")==0)
        {
            if(status ==cliType::LOGIN)
                logout();
            break;
        }else if(status == cliType::LOGIN)
        {
            msg m;
            m.type = msg_type::CHAT;
            strncpy(m.name,this->name,USER_NAME_LENGTH);
            strncpy(m.content,buf,CONTENT_LENGTH);
            
            m.serialize(buf,BUFSIZE);
            send(sock,buf,BUFSIZE,0);
        }

    }
}

void Client::run()
{
    //初始化套接字
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1)
        log(LogLevel::ERROR, "create error");
    else
        log(LogLevel::INFO, "create success");
    //1.1设置端口快速重用
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // //绑定ip和端口
    // struct sockaddr_in addr_cli;
    // memset(&addr_cli, 0, sizeof(addr_cli));
    // addr_cli.sin_family = AF_INET;
    // addr_cli.sin_port = htons(cli_port);
    // addr_cli.sin_addr.s_addr = inet_addr(cli_ip);

    // if(bind(sock, (struct sockaddr*)(&addr_cli), sizeof(addr_cli)) == -1)
    // {
    //     log(LogLevel::ERROR, "bind error");
    // }
    // else
    //     log(LogLevel::INFO, "bind success");

    //发送连接请求
    struct sockaddr_in addr_ser;
    memset(&addr_ser, 0, sizeof(addr_ser));
    addr_ser.sin_family = AF_INET;
    addr_ser.sin_port = htons(ser_port);
    addr_ser.sin_addr.s_addr = inet_addr(ser_ip);

    if(connect(sock, (struct sockaddr*)(&addr_ser), sizeof(addr_ser)) == -1)
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
    char buf[BUFSIZE];
    while(true)
    {
        bzero(buf,BUFSIZE);
        int res = recv(sock,buf,BUFSIZE,0);
        if(res == -1)
            log(LogLevel::ERROR,"recv error");
        else if(res == 0)
        {
            log(LogLevel::INFO,"服务器已下线");
            break;
        }
        else
        {
            msg m;
            m.deserialize(buf);
            m.print();
        }
    }   
}