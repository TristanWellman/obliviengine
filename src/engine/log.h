/*Modified by Tristan Wellman 2023*/

/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define LOG_VERSION "0.1.0"

typedef struct {
  va_list ap;
  const char *fmt;
  const char *file;
  struct tm *time;
  void *udata;
  int line;
  int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);
typedef void (*log_LockFn)(bool lock, void *udata);

enum { LOG_TRACE__, LOG_DEBUG__, LOG_INFO__, LOG_WARN__, LOG_ERROR__, LOG_FATAL__ };

#define log_trace(...) log_log(LOG_TRACE__, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG__, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO__,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN__,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR__, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL__, __FILE__, __LINE__, __VA_ARGS__)

#define wlog_trace(file, line_num, ...) \
    log_log(LOG_TRACE__, file, line_num, __VA_ARGS__)
#define wlog_debug(file, line_num, ...) \
    log_log(LOG_DEBUG__, file, line_num, __VA_ARGS__)
#define wlog_info(file, line_num, ...) \
    log_log(LOG_INFO__, file, line_num, __VA_ARGS__)
#define wlog_warn(file, line_num, ...) \
    log_log(LOG_WARN__, file, line_num, __VA_ARGS__)
#define wlog_error(file, line_num, ...) \
    log_log(LOG_ERROR__, file, line_num, __VA_ARGS__)
/*                                            "       |\n"
                                            "   %d|\t%s\n", line_num, line*/
#define wlog_fatal(file, line_num, ...) \
    log_log(LOG_FATAL, file, line_num, __VA_ARGS__)

const char* log_level_string(int level);
void log_set_lock(log_LockFn fn, void *udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void *udata, int level);
int log_add_fp(FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);

#endif
