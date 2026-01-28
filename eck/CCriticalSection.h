#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CCriticalSection
{
private:
    RTL_CRITICAL_SECTION m_cs;
public:
    ECK_DISABLE_COPY_MOVE(CCriticalSection);
    CCriticalSection(UINT cSpin) noexcept
    {
        RtlInitializeCriticalSectionAndSpinCount(&m_cs, cSpin);
    }
    CCriticalSection() noexcept
    {
        RtlInitializeCriticalSection(&m_cs);
    }
    ~CCriticalSection()
    {
        RtlDeleteCriticalSection(&m_cs);
    }

    EckInline _Acquires_lock_(m_cs) void Enter() noexcept
    {
        RtlEnterCriticalSection(&m_cs);
    }

    EckInline BOOL TryEnter() noexcept
    {
        RtlTryEnterCriticalSection(&m_cs);
    }

    EckInline _Releases_lock_(m_cs) void Leave() noexcept
    {
        RtlLeaveCriticalSection(&m_cs);
    }

    EckInline auto GetPointer() noexcept { return &m_cs; }
};

class CCsGuard
{
private:
    CCriticalSection& x;
public:
    ECK_DISABLE_COPY_MOVE(CCsGuard);
    CCsGuard(CCriticalSection& x) : x{ x } { x.Enter(); }
    ~CCsGuard() { x.Leave(); }
};
ECK_NAMESPACE_END