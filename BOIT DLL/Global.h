#pragma once
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include"CoolQDef.h"

//日志函数
#define _STR(x) #x
#define ADD_LOG(a, b, c) _ADD_LOG(a, b, c, __LINE__, __FUNCTION__)
#define _ADD_LOG(a, b, c, d, e) __ADD_LOG(a, b, c, d, e)
#define __ADD_LOG(a,b,c,d, e) AddLog(a, b, c "  详细信息: 在文件 " __FILE__ " 函数 " e " 第 " #d " 行")
int AddLog(int priority, const char* category, const char* content);

#define LOG_DEBUG       CQLOG_DEBUG           //调试 灰色
#define LOG_INFO        CQLOG_INFO            //信息 黑色
#define LOG_INFOSUCCESS CQLOG_INFOSUCCESS     //信息(成功) 紫色
#define LOG_INFORECV    CQLOG_INFORECV        //信息(接收) 蓝色
#define LOG_INFOSEND    CQLOG_INFOSEND        //信息(发送) 绿色
#define LOG_WARNING     CQLOG_WARNING         //警告 橙色
#define LOG_ERROR       CQLOG_ERROR           //错误 红色
#define LOG_FATAL       CQLOG_FATAL           //致命错误 深红



//AppLifeRoutine 函数
int AppInitialize();
int AppFinialize();
int AppEnabled();
int AppDisabled();


int HandlePrivateMessage(int subType, int msgId, long long fromQQ, const char* msg, int font);
int HandleGroupMessage(int subType, int msgId, long long fromGroup, long long fromQQ, const char* fromAnonymous, const char* msg, int font);