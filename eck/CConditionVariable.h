#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CConditionVariable
{
private:
    RTL_CONDITION_VARIABLE m_cv;
public:
    ECK_DISABLE_COPY_MOVE(CConditionVariable)
public:
    CConditionVariable() noexcept
    {
        RtlInitializeConditionVariable(&m_cv);
    }

    EckInline NTSTATUS SleepLi(RTL_CRITICAL_SECTION* pcs, LARGE_INTEGER* pli = nullptr) noexcept
    {
        return RtlSleepConditionVariableCS(&m_cv, pcs, pli);
    }
    EckInline NTSTATUS SleepLi(RTL_SRWLOCK* psrw, BOOL bReadLock, LARGE_INTEGER* pli = nullptr) noexcept
    {
        return RtlSleepConditionVariableSRW(&m_cv, psrw, pli,
            bReadLock ? CONDITION_VARIABLE_LOCKMODE_SHARED : 0);
    }

    EckInline NTSTATUS Sleep100ns(RTL_CRITICAL_SECTION* pcs, LONGLONG ll) noexcept
    {
        ll *= -1;
        return SleepLi(pcs, (LARGE_INTEGER*)&ll);
    }
    EckInline NTSTATUS Sleep100ns(RTL_SRWLOCK* psrw, BOOL bReadLock, LONGLONG ll) noexcept
    {
        ll *= -1;
        return SleepLi(psrw, bReadLock, (LARGE_INTEGER*)&ll);
    }

    EckInline NTSTATUS Sleep(RTL_CRITICAL_SECTION* pcs, LONGLONG ll) noexcept
    {
        return Sleep100ns(pcs, ll * 10000ll);
    }
    EckInline NTSTATUS Sleep(RTL_SRWLOCK* psrw, BOOL bReadLock, LONGLONG ll) noexcept
    {
        return Sleep100ns(psrw, bReadLock, ll * 10000ll);
    }

    EckInline void Wake() noexcept
    {
        RtlWakeConditionVariable(&m_cv);
    }

    EckInline void WakeAll() noexcept
    {
        RtlWakeAllConditionVariable(&m_cv);
    }

    EckInline auto GetPointer() noexcept { return &m_cv; }
};
ECK_NAMESPACE_END