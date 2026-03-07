#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
// 虚析构
class CReferenceCounted
{
private:
    std::atomic<UINT> m_cRef{ 1 };
public:
    virtual ~CReferenceCounted() = default;
    UINT Reference() noexcept { return ++m_cRef; }
    UINT Dereference() noexcept
    {
        const auto c = --m_cRef;
        if (!c)
            delete this;
        return c;
    }
    UINT GetReferenceCount() const noexcept { return m_cRef.load(std::memory_order_relaxed); }
};
// 非虚析构
template<class T>
class CReferenceCountedT
{
private:
    std::atomic<UINT> m_cRef{ 1 };
public:
    UINT Reference() noexcept { return ++m_cRef; }
    UINT Dereference() noexcept
    {
        const auto c = --m_cRef;
        if (!c)
            delete static_cast<T*>(this);
        return c;
    }
    UINT GetReferenceCount() const noexcept { return m_cRef.load(std::memory_order_relaxed); }
};

template<class T>
concept CcpRcObject =
std::is_base_of_v<CReferenceCounted, T> || std::is_base_of_v<CReferenceCountedT<T>, T>;

template<CcpRcObject T>
class RcPtr
{
private:
    T* p{};

    EckInline void ReleaseIt() noexcept
    {
        if (p)
        {
            p->Dereference();
            p = nullptr;
        }
    }

    constexpr RcPtr(std::nullptr_t, T* x) noexcept : p{ x } {}
public:
    template <class... Ts>
    static RcPtr Make(Ts&&... Args)
        noexcept(std::is_nothrow_constructible<T, Ts...>::value)
    {
        return RcPtr{ nullptr, new T{ std::forward<Ts>(Args)... } };
    }

    constexpr RcPtr() noexcept = default;
    constexpr RcPtr(std::nullptr_t) noexcept {}

    explicit RcPtr(T* x) noexcept : p{ x }
    {
        if (x)
            x->Reference();
    }

    RcPtr(const RcPtr& x) noexcept : p{ x.p }
    {
        if (p)
            p->Reference();
    }
    RcPtr(RcPtr&& x) noexcept { std::swap(p, x.p); }

    ~RcPtr() { ReleaseIt(); }

    RcPtr& operator=(const RcPtr& x) noexcept
    {
        if (p != x.p)
            RcPtr{ x }.Swap(*this);
        return *this;
    }
    RcPtr& operator=(T* x) noexcept
    {
        if (p != x)
            RcPtr{ x }.Swap(*this);
        return *this;
    }

    RcPtr& operator=(RcPtr&& x) noexcept
    {
        std::swap(x.p, p);
        return *this;
    }

    RcPtr& operator=(std::nullptr_t) noexcept
    {
        ReleaseIt();
        return *this;
    }

    UINT Clear() noexcept
    {
        if (p)
        {
            const auto r = p->Dereference();
            p = nullptr;
            return r;
        }
        return 0;
    }

    EckInline void Attach(T* x) noexcept
    {
        if (p && p != x)
            p->Dereference();
        p = x;
    }

    EckInlineNdCe T* Detach() noexcept
    {
        const auto t = p;
        p = nullptr;
        return t;
    }

    EckInlineNdCe T* Get() const noexcept { return p; }
    EckInlineNdCe T* operator->() const noexcept { return p; }
    EckInlineNdCe T& operator*() const noexcept { return *p; }

    EckInlineCe void Swap(RcPtr& x) noexcept { std::swap(p, x.p); }
    EckInlineCe void Swap(RcPtr&& x) noexcept { std::swap(p, x.p); }
};
ECK_NAMESPACE_END