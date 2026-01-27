#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CHandleTable
{
private:
    RTL_HANDLE_TABLE m_HandleTable{};
public:
    CHandleTable() = default;
    CHandleTable(ULONG cMax, ULONG cbEntry) noexcept
    {
        // RtlIsValidHandle要求对齐到两个指针大小
        RtlInitializeHandleTable(cMax,
            (ULONG)AlignedSize(cbEntry + sizeof(RTL_HANDLE_TABLE_ENTRY), sizeof(void*) * 2),
            &m_HandleTable);
    }

    CHandleTable(const CHandleTable&) = delete;
    CHandleTable& operator=(const CHandleTable&) = delete;
    CHandleTable(CHandleTable&& x) noexcept : m_HandleTable{ x.m_HandleTable }
    {
        x.m_HandleTable = {};
    }
    CHandleTable& operator=(CHandleTable&& x) noexcept
    {
        if (this != &x)
        {
            RtlDestroyHandleTable(&m_HandleTable);
            m_HandleTable = x.m_HandleTable;
            x.m_HandleTable = {};
        }
    }

    ~CHandleTable()
    {
        RtlDestroyHandleTable(&m_HandleTable);
    }

    EckInlineNd RTL_HANDLE_TABLE_ENTRY* Allocate(_Out_opt_ ULONG* pidxHandle = nullptr) noexcept
    {
        return RtlAllocateHandle(&m_HandleTable, pidxHandle);
    }

    EckInline BOOL Free(RTL_HANDLE_TABLE_ENTRY* Handle) noexcept
    {
        return RtlFreeHandle(&m_HandleTable, Handle);
    }

    EckInlineNd BOOL IsValid(RTL_HANDLE_TABLE_ENTRY* Handle) noexcept
    {
        return RtlIsValidHandle(&m_HandleTable, Handle);
    }

    EckInline BOOL IsValidIndex(ULONG idxHandle, _Out_ RTL_HANDLE_TABLE_ENTRY** pHandle) noexcept
    {
        return RtlIsValidIndexHandle(&m_HandleTable, idxHandle, pHandle);
    }

    EckInline void Create(ULONG cMax, ULONG cbEntry) noexcept
    {
        Destroy();
        RtlInitializeHandleTable(cMax,
            (ULONG)AlignedSize(cbEntry + sizeof(RTL_HANDLE_TABLE_ENTRY), sizeof(void*) * 2),
            &m_HandleTable);
    }

    EckInline NTSTATUS Destroy() noexcept
    {
        const auto r = RtlDestroyHandleTable(&m_HandleTable);
        m_HandleTable = {};
        return r;
    }

    EckInlineNdCe auto& GetHandleTable() noexcept { return m_HandleTable; }
    EckInlineNdCe auto& GetHandleTable() const noexcept { return m_HandleTable; }
};

template<class T, class THandle = HANDLE>
class CTypedHandleTable
{
    static_assert(sizeof(THandle) == sizeof(void*), "THandle must be the same size as void*");
private:
    CHandleTable m_HandleTable;

    void DestructEntry() noexcept
    {
        auto& Ht = m_HandleTable.GetHandleTable();
        for (auto p = Ht.CommittedHandles;
            p < Ht.UnCommittedHandles;
            p += (Ht.SizeOfHandleTableEntry / sizeof(RTL_HANDLE_TABLE_ENTRY)))
        {
            if (p->Flags & RTL_HANDLE_ALLOCATED)
                std::destroy_at((T*)((BYTE*)p + sizeof(RTL_HANDLE_TABLE_ENTRY)));
        }
    }
public:
    CTypedHandleTable() = default;
    CTypedHandleTable(ULONG cMax) noexcept : m_HandleTable(cMax, sizeof(T)) {}

    ~CTypedHandleTable()
    {
        DestructEntry();
    }

    template<class... TArgs>
    EckInlineNd THandle Allocate(
        _Out_opt_ T** ppEntry = nullptr,
        _Out_opt_ ULONG* pidxHandle = nullptr,
        TArgs &&... Args) noexcept
    {
        const auto h = m_HandleTable.Allocate(pidxHandle);
        h->Flags |= RTL_HANDLE_ALLOCATED;
        const auto p = Validate((THandle)h);
        new (p) T{ std::forward<TArgs>(Args)... };
        if (ppEntry)
            *ppEntry = p;
        return h;
    }

    template<class... TArgs>
    EckInlineNd THandle Allocate2(TArgs &&... Args) noexcept
    {
        return Allocate(nullptr, nullptr, std::forward<TArgs>(Args)...);
    }

    EckInline BOOL Free(THandle Handle) noexcept
    {
        const auto p = Validate(Handle);
        std::destroy_at(p);
        ((RTL_HANDLE_TABLE_ENTRY*)Handle)->Flags &= ~RTL_HANDLE_ALLOCATED;
        return m_HandleTable.Free((RTL_HANDLE_TABLE_ENTRY*)Handle);
    }

    EckInlineNd BOOL IsValid(THandle Handle) noexcept
    {
        return m_HandleTable.IsValid((RTL_HANDLE_TABLE_ENTRY*)Handle);
    }

    EckInlineNd BOOL IsValidIndex(ULONG idxHandle, _Out_ T** pHandle) noexcept
    {
        RTL_HANDLE_TABLE_ENTRY* pEntry{};
        if (m_HandleTable.IsValidIndex(idxHandle, &pEntry))
        {
            *pHandle = (T*)pEntry;
            return TRUE;
        }
        return FALSE;
    }

    EckInlineNd T* Validate(THandle Handle) noexcept
    {
        EckAssert(IsValid(Handle));
        return (T*)((BYTE*)Handle + sizeof(RTL_HANDLE_TABLE_ENTRY));
    }

    EckInline void Create(ULONG cMax) noexcept
    {
        m_HandleTable.Create(cMax, sizeof(T));
    }

    EckInline NTSTATUS Destroy() noexcept
    {
        DestructEntry();
        return m_HandleTable.Destroy();
    }

    EckInlineNdCe auto& GetHandleTable() noexcept { return m_HandleTable; }
    EckInlineNdCe auto& GetHandleTable() const noexcept { return m_HandleTable; }
};
ECK_NAMESPACE_END