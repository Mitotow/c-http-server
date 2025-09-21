#ifndef LOGGER_H
#define LOGGER_H

#define LOG_INFO "INFO"
#define LOG_TRACE "TRACE"
#define LOG_DEBUG "DEBUG"
#define LOG_WARN "WARN"
#define LOG_ERROR "ERROR"
#define LOG_FATAL "FATAL"
#define LOG_BUFFER_SIZE 1024

void writeLog(char *prefix, char *message, ...);
void writeFatal(char *message, ...);

#endif
