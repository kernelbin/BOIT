//有关 CoolQ 的各种定义

#define CQAPIVER 9
#define CQAPIVERTEXT "9"

#define CQAPPID "com.kernelbin.boit" 
#define CQAPPINFO CQAPIVERTEXT "," CQAPPID

#ifndef CQAPI
#define CQAPI(ReturnType) __declspec(dllimport) ReturnType __stdcall
#endif

#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size))\
__declspec(dllexport) ReturnType __stdcall Name

#define FUNC(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size))\
__declspec(dllexport) ReturnType __stdcall Name



/*
* 发送私聊消息, 成功返回消息ID
* QQID 目标QQ号
* msg 消息内容
*/

CQAPI(int) CQ_sendPrivateMsg(int AuthCode, long long QQID, const char* msg);

/*
* 发送群消息, 成功返回消息ID
* groupid 群号
* msg 消息内容
*/
CQAPI(int) CQ_sendGroupMsg(int AuthCode, long long groupid, const char* msg);

/*
* 发送讨论组消息, 成功返回消息ID
* discussid 讨论组号
* msg 消息内容
*/
//CQAPI(int) CQ_sendDiscussMsg(int AuthCode, long long discussid, const char* msg);

/*
* 撤回消息
* msgid 消息ID
*/
//CQAPI(int) CQ_deleteMsg(int AuthCode, long long msgid);

/*
* 发送赞 发送手机赞
* QQID QQ号
*/
//CQAPI(int) CQ_sendLike(int AuthCode, long long QQID);

/*
* 置群员移除
* groupid 目标群
* QQID QQ号
* rejectaddrequest 不再接收此人加群申请，请慎用
*/
//CQAPI(int) CQ_setGroupKick(int AuthCode, long long groupid, long long QQID, int rejectaddrequest);

/*
* 置群员禁言
* groupid 目标群
* QQID QQ号
* duration 禁言的时间，单位为秒。如果要解禁，这里填写0。
*/
//CQAPI(int) CQ_setGroupBan(int AuthCode, long long groupid, long long QQID, long long duration);

/*
* 置群管理员
* groupid 目标群
* QQID QQ号
* setadmin true:设置管理员 false:取消管理员
*/
//CQAPI(int) CQ_setGroupAdmin(int AuthCode, long long groupid, long long QQID, int setadmin);

/*
* 置全群禁言
* groupid 目标群
* enableban true:开启 false:关闭
*/
//CQAPI(int) CQ_setGroupWholeBan(int AuthCode, long long groupid, int enableban);

/*
* 置匿名群员禁言
* groupid 目标群
* anomymous 群消息事件收到的 anomymous 参数
* duration 禁言的时间，单位为秒。不支持解禁。
*/
//CQAPI(int) CQ_setGroupAnonymousBan(int AuthCode, long long groupid, const char* anomymous, long long duration);

/*
* 置群匿名设置
* groupid 目标群
* enableanomymous true:开启 false:关闭
*/
//CQAPI(int) CQ_setGroupAnonymous(int AuthCode, long long groupid, int enableanomymous);

/*
* 置群成员名片
* groupid 目标群
* QQID 目标QQ
* newcard 新名片(昵称)
*/
//CQAPI(int) CQ_setGroupCard(int AuthCode, long long groupid, long long QQID, const char* newcard);

/*
* 置群退出 慎用, 此接口需要严格授权
* groupid 目标群
* isdismiss 是否解散 true:解散本群(群主) false:退出本群(管理、群成员)
*/
//CQAPI(int) CQ_setGroupLeave(int AuthCode, long long groupid, int isdismiss);

/*
* 置群成员专属头衔 需群主权限
* groupid 目标群
* QQID 目标QQ
* newspecialtitle 头衔（如果要删除，这里填空）
* duration 专属头衔有效期，单位为秒。如果永久有效，这里填写-1。
*/
//CQAPI(int) CQ_setGroupSpecialTitle(int AuthCode, long long groupid, long long QQID, const char* newspecialtitle, long long duration);

