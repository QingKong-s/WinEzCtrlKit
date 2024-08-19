/*
* WinEzCtrlKit Library
*
* CSrwLock.h ： SRW锁
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CSrwLock
{
private:
	SRWLOCK m_srw;
public:
	ECK_DISABLE_COPY_MOVE(CSrwLock)
public:
	EckInline CSrwLock()
	{
		InitializeSRWLock(&m_srw);
	}

	_Acquires_shared_lock_(m_srw) EckInline void EnterRead()
	{
		AcquireSRWLockShared(&m_srw);
	}

	EckInline BOOL TryEnterRead()
	{
		return TryAcquireSRWLockShared(&m_srw);
	}

	_Releases_shared_lock_(m_srw) EckInline void LeaveRead()
	{
		ReleaseSRWLockShared(&m_srw);
	}

	_Acquires_exclusive_lock_(m_srw) EckInline void EnterWrite()
	{
		AcquireSRWLockExclusive(&m_srw);
	}

	EckInline BOOL TryEnterWrite()
	{
		return TryAcquireSRWLockExclusive(&m_srw);
	}

	_Releases_exclusive_lock_(m_srw) EckInline void LeaveWrite()
	{
#pragma warning(suppress : 26110)// 加锁顺序错误
		ReleaseSRWLockExclusive(&m_srw);
	}

	EckInline auto GetPSrw() { return &m_srw; }
};
ECK_NAMESPACE_END