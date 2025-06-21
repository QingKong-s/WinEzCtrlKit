#pragma once
#include "CSrwLock.h"
#include "ThreadPool.h"
#include "CEvent.h"

#include <coroutine>

ECK_NAMESPACE_BEGIN
struct CExceptionCoroCancelled {};

struct CoroPromiseBase
{
	using FCanceller = void(*)(void* pCtx);
	using FOnCancel = std::function<void()>;

	enum class State : LONG
	{
		Initial,	// 无
		HasNext,	// 同步任务，需要恢复到等待方
		Completed,	// 异步任务，已完成
		Cancelled,	// 提供可设置为取消标志的能力
	};

	LONG m_cRef{ 2 };// 协程引用计数，Task对象与协程帧各持有一份引用
	State m_eState{ State::Initial };
	std::coroutine_handle<> m_hCoroNext{};// 同步任务应恢复到
	CSrwLock m_Lk{};
	FCanceller m_pfnCanceller{};
	void* m_pCancellerCtx{};
	FOnCancel m_fnOnCancel{};

	constexpr auto initial_suspend() noexcept { return std::suspend_never{}; }
	auto final_suspend() noexcept
	{
		struct Awaiter
		{
			CoroPromiseBase& Pro;

			constexpr bool await_ready() const noexcept { return false; }
			bool await_suspend(std::coroutine_handle<>) const noexcept
			{
				// 如果状态为HasNext，则协程已作为同步任务，即它的任务对象被其他协程co_await，
				// 外界根本不需要知道它的完成状态，因此状态字段对外界没有必要
				// 
				// 如果状态为Initial，则协程作为异步任务，为了让外界具有查询其完成状态的能力，
				// 状态字段设置为Completed
				//
				_InterlockedCompareExchange((LONG*)&Pro.m_eState,
					(LONG)State::Completed, (LONG)State::Initial);
				if (Pro.m_hCoroNext)// 恢复到等待方
					Pro.m_hCoroNext.resume();
				// 若减引用后为0，说明Task已析构，则返回false，协程结束并自动销毁；否则，
				// Task仍保留一份引用，此时返回true，协程暂停在最终暂停点，等待Task析构时主动销毁
				//
				return Pro.DecRef() != 0;
			}
			constexpr void await_resume() const noexcept {}
		};

		return Awaiter{ *this };
	}

	void unhandled_exception()
#ifdef __cpp_exceptions
	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (CExceptionCoroCancelled)
	{
		// 提供co_await检测取消并退出的能力，在await_resume
		// 中抛出CExceptionCoroCancelled即可退出协程
		__iso_volatile_store32((int*)&m_eState, (int)State::Cancelled);
	}
	catch (...)
#endif // __cpp_exceptions
	{
		abort();
	}

	EckInline LONG IncRef() noexcept
	{
		return _InterlockedIncrement(&m_cRef);
	}

	EckInline LONG DecRef() noexcept
	{
		return _InterlockedDecrement(&m_cRef);
	}

	bool SyncResumeToIt(std::coroutine_handle<> hCoroNext_) noexcept
	{
		EckAssert(!m_hCoroNext && L"不能重复等待同步任务");
		m_hCoroNext = hCoroNext_;
		// 若原状态为Completed或Canceled，无需执行当前协程。。
		// 原状态不可能为HasNext
		return (State)_InterlockedCompareExchange((LONG*)&m_eState,
			(LONG)State::HasNext, (LONG)State::Initial) == State::Initial;
	}

	EckInline BOOL IsCompleted() const noexcept
	{
		return (State)__iso_volatile_load32((const int*)&m_eState) == State::Completed;
	}

