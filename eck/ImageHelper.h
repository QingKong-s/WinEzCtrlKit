/*
* WinEzCtrlKit Library
*
* ImageHelper.h ： 图像帮助函数
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <Shlwapi.h>
#include <wincodec.h>

ECK_NAMESPACE_BEGIN
HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData);

HBITMAP CreateHBITMAP(PCWSTR pszFile);

HBITMAP CreateHBITMAP(IWICBitmap* pBmp);

HICON CreateHICON(PCVOID pData, SIZE_T cbData);

HICON CreateHICON(PCWSTR pszFile);

ECK_NAMESPACE_END