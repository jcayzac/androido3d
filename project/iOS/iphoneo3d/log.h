#ifndef IPHONE_LOG_H_
#define IPHONE_LOG_H_

#define LOGLEVEL_ERROR  1
#define LOGLEVEL_WARN       3
#define LOGLEVEL_INFO       5
//#define LOGLEVEL_DEFAULT  LOGLEVEL_WARN
#define LOGLEVEL_DEFAULT    LOGLEVEL_INFO

/**
 * LogError() and LogWarn() are enabled by default.
 * To change log level to info, set MAXLOGLEVEL as pre-processor option
 * (ex. -DMAXLOGLEVEL=LOGLEVEL_INFO),
 * or uncomment the line below.
 */
//#define MAXLOGLEVEL   LOGLEVEL_INFO

#ifndef MAXLOGLEVEL
#   define MAXLOGLEVEL  LOGLEVEL_DEFAULT
#endif

#ifndef __OBJC__
#   include <stdio.h>
#endif

#ifdef LOG_MESSAGES
#   ifdef __OBJC__
#       define O3DLogDebug(fmt, ...) NSLog((@fmt), ##__VA_ARGS__)
#   else
#       define O3DLogDebug(fmt, ...) printf((fmt), ##__VA_ARGS__)
#   endif
#else
#   define O3DLogDebug(...) do {} while (0)
#endif

#if MAXLOGLEVEL >= LOGLEVEL_ERROR
#   define LOGE(fmt, ...) O3DLogDebug(fmt, ##__VA_ARGS__)
#else
#   define LOGE(...) do {} while (0)
#endif

#if MAXLOGLEVEL >= LOGLEVEL_WARN
#   define LOGW(fmt, ...) O3DLogDebug(fmt, ##__VA_ARGS__)
#else
#   define LOGW(...) do {} while (0)
#endif

#if MAXLOGLEVEL >= LOGLEVEL_INFO
#   define LOGI(fmt, ...) O3DLogDebug(fmt, ##__VA_ARGS__)
#else
#   define LOGI(...) do {} while (0)
#endif

#endif // IPHONE_LOG_H_

