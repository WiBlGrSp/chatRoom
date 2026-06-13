#ifndef _LOG_H_
#define _LOG_H_

enum class LogLevel{INFO,WARN,ERROR};
void log(LogLevel log_level,const char*msg);

#endif