	BOOL TryCancel() noexcept
	{
		auto eOld = (State)_InterlockedCompareExchange((LONG*)&m_eState,
			(LONG)State::Cancelled, (LONG)State::Initial);
		EckAssert(eOld != State::Cancelled && L"协程已经被取消过");
		if (eOld == State::Initial)
			goto Success;
		eOld = (State)_InterlockedCompareExchange((LONG*)&m_eState,
			(LONG)State::Cancelled, (LONG)State::HasNext);
		if (eOld == State::HasNext)
			goto Success;
		return FALSE;
	Success:;
		m_Lk.EnterWrite();
		const auto pfnCanceller{ m_pfnCanceller };
		const auto pCancellerCtx{ m_pCancellerCtx };
		const auto fnOnCancel{ std::move(m_fnOnCancel) };
		m_Lk.LeaveWrite();
		if (pfnCanceller)
			pfnCanceller(pCancellerCtx);
		if (fnOnCancel)
			fnOnCancel();
		return TRUE;
	}

	EckInline BOOL IsCanceled() const noexcept
	{
		return (State)__iso_volatile_load32((const int*)&m_eState) == State::Cancelled;
	}

	EckInline void SetCanceller(FCanceller pfnCanceller, void* pCtx) noexcept
	{
		CSrwWriteGuard _{ m_Lk };
		m_pfnCanceller = pfnCanceller;
		m_pCancellerCtx = pCtx;
	}

	template<class T>
	EckInline void SetOnCancel(T&& fnOnCancel) noexcept
	{
		CSrwWriteGuard _{ m_Lk };
		m_fnOnCancel = std::forward<T>(fnOnCancel);
	}
};

// 进度
template<class TProgress>
struct CoroPromiseBaseWithProgress : CoroPromiseBase
{
	using FOnProgress = std::function<void(TProgress)>;
	FOnProgress m_fnOnProgress{};

	template<class T>
	EckInline void SetOnProgress(T&& fnOnProgress) noexcept
	{
		CSrwWriteGuard _{ m_Lk };
		m_fnOnProgress = std::forward<T>(fnOnProgress);
	}

	template<class U>
		requires std::is_convertible_v<U, TProgress>
	void OnProgress(U&& Progress)
	{
		m_Lk.EnterRead();
		if (!m_fnOnProgress)
		{
			m_Lk.LeaveRead();
			return;
		}
		const auto fnOnProgress{ m_fnOnProgress };
		m_Lk.LeaveRead();
		fnOnProgress(std::forward<U>(Progress));
	}
};

// 进度类型选择器
template<class TProgress>
using CoroPromiseBase_T = std::conditional_t<std::is_void_v<TProgress>,
	CoroPromiseBase, CoroPromiseBaseWithProgress<TProgress>>;

// 返回值
template<class T>
struct CoroRetVal
{
private:
	std::optional<T> m_RetVal;
public:
	template<class U>
	void return_value(U&& Val) noexcept
	{
		m_RetVal.emplace(std::forward<T>(Val));
	}

	constexpr auto& GetRetVal() const noexcept { return m_RetVal; }
	constexpr auto& GetRetVal() noexcept { return m_RetVal; }
};

// 无返回值
template<>
struct CoroRetVal<void>
{
	constexpr void return_void() const noexcept {}
};

namespace Priv
{
	struct CoroPromiseTokenAwaiter_T {};
}

// 标准任务
template<class TRet = void, class TProgress = void>
struct CoroTask
{
	struct promise_type;

	std::coroutine_handle<promise_type> hCoroutine{};

	struct promise_type : CoroPromiseBase_T<TProgress>, CoroRetVal<TRet>
	{
		CoroTask get_return_object() noexcept
		{
			return CoroTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}

		template<class T>
		auto await_transform(T&& Awaitable) const noexcept
		{
			return std::forward<T>(Awaitable);
		}

		auto await_transform(Priv::CoroPromiseTokenAwaiter_T) noexcept
		{
			struct Token
			{
				promise_type& Pro;

				constexpr bool await_ready() const noexcept { return true; }
				constexpr void await_suspend(std::coroutine_handle<>) const noexcept {};
				constexpr Token await_resume() const noexcept { return *this; }

				EckInline constexpr auto& GetPromise() const noexcept { return Pro; }
				EckInline constexpr auto& GetPromise() noexcept { return Pro; }
				EckInline constexpr auto GetCoroHandle() const noexcept
				{
					return std::coroutine_handle<promise_type>::from_promise(Pro);
				}
			};
			return Token{ *this };
		}
	};

