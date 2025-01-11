/*
* WinEzCtrlKit Library
*
* ThreadPool.h : NT线程池
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CThreadPool
{
private:
	TP_POOL* m_pPool{};
public:
	CThreadPool() = default;
	CThreadPool(const CThreadPool&) = delete;
	CThreadPool& operator=(const CThreadPool&) = delete;
	constexpr CThreadPool(CThreadPool&& x) noexcept : m_pPool{ x.m_pPool }
	{
		x.m_pPool = nullptr;
	}
	constexpr CThreadPool& operator=(CThreadPool&& x) noexcept
	{
		std::swap(m_pPool, x.m_pPool);
		return *this;
	}

	~CThreadPool() { Close(); }

	EckInline NTSTATUS Create() { Close(); return TpAllocPool(&m_pPool, nullptr); }

	EckInline void Close()
	{
		if (m_pPool)
		{
			TpReleasePool(m_pPool);
			m_pPool = nullptr;
		}
	}

	EckInline void SetMaxThreads(DWORD c) { TpSetPoolMaxThreads(m_pPool, c); }

	EckInline NTSTATUS SetMinThreads(DWORD c) { return TpSetPoolMinThreads(m_pPool, c); }

	EckInline constexpr [[nodiscard]] TP_POOL* GetPool() const noexcept { return m_pPool; }
};

class CTpEnvironment
{
private:
	TP_CALLBACK_ENVIRON m_Env;
public:
	CTpEnvironment()
	{
		TpInitializeCallbackEnviron(&m_Env);
	}
	CTpEnvironment(const CTpEnvironment&) = delete;
	CTpEnvironment& operator=(const CTpEnvironment&) = delete;
	CTpEnvironment(CTpEnvironment&&) = delete;
	CTpEnvironment& operator=(CTpEnvironment&&) = delete;
	~CTpEnvironment()
	{
		TpDestroyCallbackEnviron(&m_Env);
	}

	EckInline constexpr auto& GetEnv() const noexcept { return m_Env; }
	EckInline constexpr auto& GetEnv() noexcept { return m_Env; }

	EckInline void SetCleanupGroup(TP_CLEANUP_GROUP* pGroup,
		PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfnCancel = nullptr)
	{
		TpSetCallbackCleanupGroup(&m_Env, pGroup, pfnCancel);
	}

	EckInline void SetLibrary(HMODULE hMod) { TpSetCallbackRaceWithDll(&m_Env, hMod); }

	EckInline void SetPool(TP_POOL* Pool) { TpSetCallbackThreadpool(&m_Env, Pool); }
	EckInline void SetPool(CThreadPool& Pool) { SetPool(Pool.GetPool()); }

	EckInline void SetPriority(TP_CALLBACK_PRIORITY Priority) { TpSetCallbackPriority(&m_Env, Priority); }

	EckInline void SetRunsLong(BOOL bRunsLong) { TpSetCallbackLongFunction(&m_Env); }
};

class CTpWork
{
private:
	TP_WORK* m_pWork{};

	template<class TTuple, size_t... Idx>
	static void CALLBACK EckWorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
		PTP_WORK Work)
	{
		std::unique_ptr<TTuple> pParam((TTuple*)(Context));
		std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Work);
	}

	template<class TTuple, size_t... Idx>
	static auto MakeCallback(std::index_sequence<Idx...>)
	{
		return &EckWorkCallback<TTuple, Idx...>;
	}
public:
	CTpWork() = default;
	CTpWork(const CTpWork&) = delete;
	CTpWork& operator=(const CTpWork&) = delete;
	constexpr CTpWork(CTpWork&& x) noexcept : m_pWork{ x.m_pWork }
	{
		x.m_pWork = nullptr;
	}
	constexpr CTpWork& operator=(CTpWork&& x) noexcept
	{
		std::swap(m_pWork, x.m_pWork);
		return *this;
	}

	~CTpWork() { Close(); }

	EckInline NTSTATUS Create(PTP_WORK_CALLBACK pfnCallback, void* pContext,
		TP_CALLBACK_ENVIRON* pEnv = nullptr)
	{
		Close();
		return TpAllocWork(&m_pWork, pfnCallback, pContext, pEnv);
	}

	EckInline NTSTATUS Create(PTP_WORK_CALLBACK pfnCallback, void* pContext,
		CTpEnvironment& Env)
	{
		return Create(pfnCallback, pContext, &Env.GetEnv());
	}

	// F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_WORK Work)
	template<class F, class... TArgs>
	EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args)
	{
		using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
		auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
		return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 1>{}),
			pParam.release(), pEnv);
	}

	EckInline void Close()
	{
		if (m_pWork)
		{
			TpReleaseWork(m_pWork);
			m_pWork = nullptr;
		}
	}

	EckInline void Submit() { TpPostWork(m_pWork); }

	EckInline void Wait(BOOL bCancelPending = FALSE)
	{
		TpWaitForWork(m_pWork, (LOGICAL)bCancelPending);
	}

	EckInline constexpr [[nodiscard]] TP_WORK* GetWork() const noexcept { return m_pWork; }
};

class CTpWait
{
private:
	TP_WAIT* m_pWait{};

	template<class TTuple, size_t... Idx>
	static void CALLBACK EckWaitCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
		PTP_WAIT Wait, TP_WAIT_RESULT Result)
	{
		std::unique_ptr<TTuple> pParam((TTuple*)(Context));
		std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Wait, Result);
	}

	template<class TTuple, size_t... Idx>
	static auto MakeCallback(std::index_sequence<Idx...>)
	{
		return &EckWaitCallback<TTuple, Idx...>;
	}
public:
	CTpWait() = default;
	CTpWait(const CTpWait&) = delete;
	CTpWait& operator=(const CTpWait&) = delete;
	constexpr CTpWait(CTpWait&& x) noexcept : m_pWait{ x.m_pWait }
	{
		x.m_pWait = nullptr;
	}
	constexpr CTpWait& operator=(CTpWait&& x) noexcept
	{
		std::swap(m_pWait, x.m_pWait);
		return *this;
	}

	~CTpWait() { Close(); }

	EckInline NTSTATUS Create(PTP_WAIT_CALLBACK pfnCallback, void* pContext,
		TP_CALLBACK_ENVIRON* pEnv = nullptr)
	{
		Close();
		return TpAllocWait(&m_pWait, pfnCallback, pContext, pEnv);
	}

	EckInline NTSTATUS Create(PTP_WAIT_CALLBACK pfnCallback, void* pContext,
		CTpEnvironment& Env)
	{
		return Create(pfnCallback, pContext, &Env.GetEnv());
	}

	// F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_WAIT Wait, TP_WAIT_RESULT Result)
	template<class F, class... TArgs>
	EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args)
	{
		using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
		auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
		return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 2>{}),
			pParam.release(), pEnv);
	}

	EckInline void Close()
	{
		if (m_pWait)
		{
			TpReleaseWait(m_pWait);
			m_pWait = nullptr;
		}
	}

	EckInline void SetWait(HANDLE hObj, LONGLONG llTimeout)
	{
		TpSetWait(m_pWait, hObj, (LARGE_INTEGER*)&llTimeout);
	}

	EckInline void SetWait(HANDLE hObj, LARGE_INTEGER liTimeout)
	{
		TpSetWait(m_pWait, hObj, &liTimeout);
	}

	EckInline void SetWait(HANDLE hObj)
	{
		TpSetWait(m_pWait, hObj, nullptr);
	}

	EckInline void Wait(BOOL bCancelPending = FALSE)
	{
		TpWaitForWait(m_pWait, (LOGICAL)bCancelPending);
	}

	EckInline constexpr [[nodiscard]] TP_WAIT* GetWait() const noexcept { return m_pWait; }
};

class CTpTimer
{
private:
	TP_TIMER* m_pTimer{};

	template<class TTuple, size_t... Idx>
	static void CALLBACK EckTimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
		PTP_TIMER Timer)
	{
		std::unique_ptr<TTuple> pParam((TTuple*)(Context));
		std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Timer);
	}

	template<class TTuple, size_t... Idx>
	static auto MakeCallback(std::index_sequence<Idx...>)
	{
		return &EckTimerCallback<TTuple, Idx...>;
	}
public:
	CTpTimer() = default;
	CTpTimer(const CTpTimer&) = delete;
	CTpTimer& operator=(const CTpTimer&) = delete;
	constexpr CTpTimer(CTpTimer&& x) noexcept : m_pTimer{ x.m_pTimer }
	{
		x.m_pTimer = nullptr;
	}
	constexpr CTpTimer& operator=(CTpTimer&& x) noexcept
	{
		std::swap(m_pTimer, x.m_pTimer);
		return *this;
	}

	~CTpTimer() { Close(); }

	EckInline NTSTATUS Create(PTP_TIMER_CALLBACK pfnCallback, void* pContext,
		TP_CALLBACK_ENVIRON* pEnv = nullptr)
	{
		Close();
		return TpAllocTimer(&m_pTimer, pfnCallback, pContext, pEnv);
	}

	EckInline NTSTATUS Create(PTP_TIMER_CALLBACK pfnCallback, void* pContext,
		CTpEnvironment& Env)
	{
		return Create(pfnCallback, pContext, &Env.GetEnv());
	}

	// F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_TIMER Timer)
	template<class F, class... TArgs>
	EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args)
	{
		using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
		auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
		return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 1>{}),
			pParam.release(), pEnv);
	}

	EckInline void Close()
	{
		if (m_pTimer)
		{
			TpReleaseTimer(m_pTimer);
			m_pTimer = nullptr;
		}
	}

	EckInline void SetTimer2(LARGE_INTEGER* pliDueTime, DWORD msPeriod, DWORD msAdjust = 0)
	{
		TpSetTimer(m_pTimer, pliDueTime, msPeriod, msAdjust);
	}

	EckInline void SetTimer(LONGLONG llDueTime, DWORD msPeriod, DWORD msAdjust = 0)
	{
		SetTimer2((LARGE_INTEGER*)&llDueTime, msPeriod, msAdjust);
	}

	EckInline BOOL IsSet() const { return TpIsTimerSet(m_pTimer); }

	EckInline void Wait(BOOL bCancelPending = FALSE)
	{
		TpWaitForTimer(m_pTimer, (LOGICAL)bCancelPending);
	}

	EckInline constexpr [[nodiscard]] TP_TIMER* GetTimer() const noexcept { return m_pTimer; }
};

namespace Priv
{
	template<class TTuple, size_t... Idx>
	void CALLBACK EckTpSimpleCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context)
	{
		std::unique_ptr<TTuple> pParam((TTuple*)(Context));
		std::invoke(std::move(std::get<Idx>(*pParam))..., Instance);
	}

	template<class TTuple, size_t... Idx>
	auto MakeTpSimpleCallback(std::index_sequence<Idx...>)
	{
		return &EckTpSimpleCallback<TTuple, Idx...>;
	}
}

// F = void()(..., PTP_CALLBACK_INSTANCE Instance)
template<class F, class... TArgs>
EckInline NTSTATUS TpSubmitSimpleCallback(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args)
{
	using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
	auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
	return TpSimpleTryPost(Priv::MakeTpSimpleCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs)>{}),
		pParam.release(), pEnv);
}
ECK_NAMESPACE_END