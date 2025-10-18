#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template <ccpIsComInterface T>
class ComPtr
{
    template<ccpIsComInterface U>
    friend class ComPtr;
public:
    using TInterface = T;
private:
    TInterface* p{};

    EckInline void ReleaseIt() noexcept
    {
        if (p)
        {
            TInterface* t{};
            std::swap(p, t);
            t->Release();
        }
    }

    EckInline ULONG ReleaseItRet() noexcept
    {
        if (p)
        {
            TInterface* t{};
            std::swap(p, t);
            return t->Release();
        }
        else
            return 0;
    }
public:
    ComPtr() = default;

    constexpr ComPtr(std::nullptr_t) noexcept {}

    template<ccpIsComInterface U>
        requires std::is_convertible_v<U*, TInterface*>
    ComPtr(U* x) noexcept :p{ x }
    {
        if (p)
            p->AddRef();
    }

    ComPtr(const ComPtr& x) noexcept : p{ x.p }
    {
        if (p)
            p->AddRef();
    }

    constexpr ComPtr(ComPtr&& x) noexcept { std::swap(p, x.p); }

    template<ccpIsComInterface U>
        requires std::is_convertible_v<U*, TInterface*>
    ComPtr(const ComPtr<U>& x) noexcept : p{ x.p }
    {
        if (p)
            p->AddRef();
    }

    template<ccpIsComInterface U>
        requires std::is_convertible_v<U*, TInterface*>
    constexpr ComPtr(ComPtr<U>&& x) noexcept
    {
        std::swap(p, x.p);
    }

    ComPtr(REFCLSID clsid, HRESULT* phr = nullptr)
    {
        const auto hr = CoCreateInstance(clsid, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p));
        if (phr) *phr = hr;
    }

    ~ComPtr() noexcept { ReleaseIt(); }

    ComPtr& operator=(std::nullptr_t) noexcept
    {
        ReleaseIt();
        return *this;
    }

    template<ccpIsComInterface U>
        requires std::is_convertible_v<U*, TInterface*>
    ComPtr& operator=(U* x) noexcept
    {
        if (x != p)
            ComPtr(x).Swap(*this);
        return *this;
    }

    ComPtr& operator=(const ComPtr& x) noexcept
    {
        if (p != x.p)
            ComPtr(x).Swap(*this);
        return *this;
    }

    constexpr ComPtr& operator=(ComPtr&& x) noexcept
    {
        std::swap(p, x.p);
        return *this;
    }

    template<ccpIsComInterface U>
        requires std::is_convertible_v<U*, TInterface*>
    ComPtr& operator=(const ComPtr<U>& x) noexcept
    {
        ComPtr(x).Swap(*this);
        return *this;
    }

    EckInlineNdCe TInterface* const* operator&() const noexcept { return &p; }
    EckInlineNdCe TInterface** operator&() noexcept { return &p; }

    EckInlineNdCe TInterface* Get() const noexcept { return p; }
    EckInlineNdCe TInterface* operator->() const noexcept { return p; }

    EckInlineNdCe TInterface* const* AddrOf() const noexcept { return &p; }
    EckInlineNdCe TInterface** AddrOf() noexcept { return &p; }
    EckInline [[nodiscard]] TInterface** AddrOfClear() noexcept
    {
        ReleaseIt();
        return &p;
    }

    EckInlineNdCe TInterface*& RefOf() noexcept { return p; }
    EckInlineNdCe TInterface*& RefOfClear() noexcept
    {
        ReleaseIt();
        return p;
    }

    EckInlineNdCe TInterface* Detach() noexcept
    {
        TInterface* t{};
        std::swap(p, t);
        return t;
    }
    EckInline void Attach(TInterface* x) noexcept
    {
        if (p != nullptr)
        {
#ifdef _DEBUG
            EckAssert(p->Release() || p != x);
#else
            p->Release();
#endif
        }
        p = x;
    }

    EckInline ULONG Clear() noexcept { return ReleaseItRet(); }

    template<ccpIsComInterface U>
    EckInline HRESULT As(ComPtr<U>& x) const noexcept
    {
        return p->QueryInterface(__uuidof(U), (void**)x.AddrOfClear());
    }
    EckInline HRESULT As(REFIID riid, ComPtr<IUnknown>& x) const noexcept
    {
        return p->QueryInterface(riid, (void**)x.AddrOfClear());
    }
    template<ccpIsComInterface U>
    EckInline HRESULT As(U*& x) const noexcept
    {
        return p->QueryInterface(&x);
    }

    EckInlineCe void Swap(ComPtr& x) noexcept { std::swap(p, x.p); }
    EckInlineCe void Swap(ComPtr&& x) noexcept { std::swap(p, x.p); }

    HRESULT CreateInstance(REFCLSID clsid) noexcept
    {
        ReleaseIt();
        return CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&p));
    }
};
ECK_NAMESPACE_END