#include"../include/protocol.h"
#include"../include/log.h"
#include <cstdint>
#include<cstring>
#include<arpa/inet.h>
#include <netinet/in.h>
#include<cstdio>
#include"../include/safe.h"
msg::msg(msg_type type,char*name,char*content):type(type)
{
    set_name(name);
    set_content(content);
}

void msg::set_type(msg_type type)
{
    this->type = type;
}
void msg::set_name(const char name[])
{
    if(name==nullptr){
        memset(this->name,0,kUserNameSize);
        return;
    }
    if(strlen(name)>=kUserNameSize)
        log(LogLevel::WARN,"user_name_length overflow");
    Safe::Copy(this->name, msg::kUserNameSize, name);
}
void msg::set_content(const char content[])
{
    if(content==nullptr){
        memset(this->content,0,kContentSize);
        return;
    }
    if(strlen(content)>=kContentSize)
        log(LogLevel::WARN,"content_length overflow");
    Safe::Copy(this->content,kContentSize,content);
}
void msg::serialize(char*str,int size)const{    //将消息序列化为二进制串
    if(size<kMsgLength)
    {
        log(LogLevel::WARN,"serialize: buf not enough");
    }
    memset(str,0,size);
    uint32_t n_type = htonl(static_cast<uint32_t>(type));
    memcpy(str,(char*)&n_type,kTypeSize);

    str+=kTypeSize;
    memcpy(str,this->name,kUserNameSize);

    str+=kUserNameSize;
    memcpy(str,this->content,kContentSize);

}
void msg::deserialize(char* str){    //将字符串反序列化为消息
    char* p = str;
    uint32_t n_type;
    memcpy((char*)&n_type,p,kTypeSize);
    type = static_cast<msg_type>(ntohl(n_type));

    p+=kTypeSize;
    memcpy(this->name, p,kUserNameSize);

    p+=kUserNameSize;
    memcpy(this->content,p,kContentSize);
}
//向终端打印消息
void msg::print()
{
    printf("[%s] [%s]:%s\n",type_string[(uint32_t)type],name,content);
}