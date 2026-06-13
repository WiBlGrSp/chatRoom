#ifndef _SERVER_H_
#define _SERVER_H_

//在线多人网络聊天室-服务器端
#include"../include/threadPool.h"
#include"../include/protocol.h"
#include <cstdio>
#include <cstring>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<mutex>
#define BACKLOG 128
#define BUFSIZE 1024
#define USER_ID_LENGTH 32

using namespace std;

class Server
{
private:
    struct userInfo{
        int sock;   //和用户通信的套接字
        struct sockaddr_in addr;    //用户的地址信息
        char user_id[USER_ID_LENGTH];   //用户id
    };
private:

    threadPool thread_pool; //线程池:用于执行通信任务

    mutex mutex_user_list;  //用于保护用户列表的互斥锁
    vector<userInfo> user_list; //用户列表:用于存储所有登录的用户信息
    int add_user(struct sockaddr_in&addr_user,int sock,char user_id[]);//成功返回0,失败返回-1
    void del_user(int sock);

    //广播用户消息,exclude_sock表示被排除的用户,-1表示向所有在线用户广播消息
    void broadcast(msg &m,int exclude_sock=-1);   
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
    void messageHandler(int sock,struct sockaddr_in addr_cli);  


/*
    启动服务器:connector连接模块
    功能:
        初始化监听模块:创建socket-->绑定地址-->设置监听状态
        接收客户端的连接请求
        创建连接
        将通信任务分发给线程池
*/
    void run(const char*ip,int port); 
public:
/*
    初始化服务器
    功能:
        初始化线程池
        初始化用户管理
        启动服务器
    
*/
    Server(const char*ip,int port,int numThreads);   
    ~Server();
};

#endif  //!_SERVER_H_