/*
* 置讨论组退出
* discussid 目标讨论组号
*/
//CQAPI(int) CQ_setDiscussLeave(int AuthCode, long long discussid);

/*
* 置好友添加请求
* responseflag 请求事件收到的 responseflag 参数
* responseoperation REQUEST_ALLOW 或 REQUEST_DENY
* remark 添加后的好友备注
*/
//CQAPI(int) CQ_setFriendAddRequest(int AuthCode, const char* responseflag, int responseoperation, const char* remark);

/*
* 置群添加请求
* responseflag 请求事件收到的 responseflag 参数
* requesttype根据请求事件的子类型区分 REQUEST_GROUPADD 或 REQUEST_GROUPINVITE
* responseoperation  REQUEST_ALLOW 或 REQUEST_DENY
* reason 操作理由，仅 REQUEST_GROUPADD 且 REQUEST_DENY 时可用
*/
//CQAPI(int) CQ_setGroupAddRequestV2(int AuthCode, const char* responseflag, int requesttype, int responseoperation, const char* reason);

/*
* 取群成员信息
* groupid 目标QQ所在群
* QQID 目标QQ号
* nocache 不使用缓存
*/
CQAPI(const char*) CQ_getGroupMemberInfoV2(int AuthCode, long long groupid, long long QQID, int nocache);

/*
* 取陌生人信息
* QQID 目标QQ
* nocache 不使用缓存
*/
CQAPI(const char*) CQ_getStrangerInfo(int AuthCode, long long QQID, int nocache);

/*
* 日志
* priority 优先级，CQLOG 开头的常量
* category 类型
* content 内容
*/
CQAPI(int) CQ_addLog(int AuthCode, int priority, const char* category, const char* content);

/*
* 取Cookies 慎用, 此接口需要严格授权
*/
CQAPI(const char*) CQ_getCookies(int AuthCode);

/*
* 取CsrfToken 慎用, 此接口需要严格授权
*/
//CQAPI(int) CQ_getCsrfToken(int AuthCode);

/*
* 取登录QQ
*/
//CQAPI(long long) CQ_getLoginQQ(int AuthCode);

/*
* 取登录QQ昵称
*/
//CQAPI(const char*) CQ_getLoginNick(int AuthCode);

/*
* 取应用目录，返回的路径末尾带"\"
*/
CQAPI(const char*) CQ_getAppDirectory(int AuthCode);

/*
* 置致命错误提示
* errorinfo 错误信息
*/
//CQAPI(int) CQ_setFatal(int AuthCode, const char* errorinfo);

/*
* 接收语音，接收消息中的语音(record),返回保存在 \data\record\ 目录下的文件名
* file 收到消息中的语音文件名(file)
* outformat 应用所需的语音文件格式，目前支持 mp3 amr wma m4a spx ogg wav flac
*/
//CQAPI(const char*) CQ_getRecord(int AuthCode, const char* file, const char* outformat);


#define EVENT_IGNORE 0          //事件_忽略
#define EVENT_BLOCK 1           //事件_拦截

#define REQUEST_ALLOW 1         //请求_通过
#define REQUEST_DENY 2          //请求_拒绝

#define REQUEST_GROUPADD 1      //请求_群添加
#define REQUEST_GROUPINVITE 2   //请求_群邀请

#define CQLOG_DEBUG 0           //调试 灰色
#define CQLOG_INFO 10           //信息 黑色
#define CQLOG_INFOSUCCESS 11    //信息(成功) 紫色
#define CQLOG_INFORECV 12       //信息(接收) 蓝色
#define CQLOG_INFOSEND 13       //信息(发送) 绿色
#define CQLOG_WARNING 20        //警告 橙色
#define CQLOG_ERROR 30          //错误 红色
#define CQLOG_FATAL 40          //致命错误 深红