	auto operator co_await() const noexcept
	{
		struct Awaiter
		{
			promise_type& Pro;
			bool await_ready() const noexcept { return Pro.IsCompleted(); }
			bool await_suspend(std::coroutine_handle<> h) const noexcept
			{
				return Pro.SyncResumeToIt(h);
			}
			constexpr void await_resume() const noexcept {}
		};
		return Awaiter{ hCoroutine.promise() };
	}

	CoroTask() = default;
	CoroTask(std::coroutine_handle<promise_type> h) noexcept : hCoroutine{ h } {}
	CoroTask(const CoroTask& x)
	{
		if (&x != this)
		{
			if (hCoroutine = x.hCoroutine)
				hCoroutine.promise().IncRef();
		}
	}

	CoroTask(CoroTask&& x) noexcept
	{
		std::swap(hCoroutine, x.hCoroutine);
	}

	CoroTask& operator=(const CoroTask& x) noexcept
	{
		if (&x != this)
		{
			CoroTask t{ hCoroutine };
			if (hCoroutine = x.hCoroutine)
				hCoroutine.promise().IncRef();
		}
		return *this;
	}

	CoroTask& operator=(CoroTask&& x) noexcept
	{
		std::swap(hCoroutine, x.hCoroutine);
		return *this;
	}

	~CoroTask()
	{
		if (hCoroutine && (hCoroutine.promise().DecRef() == 0))
			hCoroutine.destroy();
	}

	void SyncWait() const
	{
		CEvent Event(nullptr, FALSE, FALSE);

		[this, &Event]() -> CoroTask<> {
			co_await *this;
			Event.Signal();
			}();

		WaitObject(Event);
	}

	// Forwarder

	EckInline auto& GetPromise() const noexcept { return hCoroutine.promise(); }
	EckInline auto& GetRetVal() const noexcept { return GetPromise().GetRetVal(); }
	EckInline auto& GetRetVal() noexcept { return GetPromise().GetRetVal(); }
	EckInline BOOL IsCompleted() const noexcept { return GetPromise().IsCompleted(); }
	EckInline BOOL TryCancel() noexcept { return GetPromise().TryCancel(); }
	EckInline BOOL IsCanceled() const noexcept { return GetPromise().IsCanceled(); }
};

// 发后不理
struct CoroTaskFireAndForget
{
	struct promise_type
	{
		constexpr CoroTaskFireAndForget get_return_object() const noexcept { return {}; }
		constexpr void return_void() const noexcept {}
		void unhandled_exception() const noexcept { std::abort(); }
		constexpr auto initial_suspend() const noexcept { return std::suspend_never{}; }
		constexpr auto final_suspend() const noexcept { return std::suspend_never{}; }
	};

	constexpr auto operator co_await() const noexcept { return std::suspend_never{}; }
};

// 转为CoroPromiseBase的句柄
template<class T>
	requires std::is_base_of_v<CoroPromiseBase, T>
auto ToCoroPromiseBaseHandle(std::coroutine_handle<T> h)
{
	return std::coroutine_handle<CoroPromiseBase>::from_address(h.address());
}

namespace Priv
{
	// 定时器
	struct CoroTimerAwaiter
	{
		LONGLONG ms;
		CTpTimer Timer;
		CoroPromiseBase* pPro;

		constexpr bool await_ready() const noexcept { return false; }

		template<class T>
		void await_suspend(std::coroutine_handle<T> h) noexcept
		{
			pPro = &ToCoroPromiseBaseHandle(h).promise();
			pPro->SetCanceller([](void* pCtx)
				{
					const auto p = (CoroTimerAwaiter*)pCtx;
					p->Timer.SetTimer(0, 0);
				}, this);
			Timer.Create([](TP_CALLBACK_INSTANCE*, void* pCtx, TP_TIMER*)
				{
					std::coroutine_handle<>::from_address(pCtx).resume();
				}, h.address(), nullptr);
			Timer.SetTimer(-ms * 10000ll/*反转符号*/, 0);
		}

		void await_resume() const
		{
			pPro->SetCanceller(nullptr, nullptr);
#ifdef __cpp_exceptions
			if (pPro->IsCanceled())
				throw CExceptionCoroCancelled{};
#endif // __cpp_exceptions
		}
	};

