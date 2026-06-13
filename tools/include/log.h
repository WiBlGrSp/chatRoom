#ifndef _LOG_H_
#define _LOG_H_
#include<cstdarg>
enum class LogLevel{INFO,WARN,ERROR};
void log(LogLevel log_level,const char*fmt,...);

#endif