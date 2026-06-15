#include "server.h"
#include "common/protocol.h"
#include "common/log.h"
#include "common/messageTransporter.h"
#include <cmath>
#include <cstdio>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>

/*
    初始化服务器
    功能:
        初始化线程池
        初始化用户管理器
        启动服务器
    
*/
Server::Server(const char* ip, int port, int numThreads) : thread_pool_(numThreads)
{
    run(ip, port);
}

Server::~Server()
{
    
}

void Server::broadcast(Message& m, int exclude_sock)
{
    MessageTransporter msg_trans;
    for(const auto& user : user_list_)
    {
        if(user.sock_ != exclude_sock)
        {
            msg_trans.setSock(user.sock_);
            int res = msg_trans.sendMessage(m);
            if(res == -1) 
                log(LogLevel::ERROR, "broadcast error");
            else    
                log(LogLevel::INFO,"broadcast ok");
        }
    }
    
}

int Server::addUser(struct sockaddr_in& addr_user, int sock, char user_id[])
{
    //检查是否有重复
    for(const auto& user : user_list_)
    {
        if(strcmp(user_id, user.user_id_) == 0)
            return -1;
    }
    struct UserInfo user_info;
    user_info.addr_ = addr_user;
    user_info.sock_ = sock;
    strcpy(user_info.user_id_, user_id);

    user_list_.push_back(user_info);
    return 0;
}

int Server::delUser(int sock)
{
    auto it = user_list_.begin();
    for(; it != user_list_.end() && it->sock_ != sock; it++);
    if(it != user_list_.end()){
        user_list_.erase(it);
        return 0;
    }
    return -1;
}

const char* Server::getName(int sock)
{
    for(const auto& user : user_list_)
    {
        if(user.sock_ == sock)
            return user.user_id_;
    }
    return nullptr;
}

void Server::loginHandler(int sock, struct sockaddr_in addr_cli,Message&msg)
{
    
    unique_lock<mutex> lock(mutex_user_list_);
    MessageTransporter msg_trans(sock);
    //DEBUG:重复添加相同用户
    //向用户列表添加该用户
    if(addUser(addr_cli, sock, msg.name_) == -1)
    {
        msg_trans.sendMessage(Message(MsgType::LOGIN_FAIL));
        return;
    }
    //检查用户列表
    log(LogLevel::INFO, "user_list size:%zu", user_list_.size());
    for(const auto&user:user_list_)
    {
        printf("%s ",user.user_id_);
    }
    printf("\n");

    msg.setType(MsgType::LOGIN_OK);
    msg.setContent("login");
    msg_trans.sendMessage(msg);
    //向所有在线用户广播登录成功消息
    msg.setType(MsgType::INFO);
    broadcast(msg,sock);
}

void Server::logoutHandler(int sock,Message&msg)
{
    unique_lock<mutex> lock(mutex_user_list_);
    //从用户列表删除该用户
    int res = delUser(sock);
    if(res == -1)
    {
        msg.setType(MsgType::LOGOUT_FAIL);
        MessageTransporter(sock).sendMessage(msg);
    }else if(res == 0){
        msg.setType(MsgType::LOGOUT_OK);
        MessageTransporter(sock).sendMessage(msg);
        //向所有在线用户广播用户退出消息
        msg.setContent("logout");
        broadcast(msg);
    }
}
int Server::exitHangdler(MessageTransporter&msg_trans)
{
    //向用户发送响应
    msg_trans.sendMessage(Message(MsgType::EXIT_OK));
    return 0;
}
/*
    用户消息处理
    功能:
        接收消息
        解析消息
        根据消息类型分类处理:
            LOGIN -->将用户添加到用户管理器,广播登录成功消息
            CHAT -->广播消息
            EXIT -->将用户从用户管理器中删除
*/
void Server::messageHandler(int sock, struct sockaddr_in addr_cli)
{
 
    MessageTransporter msg_trans(sock);
    Message msg;
    bool running = true;
    while(running)
    {
        int res = msg_trans.recvMessage(msg);
        if(res == 0)
        {
            {
                unique_lock<mutex> lock(mutex_user_list_);
                const char* name = getName(sock);
                log(LogLevel::INFO, "[%s:%d]:%s已经下线",
                    inet_ntoa(addr_cli.sin_addr), ntohs(addr_cli.sin_port), name);
                //向所有在线用户广播用户退出消息
                msg.setType(MsgType::LOGOUT);
                msg.setName(name);
                msg.setContent("logout");
                broadcast(msg);
                delUser(sock);
            }
            break;
        }else if(res == -1)
        {
            log(LogLevel::ERROR, "recv error");
        }else{
            //分类处理
            switch(msg.type_)
            {
                case MsgType::LOGIN:
                    loginHandler(sock,addr_cli,msg);
                break;
                case MsgType::CHAT:
                    {
                        unique_lock<mutex> lock(mutex_user_list_);
                        //向除自己外的在线用户广播
                        broadcast(msg, sock);
                    }
                break;
                case MsgType::LOGOUT:
                    logoutHandler(sock,msg);
                    break;
                case MsgType::EXIT:
                    if(exitHangdler(msg_trans)==0)
                        running = false;
                break;
                default:
                break;
            }
        }
    }
    close(sock);
}

/*
    启动服务器:connector连接模块
    功能:
        接收客户端的连接请求
        创建连接
        将通信任务分发给线程池
*/
void Server::run(const char* ip, int port)
{
    //1.创建用于监听的socket
    int sock_lis = socket(AF_INET, SOCK_STREAM, 0);

    if(sock_lis == -1)
        log(LogLevel::ERROR, "create error");
    else
        log(LogLevel::INFO, "create success");

    //1.1设置端口快速重用
    int opt = 1;
    setsockopt(sock_lis, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //2.绑定本机ip和端口
    struct sockaddr_in addr_lis;
    memset(&addr_lis, 0, sizeof(addr_lis));
    addr_lis.sin_family = AF_INET;
    addr_lis.sin_port = htons(port);
    addr_lis.sin_addr.s_addr = inet_addr(ip);

    if(bind(sock_lis, (struct sockaddr*)(&addr_lis), sizeof(addr_lis)) == -1)
    {
        log(LogLevel::ERROR, "bind error");
    } else
        log(LogLevel::INFO, "bind success");

    //3.设置监听状态
    if(listen(sock_lis, BACKLOG) == -1)
    {
        log(LogLevel::ERROR, "listen error");
    } else
        log(LogLevel::INFO, "listen success");

    while(true){
        //4.等待客户端连接
        struct sockaddr_in addr_cli;
        socklen_t addr_len = sizeof(addr_cli);
        int sock_con = accept(sock_lis, (struct sockaddr*)&addr_cli, &addr_len);
        if(sock_con < 0){
            log(LogLevel::ERROR, "accept error");
            continue;
        } else
            log(LogLevel::INFO, "accept success");
        log(LogLevel::INFO, "[%s:%d]:连接成功",
            inet_ntoa(addr_cli.sin_addr), ntohs(addr_cli.sin_port));
        //将通信任务分发给线程池
        thread_pool_.addTask([this, sock_con, addr_cli]{
            messageHandler(sock_con, addr_cli);
        });
    }
}