/*
* WinEzCtrlKit Library
*
* ITimeLine.h ： 时间线接口
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
constexpr inline IID IID_ITimeLine
{ 0xfaf92698, 0xd74, 0x4d14, { 0x90, 0xa5, 0x4e, 0x66, 0xc1, 0xa, 0x96, 0x30 } };

ECK_COM_INTERFACE("FAF92698-0D74-4D14-90A5-4E66C10A9630")
ITimeLine : public IUnknown
{
	// 滴答时间线
	void STDMETHODCALLTYPE Tick(int iMs);

	// 时间线是否有效
	BOOL STDMETHODCALLTYPE IsValid();

	// 取当前滴答间隔
	int STDMETHODCALLTYPE GetCurrTickInterval();
};
ECK_NAMESPACE_END