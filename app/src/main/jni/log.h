/*
 * =====================================================================================
 *
 *       Filename:  mlog.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014/2/12 13:28:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Jiang Lei (JL), monect@outlook.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef MLOG_H
#define MLOG_H

#define DEBUG 1
#if DEBUG && defined(ANDROID)
#include <android/log.h>
#  define  LOGD(x...)  __android_log_print(ANDROID_LOG_INFO,"nativelibjni",x)
#  define  LOGE(x...)  __android_log_print(ANDROID_LOG_ERROR,"nativelibjni",x)
#else
#  define  LOGD(...)  do {} while (0)
#  define  LOGE(...)  do {} while (0)
#endif

#endif
