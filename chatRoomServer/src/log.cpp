#include"../include/log.h"
#include<stdio.h>
void log(LogLevel log_level,const char*msg)
{

    switch(log_level)
    {
        case LogLevel::INFO:
            printf("[INFO]: %s\n",msg);
            break;
        case LogLevel::WARN:
            printf("[WARN]: %s\n",msg);
            break;
        case LogLevel::ERROR:
            char output[128];
            sprintf(output,"[ERROR]: %s",msg);
            perror(output);
            break;
    }
}