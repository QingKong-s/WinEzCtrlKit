﻿/*
* WinEzCtrlKit Library
*
* Env.h ： 外部环境
*
* Copyright(C) 2023-2024 QingKong
*/
#pragma once
/*
* 若要使用DPI感知，添加清单文件".\eck\Others\DpiAwarePreMonV2.manifest"，
* 仅使用'系统DPI感知'和'每显示器DPI感知V2'，不使用'每显示器DPI感知V1'，代码也
* 未进行适配
*/

#ifndef ECKMACRO_NO_COMCTL60
//#pragma comment(linker,"\"/manifestdependency:						\
//		type='win32'								\
//		name='Microsoft.Windows.Common-Controls'	\
//		version='6.0.0.0'							\
//		processorArchitecture='*'					\
//		publicKeyToken='6595b64144ccf1df'			\
//		language='*'								\
//	\"")
#endif

#ifndef ECKMACRO_NO_AUTO_ADD_LIB
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Imm32.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Advpack.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Ncrypt.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")
#pragma comment(lib, "Crypt32.lib")

#ifdef _WIN64
#	ifdef _DEBUG
#		pragma comment(lib,"eck\\Detours\\detours_x64d.lib")
#	else
#		pragma comment(lib,"eck\\Detours\\detours_x64.lib")
#	endif
#else
#	ifdef _DEBUG
#		pragma comment(lib,"eck\\Detours\\detours_x86d.lib")
#	else
#		pragma comment(lib,"eck\\Detours\\detours_x86.lib")
#	endif
#endif

#ifdef _WIN64
#	ifdef _DEBUG
#		pragma comment(lib,"eck\\ZLib\\zlib_x64d.lib")
#	else
#		pragma comment(lib,"eck\\ZLib\\zlib_x64.lib")
#	endif
#else
#	ifdef _DEBUG
#		pragma comment(lib,"eck\\ZLib\\zlib_x86d.lib")
#	else
#		pragma comment(lib,"eck\\ZLib\\zlib_x86.lib")
#	endif
#endif

#endif

// ECKMACRO_NO_WIN11_22621  SDK版本是否>=22621