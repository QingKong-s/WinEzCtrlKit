#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct IScroll
{
	// 当前滚动参数是否有效
	virtual BOOL ScrIsValid() = 0;
	// 是否有必要显示滚动条
	virtual BOOL ScrIsVisible() = 0;
	virtual void ScrSetScrollInfo(const SCROLLINFO& si) = 0;
	virtual void ScrGetScrollInfo(_Inout_ SCROLLINFO& si) = 0;
	virtual void ScrSetViewSize(int iViewSize) = 0;
	virtual int ScrGetViewSize() = 0;
};
ECK_NAMESPACE_END