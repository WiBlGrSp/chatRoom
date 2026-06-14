#ifndef _MESSAGE_TRANSPORTER_H_
#define _MESSAGE_TRANSPORTER_H_
#include "../include/protocol.h"

class MessageTransporter
{
private:
    int sock_;   //用于通信的socket
    static const int kBufSize = sizeof(Message);   //缓冲区大小
    char rbuf_[kBufSize];    //读缓冲区
    char wbuf_[kBufSize];    //写缓冲区

public:
    MessageTransporter();
    MessageTransporter(int sock);
    ~MessageTransporter();
    void setSock(int sock);
    int sendMessage(const Message& message);
    int recvMessage(Message& message);
};

#endif