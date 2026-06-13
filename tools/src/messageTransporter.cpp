#include"../include/messageTransporter.h"
#include<cstring>
#include <sys/socket.h>
messageTransporter::messageTransporter():m_sock(-1)
{
    
}
messageTransporter::messageTransporter(int sock):m_sock(sock)
{
}

messageTransporter::~messageTransporter()
{
}

void messageTransporter::set_sock(int sock)
{
    m_sock = sock;
}
int messageTransporter::SendMessage(const msg&message)
{
    bzero(wbuf,kBufSize);
    message.serialize(wbuf,kBufSize);
    int res = send(m_sock,wbuf,kBufSize,0);
    return res;
}

int messageTransporter::RecvMessage(msg&message)
{
    bzero(rbuf,kBufSize);
    int res = recv(m_sock, rbuf, kBufSize, 0);
    if(res == -1 || res == 0)
        return res;
    message.deserialize(rbuf);
    return res;
}