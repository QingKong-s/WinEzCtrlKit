#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
[[noreturn]] EckInline void Terminate() noexcept
{
    EckDbgBreak();
    std::terminate();
}

EckInline void CheckPointer(PCVOID p) noexcept
{
    if (!p)
        Terminate();
}
EckInline void CheckNtHandle(HANDLE h) noexcept
{
    if (!h || h == INVALID_HANDLE_VALUE)
        Terminate();
}
EckInline void CheckNtStatus(NTSTATUS s) noexcept
{
    if (!NT_SUCCESS(s))
        Terminate();
}
EckInline void CheckWin32Error(W32ERR e) noexcept
{
    if (e != ERROR_SUCCESS)
        Terminate();
}
EckInline void CheckHResult(HRESULT hr) noexcept
{
    if (FAILED(hr))
        Terminate();
}
EckInline void CheckBool(BOOL b) noexcept
{
    if (!b)
        Terminate();
}
ECK_NAMESPACE_END