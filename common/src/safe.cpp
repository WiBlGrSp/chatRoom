#include"common/safe.h"
#include<iostream>
namespace Safe {
int copy(char*dst,std::size_t dst_size,const char*str)
{
    if(str == nullptr || dst == nullptr)
        return -1;
    if(dst_size<=0)
        return -1;
    snprintf(dst,dst_size,"%s",str);
    return 0;
}
int input(char*dst,std::size_t dst_size)
{
    if(dst_size<=0)
        return -1;
    std::string line;
    std::getline(std::cin, line);
    int flag = 0;
    if(line.size()>=dst_size)
        flag = -1;
    snprintf(
        dst,
        dst_size,
        "%s",
        line.c_str()
    );
    return flag;
}
}