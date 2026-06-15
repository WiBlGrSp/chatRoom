#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include <cstdint>
#include<string>
enum class MsgType : uint32_t {
    LOGIN = 1,
    CHAT = 2,
    LOGOUT = 3,

    LOGIN_OK = 100,
    LOGIN_FAIL = 101,

    CHAT_OK = 200,
    CHAT_FAIL = 201,

    LOGOUT_OK = 300,
    LOGOUT_FAIL = 301,

    EXIT = 4,
    EXIT_OK = 400,

    INFO = 500,
    QUIT = 999
};
// const char kTypeString[3][32] = {"LOGIN", "CHAT", "LOGOUT"};
struct Message {
    static constexpr std::size_t kTypeSize = 4;
    static constexpr std::size_t kUserNameSize = 8;
    static constexpr std::size_t kContentSize = 512;
    MsgType type_;  //消息类型
    char name_[kUserNameSize];  //用户名
    char content_[kContentSize];  //消息内容
    Message(MsgType type = MsgType::CHAT, char* name = nullptr, char* content = nullptr);
    void setType(MsgType type);
    void setName(const char name[]);
    void setContent(const char content[]);
    void serialize(char* str,size_t size) const;    //将消息序列化为二进制串
    void deserialize(char* str);                //将二进制串反序列化为消息
    void print() const;   //向终端打印消息
    std::string str()const;   //将协议信息转化为字符串
};
static constexpr std::size_t kMsgLength = sizeof(Message);
#endif  //!_PROTOCOL_H_