#include "common/messageTransporter.h"
#include "common/log.h"
#include "common/protocol.h"
#include <cstring>
#include <mutex>
#include <sys/socket.h>
#include <unordered_set>

MessageTransporter::MessageTransporter(int sock,bool start_listen) : sock_(sock),start_listen_(start_listen)
{
}

MessageTransporter::~MessageTransporter()
{
    stopCollect();
}

void MessageTransporter::setSock(int sock)
{
    sock_ = sock;
}

int MessageTransporter::sendMessage(const Message& message)
{
    std::unique_lock<std::mutex> lock(wmutex_);
    int res = -1;
    bzero(wbuf_, kBufSize);
    message.serialize(wbuf_, kBufSize);
    res = send(sock_, wbuf_, kBufSize, 0);
    if(res == -1)
        log(LogLevel::ERROR,"send error");
    else{ 
        log(LogLevel::INFO,"sendMessage:%s",message.str().c_str());
    }
    return res;
}

int MessageTransporter::recvMessage(Message& message,std::unordered_set<MsgType> include)
{
    int res = 1;
    if(start_listen_ == false){ 
        //未开启监听,接收对端
        std::unique_lock<std::mutex>lock(rmutex_);
        bzero(rbuf_, kBufSize);
        res = recv(sock_, rbuf_, kBufSize, 0);
        if(res == -1)
            log(LogLevel::ERROR,"recv error");
        else if(res == 0)
            log(LogLevel::ERROR,"对端已经下线");
        else
            message.deserialize(rbuf_);
    }else 
    {
        std::unique_lock<std::mutex>lock(rmutex_);
        if(!include.empty())
            //开启监听,接收指定类型的分发
            recv_cond_.wait(lock,[this,&include]{
                return include.find(rmsg_queue_.front().type_)!=include.end() || start_listen_ == false;
            });
        else
            //接收任何类型的分发
            recv_cond_.wait(lock,[this]{
                return start_listen_ == false;
            });
        message = rmsg_queue_.front();
        rmsg_queue_.pop();
    }
    log(LogLevel::INFO,"recvMessage:%s",message.str().c_str());
    return res;
}
void MessageTransporter::pushMessage(const Message&msg)
{
    std::unique_lock<std::mutex>lock(rmutex_);
    rmsg_queue_.push(msg);
    recv_cond_.notify_all();
}
void MessageTransporter::collectMessage
()
{
    while(start_listen_)
    {
        int res;
        // std::unique_lock<std::mutex>lock(rmutex_);
        bzero(rbuf_, kBufSize);
        res = recv(sock_, rbuf_, kBufSize, 0);
        if(res > 0)
        {
            std::unique_lock<std::mutex>lock(rmutex_);   
            Message msg;
            msg.deserialize(rbuf_);
            rmsg_queue_.push(msg);
            recv_cond_.notify_all();   
            log(LogLevel::INFO,"collectMessage:%s",msg.str().c_str());
        }else if(res == 0){
            log(LogLevel::ERROR,"对端已经下线");
            break;
        }
        else if(res == -1)
            log(LogLevel::ERROR,"recv error");   
    }
}
void MessageTransporter::stopCollect()
{
    start_listen_ = false;
    recv_cond_.notify_all();
}