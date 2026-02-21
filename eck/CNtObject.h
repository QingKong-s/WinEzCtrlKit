#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
class CNtObject
{
protected:
    HANDLE m_hObject{};
public:
    CNtObject() = default;
    explicit constexpr CNtObject(HANDLE h) noexcept : m_hObject{ h } {}
    CNtObject(const CNtObject& x) noexcept
    {
        CloneFrom(x.Get());
    }
    CNtObject(CNtObject&& x) noexcept { std::swap(m_hObject, x.m_hObject); }
    CNtObject& operator=(const CNtObject& x) noexcept
    {
        if (&x == this)
            return *this;
        CloneFrom(x.Get());
        return *this;
    }
    CNtObject& operator=(CNtObject&& x) noexcept { std::swap(m_hObject, x.m_hObject); }
    ~CNtObject()
    {
        if (m_hObject && m_hObject != INVALID_HANDLE_VALUE)
            NtClose(m_hObject);
    }

    EckInlineNdCe void Attach(HANDLE h) noexcept
    {
        if (h != m_hObject)
        {
            Clear();
            m_hObject = h;
        }
    }
    EckInlineNdCe HANDLE Detach() noexcept { return std::exchange(m_hObject, nullptr); }
    NTSTATUS CloneFrom(HANDLE h) noexcept
    {
        Clear();
        return NtDuplicateObject(NtCurrentProcess(), h,
            NtCurrentProcess(), &m_hObject, 0, 0,
            DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES);
    }
    EckInlineNdCe HANDLE Get() const noexcept { return m_hObject; }
    EckInline void Clear() noexcept
    {
        if (m_hObject && m_hObject != INVALID_HANDLE_VALUE)
        {
            NtClose(m_hObject);
            m_hObject = nullptr;
        }
    }
    EckInlineNdCe BOOL IsValid() const noexcept
    {
        return m_hObject && m_hObject != INVALID_HANDLE_VALUE;
    }

    EckInlineNdCe HANDLE* AddrOf() noexcept { return &m_hObject; }
    EckInlineNd HANDLE* AddrOfClear() noexcept
    {
        Clear();
        return &m_hObject;
    }
};
ECK_NAMESPACE_END