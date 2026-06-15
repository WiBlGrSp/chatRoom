#ifndef _MESSAGE_TRANSPORTER_H_
#define _MESSAGE_TRANSPORTER_H_
#include "protocol.h"
#include <atomic>
#include<unordered_set>
#include<mutex>
#include<condition_variable>
#include<queue>
class MessageTransporter
{
private:
    int sock_;   //用于通信的socket
    static constexpr int kBufSize = sizeof(Message);   //缓冲区大小
    char rbuf_[kBufSize];    //读缓冲区
    char wbuf_[kBufSize];    //写缓冲区
    std::queue<Message> rmsg_queue_;   //读消息
    std::mutex wmutex_;     //保护写缓冲区的互斥锁
    std::mutex rmutex_;     //保护读缓冲区的互斥锁
    std::condition_variable recv_cond_; //用于分发消息的条件变量
    std::atomic<bool> start_listen_;  //是否监听

public:
    MessageTransporter(int sock=-1,bool start_listen = false);
    ~MessageTransporter();
    void setSock(int sock);
    int sendMessage(const Message& message);
    int recvMessage(Message& message,
        std::unordered_set<MsgType> include = std::unordered_set<MsgType>());
    void pushMessage(const Message&msg);
    void collectMessage();   //收集来自对端的消息,并进行分发
    void stopCollect();
};

#endif