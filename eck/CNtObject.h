#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CNtObject
{
protected:
    HANDLE m_hObject{};
public:
    CNtObject() = default;
    CNtObject(HANDLE h) noexcept
    {
        NtDuplicateObject(NtCurrentProcess(), h,
            NtCurrentProcess(), &m_hObject, 0, 0,
            DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
    }
    CNtObject(CNtObject& x) noexcept : CNtObject{ x.Get() } {}
    CNtObject(CNtObject&& x) noexcept { std::swap(m_hObject, x.m_hObject); }
    CNtObject& operator=(CNtObject& x) noexcept
    {
        if (&x == this)
            return *this;
        NtDuplicateObject(NtCurrentProcess(), x.Get(),
            NtCurrentProcess(), &m_hObject, 0, 0,
            DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
        return *this;
    }
    CNtObject& operator=(CNtObject&& x) noexcept { std::swap(m_hObject, x.m_hObject); }
    ~CNtObject()
    {
        if (m_hObject && m_hObject != INVALID_HANDLE_VALUE)
            NtClose(m_hObject);
    }

    EckInlineNdCe HANDLE Attach(HANDLE h) noexcept { return std::exchange(m_hObject, h); }
    EckInlineNdCe HANDLE Detach() noexcept { return std::exchange(m_hObject, nullptr); }
    EckInlineNdCe HANDLE Get() const noexcept { return m_hObject; }
    EckInline void Clear() noexcept
    {
        if (m_hObject && m_hObject != INVALID_HANDLE_VALUE)
        {
            NtClose(m_hObject);
            m_hObject = nullptr;
        }
    }
    EckInlineNdCe BOOL IsValid() const noexcept { return m_hObject && m_hObject != INVALID_HANDLE_VALUE; }
};
ECK_NAMESPACE_END