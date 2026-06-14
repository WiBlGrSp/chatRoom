#include "common/protocol.h"
#include "common/log.h"
#include <cstdint>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdio>
#include "common/safe.h"

Message::Message(MsgType type, char* name, char* content) : type_(type)
{
    setName(name);
    setContent(content);
}

void Message::setType(MsgType type)
{
    this->type_ = type;
}

void Message::setName(const char name[])
{
    if (name == nullptr) {
        memset(this->name_, 0, kUserNameSize);
        return;
    }
    if (strlen(name) >= kUserNameSize)
        log(LogLevel::WARN, "user_name_length overflow");
    Safe::copy(this->name_, Message::kUserNameSize, name);
}

void Message::setContent(const char content[])
{
    if (content == nullptr) {
        memset(this->content_, 0, kContentSize);
        return;
    }
    if (strlen(content) >= kContentSize)
        log(LogLevel::WARN, "content_length overflow");
    Safe::copy(this->content_, kContentSize, content);
}

void Message::serialize(char* str, int size) const {    //将消息序列化为二进制串
    if (size < kMsgLength)
    {
        log(LogLevel::WARN, "serialize: buf not enough");
    }
    memset(str, 0, size);
    uint32_t n_type = htonl(static_cast<uint32_t>(type_));
    memcpy(str, (char*)&n_type, kTypeSize);

    str += kTypeSize;
    memcpy(str, this->name_, kUserNameSize);

    str += kUserNameSize;
    memcpy(str, this->content_, kContentSize);
}

void Message::deserialize(char* str) {    //将字符串反序列化为消息
    char* p = str;
    uint32_t n_type;
    memcpy((char*)&n_type, p, kTypeSize);
    type_ = static_cast<MsgType>(ntohl(n_type));

    p += kTypeSize;
    memcpy(this->name_, p, kUserNameSize);

    p += kUserNameSize;
    memcpy(this->content_, p, kContentSize);
}

//向终端打印消息
void Message::print()
{
    printf("[%s] [%s]:%s\n", kTypeString[(uint32_t)type_], name_, content_);
}