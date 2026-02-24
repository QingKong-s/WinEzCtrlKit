# WinEzCtrlKit

[English](README-en.md)

一个Win32常用功能集合库。

* 使用C++20
* 维护所有窗口的列表，提供了诸多自动化设施完成高DPI感知和深/浅色模式切换等
* 提供线性、流式、表格、帧四种布局
* 在`CWindow::Create`和`CLayoutBase`之上提供了一个层，允许以声明式语法创建控件
* 通过`Observable<>`和`DdxBind`前缀函数提供简单双向数据绑定支持
  ```C++
  Observable<CStringW> obString{};
  eck::DdxBindEdit(m_Edit, *this/* parent */, obString);
  ```
* `OnMessage`代替窗口过程，除此以外，提供以下机制调整消息处理
  * 消息循环过滤：为`IsDialogMessageW`、翻译加速器、按键过滤等提供支持
  * 通知自处理：提供`CWindow::OnNotifyMessage`接收窗口自身产生的所有通知
  * 窗口消息过滤：`CWindow::GetEventChain`返回一个事件链，该对象保存一系列回调，产生消息时，在`OnMessage`之前按顺序调用链上的每个回调，回调可拦截某消息阻止其向后传递
* 提供大部分标准控件的完全封装，并进行必要的改装以适配暗色模式
* 不分拣任何特定消息（如按钮被点击、窗口大小改变），保留传统消息处理过程，大部分操作都使用原始WinApi进行

## 第三方代码

**[Detours](https://github.com/microsoft/Detours)**

MIT License

**[phnt](https://github.com/winsiderss/phnt)**

MIT License

**[zlib](https://www.zlib.net/)**

zlib License

**[yyjson](https://github.com/ibireme/yyjson)**

MIT License

**[win32-custom-menubar-aero-theme](https://github.com/adzm/win32-custom-menubar-aero-theme)**

MIT License

**[findContours](https://github.com/Alexbeast-CN/findContours)**

Apache License V2

**[pugixml](https://github.com/zeux/pugixml)**

MIT License

**[uchardet](https://www.freedesktop.org/wiki/Software/uchardet/)**

MPL V1.1

**[skia](https://skia.org/)**

BSD-3-Clause License