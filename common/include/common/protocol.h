#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include <cstdint>
enum class MsgType : uint32_t {
    LOGIN = 0,
    CHAT = 1,
    LOGOUT = 2
};
const char kTypeString[3][32] = {"LOGIN", "CHAT", "LOGOUT"};
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
    void serialize(char* str, int size) const;    //将消息序列化为二进制串
    void deserialize(char* str);                //将二进制串反序列化为消息
    void print();   //向终端打印消息
};
static constexpr std::size_t kMsgLength = sizeof(Message);
#endif  //!_PROTOCOL_H_