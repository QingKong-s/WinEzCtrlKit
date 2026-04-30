#pragma once
#include "CWindow.h"

ECK_NAMESPACE_BEGIN
template<class T>
class Observable
{
public:
    using FValueChanged = void(*)(const T&, void*);
private:
    std::variant<T, T*> m_Var{};
    FValueChanged m_pfnCallback{};
    void* m_pCtx{};
public:
    Observable() = default;
    Observable(T&& x) noexcept { AsValue(std::forward<T>(x)); }
    Observable(T* x) noexcept { AsReference(x); }

    void AsReference(T* x) noexcept
    {
        m_Var = x;
        OnValueChanged();
    }
    void AsReference(T& x) noexcept { AsReference(&x); }

    void AsValue(T&& x) noexcept
    {
        m_Var = std::forward<T>(x);
        OnValueChanged();
    }
    void AsValue() noexcept
    {
        m_Var.emplace<T>();
        OnValueChanged();
    }

    void Set(T&& x) noexcept
    {
        Get() = std::forward<T>(x);
        OnValueChanged();
    }

    Observable& operator=(T&& x) noexcept
    {
        Set(std::forward<T>(x));
        return *this;
    }

    void OnValueChanged() noexcept
    {
        if (m_pfnCallback)
            m_pfnCallback(Get(), m_pCtx);
    }

    EckInlineCe void SetCallback(FValueChanged pfn, void* pCtx) noexcept
    {
        m_pfnCallback = pfn;
        m_pCtx = pCtx;
    }
    EckInlineCe void ClearCallback() noexcept
    {
        m_pfnCallback = nullptr;
        m_pCtx = nullptr;
    }

    EckInlineNdCe T& Get() noexcept
    {
        if (std::holds_alternative<T*>(m_Var))
            return *std::get<T*>(m_Var);
        else
            return std::get<T>(m_Var);
    }
    EckInlineNdCe T& Get() const noexcept
    {
        if (std::holds_alternative<T*>(m_Var))
            return *std::get<T*>(m_Var);
        else
            return std::get<T>(m_Var);
    }
};

template<class F>
class DdxCommand
{
private:
    std::function<F> m_Fn{};
public:
    DdxCommand() = default;

    template<class F1>
    DdxCommand(F1&& Fn) noexcept : m_Fn{ std::forward<F1>(Fn) } {}

    void Call(auto&&... Args) const
    {
        if (m_Fn)
            m_Fn(std::forward<decltype(Args)>(Args)...);
    }

    template<class F1>
    void Set(F1&& Fn) noexcept
    {
        m_Fn = std::forward<F1>(Fn);
    }
};

template<class TExtra>
class CDdxControlCollection
{
private:
    struct CTRL
    {
        HWND hWnd;
        TExtra Extra;
    };
    std::vector<CTRL> m_vCtrl{};

    auto LowerBound(HWND hWnd) const noexcept
    {
        return std::lower_bound(m_vCtrl.begin(), m_vCtrl.end(), hWnd,
            [](const CTRL& x, HWND h) { return x.hWnd < h; });
    }
    auto LowerBound(HWND hWnd) noexcept
    {
        return std::lower_bound(m_vCtrl.begin(), m_vCtrl.end(), hWnd,
            [](const CTRL& x, HWND h) { return x.hWnd < h; });
    }
public:
    void Add(HWND hWnd, const TExtra& Extra) noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
            it->Extra = Extra;
        else
            m_vCtrl.emplace(it, CTRL{ hWnd, Extra });
    }
    BOOL Remove(HWND hWnd) noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
        {
            m_vCtrl.erase(it);
            return TRUE;
        }
        return FALSE;
    }
    const TExtra* At(HWND hWnd) const noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
            return &it->Extra;
        else
            return nullptr;
    }
};

template<class Fn, USHORT Id, class TExtra>
    requires std::derived_from<Fn, CDdxControlCollection<TExtra>>
EckInlineNd CWindow::HSlot DdxpConnect(CWindow& Ctrl, CWindow& Parent, const TExtra& Extra) noexcept
{
    auto& ec = Parent.GetEventChain();
    auto hSlot = ec.FindSlot(Id);
    if (!hSlot)
        hSlot = ec.Connect(Fn{}, Id);
    ec.GetFunctionTarget<Fn>(hSlot)->Add(Ctrl.Handle, Extra);
    return hSlot;
}
template<class Fn, USHORT Id>
EckInlineNd BOOL DdxpDisconnect(CWindow& Ctrl, CWindow& Parent) noexcept
{
    auto& ec = Parent.GetEventChain();
    const auto hSlot = ec.FindSlot(Id);
    if (hSlot)
        return ec.GetFunctionTarget<Fn>(hSlot)->Remove(Ctrl.Handle);
    return FALSE;
}
ECK_NAMESPACE_END