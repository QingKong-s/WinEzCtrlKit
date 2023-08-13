/*
* WinEzCtrlKit Library
*
* ImageHelper.h £º Í¼Ïñ°ïÖúº¯Êý
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "ECK.h"

#include <Shlwapi.h>

ECK_NAMESPACE_BEGIN
HBITMAP CreateHBITMAP(PCVOID pData, SIZE_T cbData);
HBITMAP CreateHBITMAP(PCWSTR pszFile);
HICON CreateHICON(PCVOID pData, SIZE_T cbData);
HICON CreateHICON(PCWSTR pszFile);
ECK_NAMESPACE_END