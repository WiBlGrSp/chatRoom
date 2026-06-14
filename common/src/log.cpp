#include"common/log.h"
#include<stdio.h>
#include<ctime>
void log(LogLevel log_level,const char*fmt,...)
{
    char buf[4096];

    va_list args;
    va_start(args,fmt);
    vsnprintf(buf,sizeof(buf),fmt,args);

    time_t now = time(nullptr);

    tm* t = localtime(&now);

    char timebuf[64];

    strftime(
        timebuf,
        sizeof(timebuf),
        "%Y-%m-%d %H:%M:%S",
        t
    );
    
    switch(log_level)
    {
        case LogLevel::INFO:
            printf("[%s] [INFO]: %s\n",timebuf,buf);
            break;
        case LogLevel::WARN:
            printf("[%s] [WARN]: %s\n",timebuf,buf);
            break;
        case LogLevel::ERROR:
            char output[128];
            sprintf(output,"[%s] [ERROR]: %s",timebuf,buf);
            perror(output);
            break;
    }
    va_end(args);
}