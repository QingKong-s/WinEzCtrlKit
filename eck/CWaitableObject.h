#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CWaitableObject
{
protected:
	HANDLE m_hObj{};
public:
	ECK_DISABLE_COPY_DEF_CONS(CWaitableObject);
	constexpr explicit CWaitableObject(HANDLE hObj) : m_hObj{ hObj } {}

	~CWaitableObject() { NtClose(m_hObj); }

	constexpr CWaitableObject(CWaitableObject&& x) noexcept: m_hObj{ x.m_hObj }
	{
		x.m_hObj = nullptr;
	}

	constexpr CWaitableObject& operator=(CWaitableObject&& x) noexcept
	{
		std::swap(m_hObj, x.m_hObj);
		return *this;
	}

	EckInline [[nodiscard]] constexpr HANDLE GetHandle() const { return m_hObj; }

	EckInline [[nodiscard]] constexpr HANDLE Detach() { return std::exchange(m_hObj, nullptr); }

	EckInline constexpr HANDLE Attach(HANDLE hObj) { return std::exchange(m_hObj, hObj); }
};

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LONGLONG llMilliseconds)
{
	return NtWaitForSingleObject(Object.GetHandle(), FALSE, (LARGE_INTEGER*)&llMilliseconds);
}

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LARGE_INTEGER* pliMilliseconds = nullptr)
{
	return NtWaitForSingleObject(Object.GetHandle(), FALSE, pliMilliseconds);
}
ECK_NAMESPACE_END