#include<Windows.h>

//版本定义
#define VERSION_MAJOR 2
#define VERSION_MINOR 0

//返回值定义
#define SETTINGS_LOADED 0			//找到设置并加载成功
#define SETTINGS_SAVED 1
#define SETTINGS_CLEARED 2
#define SETTINGS_INITIALIZED 3

#define SETTINGS_NOT_FOUND 4			//设置注册表项（根）没找到 应该是第一次使用这个软件
#define SETTINGS_OUT_OF_DATE 5				//设置是老版本的
#define SETTINGS_ERROR 6			//出现错误
#define SETTINGS_NULL_BUFFER 7       //缓冲区是NULL