	struct CoroWaitableObjectAwaiter
	{
		HANDLE hWaitable;
		LONGLONG msTimeout;
		CTpWait Wait;
		CoroPromiseBase* pPro;

		constexpr bool await_ready() const noexcept { return false; }

		template<class T>
		void await_suspend(std::coroutine_handle<T> h) noexcept
		{
			pPro = &ToCoroPromiseBaseHandle(h).promise();
			pPro->SetCanceller([](void* pCtx)
				{
					const auto p = (CoroWaitableObjectAwaiter*)pCtx;
#if NTDDI_VERSION >= NTDDI_WIN8
					if (p->Wait.SetWaitEx(nullptr))// winrt does this, but WHY??
						p->Wait.SetWaitEx(NtCurrentProcess(), 0);
#else
					p->Wait.SetWait(nullptr, 0);
#endif// NTDDI_VERSION >= NTDDI_WIN8
				}, this);
			Wait.Create([](PTP_CALLBACK_INSTANCE, PVOID pCtx, PTP_WAIT, TP_WAIT_RESULT)
				{
					std::coroutine_handle<>::from_address(pCtx).resume();
				}, h.address());
			if (msTimeout == LLONG_MAX)
				Wait.SetWait(hWaitable);
			else
				Wait.SetWait(hWaitable, msTimeout * 10000ll);
		}

		void await_resume() const
		{
#ifdef __cpp_exceptions
			if (pPro->IsCanceled())
				throw CExceptionCoroCancelled{};
#endif // __cpp_exceptions
		}
	};
}

// 取承诺令牌
EckInline auto CoroGetPromiseToken() { return Priv::CoroPromiseTokenAwaiter_T{}; }

// 在线程池中恢复当前协程
EckInline auto CoroResumeBackground()
{
	struct Awaiter
	{
		constexpr bool await_ready() const noexcept { return false; }
		void await_suspend(std::coroutine_handle<> h) const noexcept
		{
			TpSimpleTryPost([](TP_CALLBACK_INSTANCE*, void* pCtx) {
				std::coroutine_handle<>::from_address(pCtx).resume();
				}, h.address(), nullptr);
		}
		constexpr void await_resume() const noexcept {}
	};
	return Awaiter{};
}

/// <summary>
/// 等待指定时间
/// </summary>
/// <param name="ms">正值为相对时间，负值为绝对时间</param>
/// <returns>等待体</returns>
EckInline auto CoroSleep(LONGLONG ms) { return Priv::CoroTimerAwaiter{ ms }; }

/// <summary>
/// 等待指定对象
/// </summary>
/// <param name="hWaitable">等待对象</param>
/// <param name="msTimeout">超时时间，正值为相对时间，负值为绝对时间，LLONG_MAX为无限等待</param>
/// <returns>等待体</returns>
EckInline auto CoroWait(HANDLE hWaitable, LONGLONG msTimeout = LLONG_MAX)
{
	return Priv::CoroWaitableObjectAwaiter{ hWaitable, msTimeout };
}

// 捕获UI线程上下文，稍后可使用co_await返回至UI线程
EckInline auto CoroCaptureUiThread(THREADCTX* ptc = nullptr)
{
	struct Context
	{
	private:
		Priv::QueuedCallbackQueue* m_pCallback{};
	public:
		UINT Priority{ UINT_MAX };
		BOOL IsWakeUiThread{ FALSE };

		Context(THREADCTX* ptc = nullptr) : m_pCallback{ &ptc->Callback } {}

		constexpr bool await_ready() const noexcept { return false; }
		void await_suspend(std::coroutine_handle<> h) const noexcept
		{
			m_pCallback->EnQueueCoroutine(h.address(), Priority, IsWakeUiThread);
		}
		constexpr void await_resume() const noexcept {}

		EckInlineNdCe auto GetCallbackQueue() const noexcept { return m_pCallback; }
	};
	return Context{ ptc ?  ptc : GetThreadCtx() };
}
ECK_NAMESPACE_END