#include"../include/protocol.h"
#include"../include/log.h"
#include <cstdint>
#include<cstring>
#include<arpa/inet.h>
#include <netinet/in.h>
#include<cstdio>
msg::msg(msg_type type,char*name,char*content):type(type)
{
    if(name!=nullptr)
    {
        strncpy(this->name,name,USER_NAME_LENGTH);
    }else {
        memset(this->name,0,sizeof(this->name));
    }
    if(content !=nullptr)
    {
        strncpy(this->content,content,CONTENT_LENGTH);
    }else {
        memset(this->content,0,sizeof(this->content));
    }
}

void msg::set_type(msg_type type)
{
    this->type = type;
}
void msg::set_name(const char name[])
{
    strncpy(this->name,name,USER_NAME_LENGTH);
}
void msg::set_content(const char content[])
{
    strncpy(this->content,content,CONTENT_LENGTH);
}
void msg::serialize(char*str,int size)const{    //将消息序列化为字符串
    if(size<sizeof(msg))
    {
        log(LogLevel::WARN,"serialize: buf not enough");
    }
    memset(str,0,size);
    uint32_t n_type = htonl(static_cast<uint32_t>(type));
    memcpy(str,(char*)&n_type,TYPE_LENGTH);

    str+=TYPE_LENGTH;
    memcpy(str,this->name,USER_NAME_LENGTH);

    str+=USER_NAME_LENGTH;
    memcpy(str,this->content,CONTENT_LENGTH);

}
void msg::deserialize(char* str){    //将字符串反序列化为消息
    char* p = str;
    uint32_t n_type;
    memcpy((char*)&n_type,p,TYPE_LENGTH);
    type = static_cast<msg_type>(ntohl(n_type));

    p+=TYPE_LENGTH;
    memcpy(this->name, p,USER_NAME_LENGTH);

    p+=USER_NAME_LENGTH;
    memcpy(this->content,p,CONTENT_LENGTH);
}
//向终端打印消息
void msg::print()
{
    printf("[%s] [%s]:%s\n",type_string[(uint32_t)type],name,content);
}   