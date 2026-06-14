#include "server.h"
#include "common/protocol.h"
#include "common/log.h"
#include "common/messageTransporter.h"
#include <cmath>
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
            if(res == -1) log(LogLevel::ERROR, "broadcast error");
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

void Server::delUser(int sock)
{
    auto it = user_list_.begin();
    for(; it != user_list_.end() && it->sock_ != sock; it++);
    if(it != user_list_.end())
        user_list_.erase(it);
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
    struct Message m;
    while(true)
    {
        int res = msg_trans.recvMessage(m);
        if(res == 0)
        {
            {
                unique_lock<mutex> lock(mutex_user_list_);
                const char* name = getName(sock);
                log(LogLevel::INFO, "[%s:%d]:%s已经下线",
                    inet_ntoa(addr_cli.sin_addr), ntohs(addr_cli.sin_port), name);
                //向所有在线用户广播用户退出消息
                m.setType(MsgType::LOGOUT);
                m.setName(name);
                m.setContent("logout");
                broadcast(m);
                delUser(sock);
            }
            break;
        }else if(res == -1)
        {
            log(LogLevel::ERROR, "recv error");
        }else{
            //分类处理
            switch(m.type_)
            {
                case MsgType::LOGIN:
                    {
                        unique_lock<mutex> lock(mutex_user_list_);
            
                        //DEBUG:重复添加相同用户
                        //向用户列表添加该用户
                        if(addUser(addr_cli, sock, m.name_) == -1)
                        {
                            m.setContent("login error:id重复");
                            msg_trans.sendMessage(m);
                            break;
                        }
                        //检查用户列表
                        log(LogLevel::INFO, "user_list size:%zu", user_list_.size());

                        //向所有在线用户广播登录成功消息
                        m.setContent("login");
                        broadcast(m);
                    }
                break;
                case MsgType::CHAT:
                    {
                        unique_lock<mutex> lock(mutex_user_list_);
                        //向除自己外的在线用户广播
                        broadcast(m, sock);
                    }
                break;
                case MsgType::LOGOUT:
                    {
                        unique_lock<mutex> lock(mutex_user_list_);
                        //从用户列表删除该用户
                        delUser(sock);
                        //向所有在线用户广播用户退出消息
                        m.setContent("logout");
                        broadcast(m);
                    }
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