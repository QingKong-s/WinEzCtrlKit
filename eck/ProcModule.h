#pragma once
#include "CRefStr.h"

ECK_NAMESPACE_BEGIN
struct CLoaderLockGuard
{
    CLoaderLockGuard() { RtlEnterCriticalSection(NtCurrentPeb()->LoaderLock); }
    ~CLoaderLockGuard() { RtlLeaveCriticalSection(NtCurrentPeb()->LoaderLock); }
};

inline LDR_DATA_TABLE_ENTRY* FindModuleEntry(
    void* pBase,
    std::wstring_view svModuleName = {},
    std::wstring_view svModulePath = {}) noexcept
{
    EckAssert(RtlIsCriticalSectionLocked(NtCurrentPeb()->LoaderLock));
    const auto pHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    for (auto p = pHead->Flink; p != pHead; p = p->Flink)
    {
        // InLoadOrderModuleList刚好指向LDR_DATA_TABLE_ENTRY中的第一个字段
        const auto pEntry = (LDR_DATA_TABLE_ENTRY*)p;
        if ((!pBase || pEntry->DllBase == pBase) &&
            (!svModuleName.empty() || TcsIsStartWithLen2I(
                pEntry->BaseDllName.Buffer, pEntry->BaseDllName.Length / sizeof(WCHAR),
                svModuleName.data(), svModuleName.size())) &&
            (!svModulePath.empty() || TcsIsStartWithLen2I(
                pEntry->FullDllName.Buffer, pEntry->FullDllName.Length / sizeof(WCHAR),
                svModulePath.data(), svModulePath.size())))
        {
            return pEntry;
        }
    }
    return nullptr;
}

inline BOOL GetModuleFile(void* pBase, eck::CRefStrW& rsFileName) noexcept
{
    CLoaderLockGuard _{};
    const auto pEntry = FindModuleEntry(pBase);
    if (pEntry)
    {
        rsFileName.PushBack(pEntry->FullDllName.Buffer,
            pEntry->FullDllName.Length / sizeof(WCHAR));
        return TRUE;
    }
    return FALSE;
}
ECK_NAMESPACE_END