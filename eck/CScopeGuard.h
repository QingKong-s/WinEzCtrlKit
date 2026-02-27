#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
template <class F>
class CScopeGuard
{
private:
    F m_Fn;
    BOOLEAN m_bActive;
public:
    explicit CScopeGuard(F&& f) noexcept
        : m_Fn{ std::move<F>(f) }, m_bActive{ TRUE } {}

    CScopeGuard(CScopeGuard&& x) noexcept
        : m_Fn{ std::move(x.m_Fn) }, m_bActive{ x.m_bActive }
    {
        x.m_bActive = FALSE;
    }
    CScopeGuard& operator=(CScopeGuard&& x) noexcept
    {
        if (&x == this)
            return *this;
        m_Fn = std::move(x.m_Fn);
        x.m_bActive = FALSE;
    }

    CScopeGuard(const CScopeGuard&) = delete;
    CScopeGuard& operator=(const CScopeGuard&) = delete;

    ~CScopeGuard()
    {
        if (m_bActive)
            m_Fn();
    }

    EckInlineCe void Cancel() noexcept { m_bActive = false; }
};
ECK_NAMESPACE_END