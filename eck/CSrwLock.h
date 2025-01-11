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
	EckInline CSrwLock()
	{
		RtlInitializeSRWLock(&m_srw);
	}

	_Acquires_shared_lock_(m_srw) EckInline void EnterRead()
	{
		RtlAcquireSRWLockShared(&m_srw);
	}

	EckInline BOOL TryEnterRead()
	{
		return RtlTryAcquireSRWLockShared(&m_srw);
	}

	_Releases_shared_lock_(m_srw) EckInline void LeaveRead()
	{
		RtlReleaseSRWLockShared(&m_srw);
	}

	_Acquires_exclusive_lock_(m_srw) EckInline void EnterWrite()
	{
		RtlAcquireSRWLockExclusive(&m_srw);
	}

	EckInline BOOL TryEnterWrite()
	{
		return RtlTryAcquireSRWLockExclusive(&m_srw);
	}

	_Releases_exclusive_lock_(m_srw) EckInline void LeaveWrite()
	{
#pragma warning(suppress : 26110)// 加锁顺序错误
		RtlReleaseSRWLockExclusive(&m_srw);
	}

	EckInline auto GetPSrw() { return &m_srw; }
};

class CSrwReadGuard
{
private:
	CSrwLock& x;
public:
	ECK_DISABLE_COPY_MOVE(CSrwReadGuard);
	EckInline CSrwReadGuard(CSrwLock& x) :x{ x }
	{
		x.EnterRead();
	}

	EckInline ~CSrwReadGuard()
	{
		x.LeaveRead();
	}
};

class CSrwWriteGuard
{
private:
	CSrwLock& x;
public:
	ECK_DISABLE_COPY_MOVE(CSrwWriteGuard);
	EckInline CSrwWriteGuard(CSrwLock& x) :x{ x }
	{
		x.EnterWrite();
	}

	EckInline ~CSrwWriteGuard()
	{
		x.LeaveWrite();
	}
};
ECK_NAMESPACE_END