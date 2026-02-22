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

class CDdxControlCollection
{
private:
    struct CTRL
    {
        HWND hWnd;
        void* pObservable;
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
    void Add(HWND hWnd, void* pObservable) noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
            it->pObservable = pObservable;
        else
            m_vCtrl.emplace(it, CTRL{ hWnd, pObservable });
    }
    void Remove(HWND hWnd) noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
            m_vCtrl.erase(it);
    }
    void* At(HWND hWnd) const noexcept
    {
        const auto it = LowerBound(hWnd);
        if (it != m_vCtrl.end() && it->hWnd == hWnd)
            return it->pObservable;
        else
            return nullptr;
    }
};

template<class Fn, USHORT Id, class T>
    requires std::derived_from<Fn, CDdxControlCollection>
EckInlineNd CWindow::HSlot DdxpConnectSlot(CWindow& Parent, CWindow& Ctrl, Observable<T>& o) noexcept
{
    auto& ec = Parent.GetEventChain();
    auto hSlot = ec.FindSlot(Id);
    if (!hSlot)
        hSlot = ec.Connect(Fn{}, Id);

    Fn* const p = ec.GetFunctionTarget<Fn>(hSlot);
    p->Add(Ctrl.HWnd, &o);
    return hSlot;
}
ECK_NAMESPACE_END