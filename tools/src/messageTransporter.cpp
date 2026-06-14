#include "../include/messageTransporter.h"
#include <cstring>
#include <sys/socket.h>

MessageTransporter::MessageTransporter() : sock_(-1)
{
    
}

MessageTransporter::MessageTransporter(int sock) : sock_(sock)
{
}

MessageTransporter::~MessageTransporter()
{
}

void MessageTransporter::setSock(int sock)
{
    sock_ = sock;
}

int MessageTransporter::sendMessage(const Message& message)
{
    bzero(wbuf_, kBufSize);
    message.serialize(wbuf_, kBufSize);
    int res = send(sock_, wbuf_, kBufSize, 0);
    return res;
}

int MessageTransporter::recvMessage(Message& message)
{
    bzero(rbuf_, kBufSize);
    int res = recv(sock_, rbuf_, kBufSize, 0);
    if (res == -1 || res == 0)
        return res;
    message.deserialize(rbuf_);
    return res;
}