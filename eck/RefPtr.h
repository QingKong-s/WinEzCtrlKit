#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template <class T>
class RefPtr
{
private:
    struct ControlBlock
    {
        std::atomic<UINT> cRef;
        T Data;

        template <class... Ts>
        ControlBlock(Ts&&... Args)
            noexcept(std::is_nothrow_constructible<T, Ts...>::value)
            : cRef{ 1 }, Data{ std::forward<Ts>(Args)... }
        {}
    };

    ControlBlock* p{};

    explicit constexpr RefPtr(ControlBlock* p_) noexcept : p{ p_ } {}

    void Reference() noexcept
    {
        if (p)
            p->cRef.fetch_add(1, std::memory_order_relaxed);
    }
public:
    template <class... Ts>
    static RefPtr Make(Ts&&... Args)
        noexcept(std::is_nothrow_constructible<T, Ts...>::value)
    {
        return RefPtr{ new ControlBlock(std::forward<Ts>(Args)...) };
    }

    RefPtr() = default;
    ~RefPtr() { Clear(); }
    RefPtr(const RefPtr& x) noexcept : p{ x.p } { Reference(); }

    RefPtr(RefPtr&& x) noexcept { std::swap(p, x.p); }

    RefPtr& operator=(const RefPtr& x) noexcept
    {
        if (this == &x)
            return *this;
        Clear();
        p = x.p;
        Reference();
        return *this;
    }

    RefPtr& operator=(RefPtr&& x) noexcept
    {
        std::swap(p, x.p);
        return *this;
    }

    T& operator*() const noexcept { return p->Data; }
    T* operator->() const noexcept { return &p->Data; }
    T* Get() const noexcept { return p ? &p->Data : nullptr; }

    UINT GetReferenceCount() const noexcept
    {
        return p ? p->cRef.load(std::memory_order_relaxed) : 0;
    }

    void Clear() noexcept
    {
        if (p)
        {
            if (p->cRef.fetch_sub(1, std::memory_order_acq_rel) == 1)
                delete p;
            p = nullptr;
        }
    }
};
ECK_NAMESPACE_END