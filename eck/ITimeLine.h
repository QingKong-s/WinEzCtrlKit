#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
struct ITimeLine
{
    // 滴答时间线
    virtual void TlTick(int iMs) noexcept = 0;
    // 时间线是否有效
    virtual BOOL TlIsValid() noexcept = 0;
    // 取当前滴答间隔
    virtual int TlGetCurrentInterval() noexcept = 0;
};
ECK_NAMESPACE_END