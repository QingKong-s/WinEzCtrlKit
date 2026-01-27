#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CSrwLock
{
private:
    RTL_SRWLOCK m_srw;
public:
    ECK_DISABLE_COPY_MOVE(CSrwLock)
public:
    CSrwLock() noexcept
    {
        RtlInitializeSRWLock(&m_srw);
    }

    _Acquires_shared_lock_(m_srw) EckInline void EnterRead() noexcept
    {
        RtlAcquireSRWLockShared(&m_srw);
    }

    EckInline BOOL TryEnterRead() noexcept
    {
        return RtlTryAcquireSRWLockShared(&m_srw);
    }

    _Releases_shared_lock_(m_srw) EckInline void LeaveRead() noexcept
    {
        RtlReleaseSRWLockShared(&m_srw);
    }

    _Acquires_exclusive_lock_(m_srw) EckInline void EnterWrite() noexcept
    {
        RtlAcquireSRWLockExclusive(&m_srw);
    }

    EckInline BOOL TryEnterWrite() noexcept
    {
        return RtlTryAcquireSRWLockExclusive(&m_srw);
    }

    _Releases_exclusive_lock_(m_srw) EckInline void LeaveWrite() noexcept
    {
#pragma warning(suppress : 26110)// 加锁顺序错误
        RtlReleaseSRWLockExclusive(&m_srw);
    }

    EckInlineNdCe auto GetPointer() noexcept { return &m_srw; }
};

class CSrwReadGuard
{
private:
    CSrwLock& x;
public:
    ECK_DISABLE_COPY_MOVE(CSrwReadGuard);
    CSrwReadGuard(CSrwLock& x) noexcept : x{ x } { x.EnterRead(); }
    ~CSrwReadGuard() { x.LeaveRead(); }
};

class CSrwWriteGuard
{
private:
    CSrwLock& x;
public:
    ECK_DISABLE_COPY_MOVE(CSrwWriteGuard);
    CSrwWriteGuard(CSrwLock& x) noexcept : x{ x } { x.EnterWrite(); }
    ~CSrwWriteGuard() { x.LeaveWrite(); }
};
ECK_NAMESPACE_END