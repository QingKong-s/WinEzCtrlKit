#pragma once
#include "CNtObject.h"

ECK_NAMESPACE_BEGIN
class CWaitableObject : public CNtObject {};

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LONGLONG llMilliseconds) noexcept
{
    return NtWaitForSingleObject(Object.Get(), FALSE, (LARGE_INTEGER*)&llMilliseconds);
}

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LARGE_INTEGER* pliMilliseconds = nullptr) noexcept
{
    return NtWaitForSingleObject(Object.Get(), FALSE, pliMilliseconds);
}
ECK_NAMESPACE_END