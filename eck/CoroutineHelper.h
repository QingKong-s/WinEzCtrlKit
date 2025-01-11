#pragma once
#include "CSrwLock.h"
#include "ThreadPool.h"

#include <coroutine>

ECK_NAMESPACE_BEGIN
struct CExceptionCoroCancelled {};

struct CoroPromiseBase
{
	using FCanceller = void(*)(void* pCtx);

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
	FCanceller m_pfnCanceller{};
	void* m_pCtx{};
	std::function<void()> m_fnOnCancel{};
	CSrwLock m_Lk{};

	constexpr auto initial_suspend() { return std::suspend_never{}; }
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

	void unhandled_exception() try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (CExceptionCoroCancelled)
	{
		// 提供co_await检测取消的能力，在await_resume中抛出CExceptionCoroCancelled即可退出协程
		__iso_volatile_store32((int*)&m_eState, (int)State::Cancelled);
	}
	catch (...)
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

	EckInline BOOL TryCancel() noexcept
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
		CSrwWriteGuard _{ m_Lk };
		if (m_pfnCanceller)
			m_pfnCanceller(m_pCtx);
		if (m_fnOnCancel)
		{
			auto fn{ std::move(m_fnOnCancel) };
			fn();
		}
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
		m_pCtx = pCtx;
	}

	EckInline void SetOnCancel(const std::function<void()>& fnOnCancel) noexcept
	{
		CSrwWriteGuard _{ m_Lk };
		m_fnOnCancel = fnOnCancel;
	}

	EckInline void SetOnCancel(std::function<void()>&& fnOnCancel) noexcept
	{
		CSrwWriteGuard _{ m_Lk };
		m_fnOnCancel = std::move(fnOnCancel);
	}
};

template<class T>
struct CoroRetVal
{
private:
	std::optional<T> m_RetVal;
public:
	void return_value(T&& Val) noexcept
	{
		m_RetVal.emplace(std::forward<T>(Val));
	}

	constexpr auto& GetOptional() const noexcept { return m_RetVal; }
};

template<>
struct CoroRetVal<void>
{
	constexpr void return_void() const noexcept {}
};

// 标准任务
template<class TRet = void>
struct CoroTask
{
	struct promise_type;

	std::coroutine_handle<promise_type> hCoroutine{};

	struct promise_type : CoroPromiseBase, CoroRetVal<TRet>
	{
		CoroTask get_return_object() noexcept
		{
			return CoroTask{ std::coroutine_handle<promise_type>::from_promise(*this) };
		}
	};

	auto operator co_await() const noexcept
	{
		struct Awaiter
		{
			promise_type& Pro;
			bool await_ready() const noexcept { return Pro.IsCompleted(); }
			bool await_suspend(std::coroutine_handle<promise_type> h) const noexcept
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

	CoroTask& operator=(const CoroTask& x)
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

	// Forwarder

	EckInline BOOL IsCompleted() const noexcept { return hCoroutine.promise().IsCompleted(); }
	EckInline BOOL TryCancel() noexcept { return hCoroutine.promise().TryCancel(); }
	EckInline BOOL IsCanceled() const noexcept { return hCoroutine.promise().IsCanceled(); }
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

namespace Priv
{
	template<class T>
		requires std::is_base_of_v<CoroPromiseBase, T>
	auto ToCoroPromiseBaseHandle(std::coroutine_handle<T> h)
	{
		return std::coroutine_handle<CoroPromiseBase>::from_address(h.address());
	}

	struct CancellationTokenAwaiter
	{
		CoroPromiseBase* pPro{};
		bool await_ready() const noexcept { return false; }

		template<class T>
		bool await_suspend(std::coroutine_handle<T> h) noexcept
		{
			pPro = &ToCoroPromiseBaseHandle(h).promise();
			return false;
		}

		constexpr auto await_resume() const noexcept
		{
			struct Token
			{
				CoroPromiseBase* pPro;
				EckInline BOOL IsCanceled() const noexcept { return pPro->IsCanceled(); }
			};
			return Token{ pPro };
		}
	};

	struct TimerAwaiter
	{
		LONGLONG ms;
		CTpTimer Timer;
		CoroPromiseBase* pPro;

		static void CALLBACK TimerProc(TP_CALLBACK_INSTANCE*, void* pCtx, TP_TIMER*)
		{
			std::coroutine_handle<>::from_address(pCtx).resume();
		}

		constexpr bool await_ready() const noexcept { return false; }

		template<class T>
		void await_suspend(std::coroutine_handle<T> h) noexcept
		{
			pPro = &Priv::ToCoroPromiseBaseHandle(h).promise();
			pPro->SetCanceller([](void* pCtx)
				{
					const auto p = (TimerAwaiter*)pCtx;
					p->Timer.SetTimer(0, 0);
				}, this);
			Timer.Create(TimerProc, h.address(), nullptr);
			Timer.SetTimer(-ms * 10000ll, 0);
		}

		void await_resume() const
		{
			if (pPro->IsCanceled())
				throw CExceptionCoroCancelled{};
		}
	};
}

// 取取消令牌，用于主动轮询取消状态
EckInline auto CoroGetCancellationToken() { return Priv::CancellationTokenAwaiter{}; }

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
EckInline auto CoroSleep(LONGLONG ms) { return Priv::TimerAwaiter{ ms }; }

// 捕获UI线程上下文，稍后可使用co_await返回至UI线程
EckInline auto CoroCaptureUiThread()
{
	struct Context
	{
	private:
		UINT Id;
	public:
		Context() : Id{ NtCurrentThreadId32() } { EckAssert(GetThreadCtx()); }

		constexpr bool await_ready() const noexcept { return false; }
		void await_suspend(std::coroutine_handle<> h) const noexcept
		{
			PostThreadMessageW(Id, TM_ENQUEUECALLBACK, (WPARAM)h.address(), -1);
		}
		constexpr void await_resume() const noexcept {}

		EckInline constexpr UINT GetThreadId() const noexcept { return Id; }
	};
	return Context{};
}
ECK_NAMESPACE_END