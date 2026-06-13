#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#include <cstdint>
#define CONTENT_LENGTH 512
#define USER_NAME_LENGTH 32
#define TYPE_LENGTH 4
enum class msg_type:uint32_t{
    LOGIN = 0,
    CHAT = 1,
    LOGOUT = 2
};
const char type_string[3][32]={"LOGIN","CHAT","LOGOUT"};
struct msg{
    msg_type type;  //消息类型
    char name[USER_NAME_LENGTH];  //用户名
    char content[CONTENT_LENGTH];  //消息内容
    msg(msg_type type=msg_type::CHAT,char*name=nullptr,char*content=nullptr);
    void set_type(msg_type type);
    void set_name(const char name[]);
    void set_content(const char content[]);
    void serialize(char*str,int size)const;    //将消息序列化为二进制串
    void deserialize(char* str);    //将二进制串反序列化为消息
    void print();   //向终端打印消息
};
#endif  //!_PROTOCOL_H_