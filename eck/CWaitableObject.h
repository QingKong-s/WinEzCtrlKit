#pragma once
#include "CNtObject.h"

ECK_NAMESPACE_BEGIN
class CWaitableObject : public CNtObject {};

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LONGLONG llMilliseconds)
{
    return NtWaitForSingleObject(Object.Get(), FALSE, (LARGE_INTEGER*)&llMilliseconds);
}

EckInline NTSTATUS WaitObject(const CWaitableObject& Object, LARGE_INTEGER* pliMilliseconds = nullptr)
{
    return NtWaitForSingleObject(Object.Get(), FALSE, pliMilliseconds);
}
ECK_NAMESPACE_END