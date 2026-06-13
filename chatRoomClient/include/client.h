#ifndef _CLIENT_H_
#define _CLIENT_H_
#include"../include/protocol.h"
#define BUFSIZE 1024
/*网络聊天室,客户端类
功能:
    主线程:初始化客户端,发起连接请求,接收服务端消息
    分支线程:发送消息到服务端
*/
enum class cliType{DEFAULT,LOGIN,LOGOUT};
class Client
{
private:
    int sock;           //用于通信的套接字
    const char* cli_ip; //客户端IP地址
    int cli_port;       //客户端端口号
    const char* ser_ip; //服务器IP地址
    int ser_port;       //服务器端口号
    cliType status;         //客户端状态
    char name[USER_NAME_LENGTH];

    void login();
    void logout();

    void messageHandler();  
    void run();
public:
    Client(const char* ip, int port, const char* server_ip, int server_port);
    ~Client();    
};

#endif