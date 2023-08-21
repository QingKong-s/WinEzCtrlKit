# WinEzCtrlKit

## 构建
C++语言标准>=20

必须定义`NOMINMAX`宏，同时有以下宏开关：

`ECKMACRO_NO_WIN11_22621` Windows版本是否大于Build22621

`ECKMACRO_NO_AUTO_ADD_LIB` 是否自动添加#pragma comment(lib,"")

`ECKMACRO_NO_COMCTL60` 是否使用通用组件库6.0

其他内容查看eck\Env.h