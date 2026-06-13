#ifndef _MESSAGE_TRANSPORTER_H_
#define _MESSAGE_TRANSPORTER_H_
#include"../include/protocol.h"

class messageTransporter
{
private:
    int m_sock;   //用于通信的socket
    static const int kBufSize = sizeof(msg);   //缓冲区大小
    char rbuf[kBufSize];    //读缓冲区
    char wbuf[kBufSize];    //写缓冲区

public:
    messageTransporter();
    messageTransporter(int sock);
    ~messageTransporter();
    void set_sock(int sock);
    int SendMessage(const msg&message);
    int RecvMessage(msg&message);
};

#endif