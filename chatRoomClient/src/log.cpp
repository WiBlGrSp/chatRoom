#include"../include/log.h"
#include<stdio.h>
void log(LogLevel log_level,const char*fmt,...)
{
    char buf[4096];

    va_list args;
    va_start(args,fmt);
    vsnprintf(buf,sizeof(buf),fmt,args);
    
    switch(log_level)
    {
        case LogLevel::INFO:
            printf("[INFO]: %s\n",buf);
            break;
        case LogLevel::WARN:
            printf("[WARN]: %s\n",buf);
            break;
        case LogLevel::ERROR:
            char output[128];
            sprintf(output,"[ERROR]: %s",buf);
            perror(output);
            break;
    }

    va_end(args);
}