#pragma once
#include<Windows.h>

//本来是Command Event，缩写该是CE。但是这个项目是给OIer写的，CE不太吉利所以全部换成EC（滑稽）


/* CoolQ正在初始化或者该指令首次加载，需要生成相应目录
ParamA: 未使用
ParamB: 未使用  */
#define EC_DIRINIT 0x01 

/* 指令加载，可以进行指令初始化工作
ParamA: 未使用
ParamB: 未使用  */
#define EC_CMDLOAD 0x02

/* 指令卸载，可以进行指令初清理工作
ParamA: 未使用
ParamB: 未使用  */
#define EC_CMDFREE 0x03