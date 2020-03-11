# BOIT
yh蒟蒻写的给OIers用的一个bot，OI Bot的第二版。

# features
**小!!!**  目前可执行文件和dll打包大小不到400KB
 
**快!!!**  多线程架构并发处理指令，run等高性能要求指令使用IO完成端口负载均衡技术

**功能强大**  内置沙盒，可以运行代码，支持输入数据等功能，内置查询OIer等功能，查询codeforces等功能

**易于部署**  程序本身不需要任何依赖（除CoolQ），可轻易部署

# Try it!
目前该bot部署在 QQ号 2068736261 上。可以加好友执行指令。（目前加好友审核是人工的，可能不会第一时间通过申请）
指令前缀是 '#' 用 "#help"指令查看帮助

# 部署
从 [Release](https://github.com/kernelbin/BOIT/releases) 页面下载最新的Release，包括.cpk文件（CoolQ插件）和与您计算机位数相匹配的可执行文件（x86或x64）

在CoolQ官网注册账号，并下载 CoolQ Pro/Air (CoolQ Pro为收费版本，不支持发送图片，语音等功能) 详见 [CoolQ社区](https://cqp.cc/)

将.cpk文件放入CoolQ插件目录 （位于Path_To_CoolQ\app\）

打开CoolQ，并登录您的CoolQ。（请右击CoolQ在任务栏中的图标，在菜单中选择 应用->应用管理 ，并确认 BOIT插件 已经启用）

启动从Release中下载的可执行文件（BOIT.Server.xxxx.exe）。第一次使用会需要输入 BOIT目录，（用于存储权限信息，配置信息，缓存编译文件等）

如果控制台显示 “连接成功！”，说明正常工作了。

# 注意！！！！
- 目前由于技术问题，同时运行多个CoolQ，或者同时运行多个可执行文件（BOIT.Server.xxxx.exe）可能会导致不正常工作。在一台计算机上只能打开一个CoolQ和一个可执行文件。

- 在一台计算机上第一次使用CoolQ登录某个账号，通常会出现消息发送不出去的情况。（CoolQ日志显示为发送，但实际没有收到消息）。这种情况下保持CoolQ在该IP地址多登录几次，等待数天即可解决。

- 目前run指令使用的内置沙盒并没有限制读取权限。为了防止隐私泄露，请避免在可能存放有您的个人信息的计算机上启用run指令（最简单的办法就是不去配置#run指令）

## 如何配置编译指令？（#run指令）
安装好你需要的编译器或解释器。（BOIT目前没有内置任何编译器或解释器）
找到BOIT目录，在 "BOIT\CommandCfg\run\Compiler\" 目录下新建后缀为.cfg的文件，如 "gcc.cfg"
下面以gcc为例子

```
# 允许行首以井号开头的注释

# 语言名称，允许以空格为分割存放多个。在#run /help的时候，只有第一个名称会显示
Name=c gcc

# 可以使用 Compile 或是 Script
Type=Compile

# 源代码文件后缀
Suffix=.c

# 编译或是运行指令
Command=gcc %In -o %Out

# 可选，源代码的编码格式，支持GB18030，UTF8和ANSI。默认GB18030
SourceEncode = GB18030

# 可选，标准输入输出编码格式，支持GB18030，UTF8和ANSI。默认GB18030
OutputEncode = GB18030

# 对于Type=Compile，Command为编译指令。
# 对于Type=Script，Command为脚本运行指令。
# 其中 %In会被扩展为源文件名，%Out会被扩展为可执行文件名称(.exe)，%% 为 % 本身
```


## contribute
- 向他人介绍该项目，推广BOIT。

- 部署BOIT并帮助进行测试，bug反馈等。

- 提供好的 idea，或者是feature request

- 参与BOIT的编码。（如果你熟悉Win32开发）

## todo
- 指令插件化

- 支持玩人力资源机

- 支持沙盒文件读写权限控制

- 支持洛谷查询

- 支持OI wiki查询
