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

    EckInline NTSTATUS Create() noexcept { Close(); return TpAllocPool(&m_pPool, nullptr); }

    EckInline void Close() noexcept
    {
        if (m_pPool)
        {
            TpReleasePool(m_pPool);
            m_pPool = nullptr;
        }
    }

    EckInline void SetMaximumThreads(UINT c) noexcept { TpSetPoolMaxThreads(m_pPool, c); }

    EckInline NTSTATUS SetMinimumThreads(UINT c)  noexcept { return TpSetPoolMinThreads(m_pPool, c); }

    EckInlineNdCe TP_POOL* GetPool() const noexcept { return m_pPool; }
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

    EckInlineNdCe auto& GetEnvironment() const noexcept { return m_Env; }
    EckInlineNdCe auto& GetEnvironment() noexcept { return m_Env; }

    EckInline void SetCleanupGroup(TP_CLEANUP_GROUP* pGroup,
        PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfnCancel = nullptr) noexcept
    {
        TpSetCallbackCleanupGroup(&m_Env, pGroup, pfnCancel);
    }

    EckInline void SetLibrary(HMODULE hMod) noexcept { TpSetCallbackRaceWithDll(&m_Env, hMod); }

    EckInline void SetPool(TP_POOL* Pool) noexcept { TpSetCallbackThreadpool(&m_Env, Pool); }
    EckInline void SetPool(CThreadPool& Pool) noexcept { SetPool(Pool.GetPool()); }

    EckInline void SetPriority(TP_CALLBACK_PRIORITY Priority) noexcept { TpSetCallbackPriority(&m_Env, Priority); }

    EckInline void SetLongFunction() noexcept { TpSetCallbackLongFunction(&m_Env); }
};

class CTpWork
{
private:
    TP_WORK* m_pWork{};

    template<class TTuple, size_t... Idx>
    static void CALLBACK EckWorkCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
        PTP_WORK Work) noexcept
    {
        std::unique_ptr<TTuple> pParam((TTuple*)(Context));
        std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Work);
    }

    template<class TTuple, size_t... Idx>
    static auto MakeCallback(std::index_sequence<Idx...>) noexcept
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
        TP_CALLBACK_ENVIRON* pEnv = nullptr) noexcept
    {
        Close();
        return TpAllocWork(&m_pWork, pfnCallback, pContext, pEnv);
    }

    EckInline NTSTATUS Create(PTP_WORK_CALLBACK pfnCallback, void* pContext,
        CTpEnvironment& Env) noexcept
    {
        return Create(pfnCallback, pContext, &Env.GetEnvironment());
    }

    // F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_WORK Work)
    template<class F, class... TArgs>
    EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args) noexcept
    {
        using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
        auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
        return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 1>{}),
            pParam.release(), pEnv);
    }

    EckInline void Close() noexcept
    {
        if (m_pWork)
        {
            TpReleaseWork(m_pWork);
            m_pWork = nullptr;
        }
    }

    EckInline void Submit() noexcept { TpPostWork(m_pWork); }

    EckInline void Wait(BOOL bCancelPending = FALSE) noexcept
    {
        TpWaitForWork(m_pWork, (LOGICAL)bCancelPending);
    }

    EckInlineNdCe TP_WORK* GetWork() const noexcept { return m_pWork; }
};

class CTpWait
{
private:
    TP_WAIT* m_pWait{};

    template<class TTuple, size_t... Idx>
    static void CALLBACK EckWaitCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
        PTP_WAIT Wait, TP_WAIT_RESULT Result) noexcept
    {
        std::unique_ptr<TTuple> pParam((TTuple*)(Context));
        std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Wait, Result);
    }

    template<class TTuple, size_t... Idx>
    static auto MakeCallback(std::index_sequence<Idx...>) noexcept
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
        TP_CALLBACK_ENVIRON* pEnv = nullptr) noexcept
    {
        Close();
        return TpAllocWait(&m_pWait, pfnCallback, pContext, pEnv);
    }

    EckInline NTSTATUS Create(PTP_WAIT_CALLBACK pfnCallback, void* pContext,
        CTpEnvironment& Env) noexcept
    {
        return Create(pfnCallback, pContext, &Env.GetEnvironment());
    }

    // F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_WAIT Wait, TP_WAIT_RESULT Result)
    template<class F, class... TArgs>
    EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args) noexcept
    {
        using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
        auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
        return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 2>{}),
            pParam.release(), pEnv);
    }

    EckInline void Close() noexcept
    {
        if (m_pWait)
        {
            TpReleaseWait(m_pWait);
            m_pWait = nullptr;
        }
    }

    EckInline void SetWait(HANDLE hObj, LONGLONG llTimeout) noexcept
    {
        TpSetWait(m_pWait, hObj, (LARGE_INTEGER*)&llTimeout);
    }

    EckInline void SetWait(HANDLE hObj, LARGE_INTEGER liTimeout) noexcept
    {
        TpSetWait(m_pWait, hObj, &liTimeout);
    }

    EckInline void SetWait(HANDLE hObj) noexcept
    {
        TpSetWait(m_pWait, hObj, nullptr);
    }
