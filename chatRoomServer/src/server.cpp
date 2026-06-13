#include"server.h"
#include"protocol.h"
#include"log.h"
#include"messageTransporter.h"
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
Server::Server(const char* ip, int port, int numThreads):thread_pool(numThreads)
{
    run(ip,port);
}

Server::~Server()
{
    
}
void Server::broadcast(msg& m,int exclude_sock)
{
    messageTransporter msg_trans;
    for(const auto&user:user_list)
    {
        if(user.sock!=exclude_sock)
        {
            msg_trans.set_sock(user.sock);
            int res = msg_trans.SendMessage(m);
            if(res==-1) log(LogLevel::ERROR,"broadcast error");
        }
    }
    
}
int Server::add_user(struct sockaddr_in&addr_user,int sock,char user_id[])
{
    //检查是否有重复
    for(const auto&user:user_list)
    {
        if(strcmp(user_id,user.user_id)==0)
            return -1;
    }
    struct userInfo user_info;
    user_info.addr = addr_user;
    user_info.sock = sock;
    strcpy(user_info.user_id,user_id);

    user_list.push_back(user_info);
    return 0;
}
void Server::del_user(int sock)
{
    auto it = user_list.begin();
    for(;it!=user_list.end() && it->sock != sock;it++);
    if(it !=user_list.end())
        user_list.erase(it);
}
const char* Server::get_name(int sock)
{
    for(const auto&user:user_list)
    {
        if(user.sock == sock)
            return user.user_id;
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
void Server::messageHandler(int sock,struct sockaddr_in addr_cli)
{
 
    messageTransporter msg_trans(sock);
    struct msg m;
    while(true)
    {
        int res = msg_trans.RecvMessage(m);
        if(res == 0)
        {
            {
                unique_lock<mutex>lock(mutex_user_list);
                const char *name = get_name(sock);
                log(LogLevel::INFO,
                    "[%s:%d]:%s已经下线",
                    inet_ntoa(addr_cli.sin_addr),ntohs(addr_cli.sin_port),name);
                //向所有在线用户广播用户退出消息
                m.set_type(msg_type::LOGOUT);
                m.set_name(name);
                m.set_content("logout");
                broadcast(m);
                del_user(sock);
            }
            break;
        }else if(res == -1)
        {
            log(LogLevel::ERROR,"recv error");
        }else{
            //分类处理
            switch(m.type)
            {
                case msg_type::LOGIN:
                    {
                        unique_lock<mutex>lock(mutex_user_list);
            
                        //DEBUG:重复添加相同用户
                        //向用户列表添加该用户
                        if(add_user(addr_cli,sock,m.name) == -1)
                        {
                            m.set_content("login error:id重复");
                            msg_trans.SendMessage(m);
                            break;
                        }
                        //检查用户列表
                        log(LogLevel::INFO,"user_list size:%zu",user_list.size());

                        //向所有在线用户广播登录成功消息
                        m.set_content("login");
                        broadcast(m);
                    }
                break;
                case msg_type::CHAT:
                    {
                        unique_lock<mutex>lock(mutex_user_list);
                        //向除自己外的在线用户广播
                        broadcast(m,sock);
                    }
                break;
                case msg_type::LOGOUT:
                    {
                        unique_lock<mutex>lock(mutex_user_list);
                        //从用户列表删除该用户
                        del_user(sock);
                        //向所有在线用户广播用户退出消息
                        m.set_content("logout");
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
void Server::run(const char*ip,int port)
{
    //1.创建用于监听的socket
    int sock_lis = socket(AF_INET,SOCK_STREAM,0);

    if(sock_lis == -1)
        log(LogLevel::ERROR, "create error");
    else
        log(LogLevel::INFO, "create success");

     //1.1设置端口快速重用
    int opt = 1;
    setsockopt(sock_lis, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //2.绑定本机ip和端口
    struct sockaddr_in addr_lis;
    memset(&addr_lis,0,sizeof(addr_lis));
    addr_lis.sin_family=AF_INET;
    addr_lis.sin_port = htons(port);
    addr_lis.sin_addr.s_addr = inet_addr(ip);

    if(bind(sock_lis,(struct sockaddr*)(&addr_lis),sizeof(addr_lis)) == -1)
    {
        log(LogLevel::ERROR, "bind error");
    }else
        log(LogLevel::INFO, "bind success");

    //3.设置监听状态
    if(listen(sock_lis,BACKLOG) == -1)
    {
        log(LogLevel::ERROR, "listen error");
    }else
        log(LogLevel::INFO, "listen success");

    while(true){
        //4.等待客户端连接
        struct sockaddr_in addr_cli;
        socklen_t addr_len = sizeof(addr_cli);
        int sock_con = accept(sock_lis,(struct sockaddr*)&addr_cli,&addr_len);
        if(sock_con < 0){
            log(LogLevel::ERROR,"accept error");
            continue;
        }else
            log(LogLevel::INFO,"accept success");
        log(LogLevel::INFO,
            "[%s:%d]连接成功",
            inet_ntoa(addr_cli.sin_addr),ntohs(addr_cli.sin_port));
        //将通信任务分发给线程池
        thread_pool.addTask([this,sock_con,addr_cli]{
            messageHandler(sock_con,addr_cli);
        });
    }
}