#if NTDDI_VERSION >= NTDDI_WIN8
    EckInline BOOL SetWaitEx(HANDLE hObj, LONGLONG llTimeout) noexcept
    {
        return SetThreadpoolWaitEx(m_pWait, hObj, (FILETIME*)&llTimeout, nullptr);
    }

    EckInline BOOL SetWaitEx(HANDLE hObj, LARGE_INTEGER liTimeout) noexcept
    {
        return SetThreadpoolWaitEx(m_pWait, hObj, (FILETIME*)&liTimeout, nullptr);
    }

    EckInline BOOL SetWaitEx(HANDLE hObj) noexcept
    {
        return SetThreadpoolWaitEx(m_pWait, hObj, nullptr, nullptr);
    }
#endif // NTDDI_VERSION >= NTDDI_WIN8
    EckInline void Wait(BOOL bCancelPending = FALSE) noexcept
    {
        TpWaitForWait(m_pWait, (LOGICAL)bCancelPending);
    }

    EckInlineNdCe TP_WAIT* GetWait() const noexcept { return m_pWait; }
};

class CTpTimer
{
private:
    TP_TIMER* m_pTimer{};

    template<class TTuple, size_t... Idx>
    static void CALLBACK EckTimerCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context,
        PTP_TIMER Timer) noexcept
    {
        std::unique_ptr<TTuple> pParam((TTuple*)(Context));
        std::invoke(std::move(std::get<Idx>(*pParam))..., Instance, Timer);
    }

    template<class TTuple, size_t... Idx>
    static auto MakeCallback(std::index_sequence<Idx...>) noexcept
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
        TP_CALLBACK_ENVIRON* pEnv = nullptr) noexcept
    {
        Close();
        return TpAllocTimer(&m_pTimer, pfnCallback, pContext, pEnv);
    }

    EckInline NTSTATUS Create(PTP_TIMER_CALLBACK pfnCallback, void* pContext,
        CTpEnvironment& Env) noexcept
    {
        return Create(pfnCallback, pContext, &Env.GetEnvironment());
    }

    // F = void()(..., PTP_CALLBACK_INSTANCE Instance, PTP_TIMER Timer)
    template<class F, class... TArgs>
    EckInline NTSTATUS Create2(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args) noexcept
    {
        using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
        auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);
        return Create(MakeCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs) + 1>{}),
            pParam.release(), pEnv);
    }

    EckInline void Close() noexcept
    {
        if (m_pTimer)
        {
            TpReleaseTimer(m_pTimer);
            m_pTimer = nullptr;
        }
    }

    EckInline void SetTimer2(LARGE_INTEGER* pliDueTime, UINT msPeriod, UINT msAdjust = 0) noexcept
    {
        TpSetTimer(m_pTimer, pliDueTime, msPeriod, msAdjust);
    }

    EckInline void SetTimer(LONGLONG llDueTime, UINT msPeriod, UINT msAdjust = 0) noexcept
    {
        SetTimer2((LARGE_INTEGER*)&llDueTime, msPeriod, msAdjust);
    }

    EckInline BOOL IsSet() const noexcept { return TpIsTimerSet(m_pTimer); }

    EckInline void Wait(BOOL bCancelPending = FALSE) noexcept
    {
        TpWaitForTimer(m_pTimer, (LOGICAL)bCancelPending);
    }

    EckInlineNdCe TP_TIMER* GetTimer() const noexcept { return m_pTimer; }
};

namespace Priv
{
    template<class TTuple, size_t... Idx>
    void CALLBACK EckTpSimpleCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context) noexcept
    {
        std::unique_ptr<TTuple> pParam(reinterpret_cast<TTuple*>(Context));
        auto callable = std::move(std::get<0>(*pParam));
        std::invoke(std::move(callable), std::move(std::get<Idx + 1>(*pParam))..., Instance);
    }

    template<class TTuple, size_t... Idx>
    auto MakeTpSimpleCallback(std::index_sequence<Idx...>) noexcept
    {
        return &EckTpSimpleCallback<TTuple, Idx...>;
    }
}

// F = void()(..., PTP_CALLBACK_INSTANCE Instance)
template<class F, class... TArgs>
EckInline NTSTATUS TpSubmitSimpleCallback(TP_CALLBACK_ENVIRON* pEnv, F&& Fn, TArgs&&... Args) noexcept
{
    using TTuple = std::tuple<std::decay_t<F>, std::decay_t<TArgs>...>;
    auto pParam = std::make_unique<TTuple>(std::forward<F>(Fn), std::forward<TArgs>(Args)...);

    return TpSimpleTryPost(
        Priv::MakeTpSimpleCallback<TTuple>(std::make_index_sequence<sizeof...(TArgs)>{}),
        pParam.release(), pEnv);
}
ECK_NAMESPACE_END