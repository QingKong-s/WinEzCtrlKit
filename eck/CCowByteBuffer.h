#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
using TDefaultCowByteBufferAllocator = std::allocator<BYTE>;

class CCowByteBuffer
{
private:
    struct ControlBlock
    {
        size_t cbCapacity;
        size_t cbSize;
        std::atomic<size_t> cRef;

        constexpr ControlBlock(size_t cbCapacity_, size_t cbSize_) noexcept
            : cbCapacity{ cbCapacity_ }, cbSize{ cbSize_ }, cRef{ 1 } {}

        EckInlineNdCe BYTE* Data() noexcept { return (BYTE*)(this + 1); }
        EckInlineNdCe PCBYTE Data() const noexcept { return (PCBYTE)(this + 1); }
    };

    ControlBlock* m_pBlock;
    [[no_unique_address]] TDefaultCowByteBufferAllocator m_Alloc{};

    ControlBlock* Allocate(size_t cbCapacity, size_t cbSize) noexcept
    {
        EckAssert(cbSize <= cbCapacity);
        cbCapacity = std::max(cbCapacity, (size_t)16);
        const auto p = m_Alloc.allocate(sizeof(ControlBlock) + cbCapacity);
        EckCheckMemory(p);
        return new (p) ControlBlock(cbCapacity, cbSize);
    }

    void Deallocate(ControlBlock* p) noexcept
    {
        if (p)
        {
            const auto cb = sizeof(ControlBlock) + p->cbCapacity;
            p->~ControlBlock();
            m_Alloc.deallocate((BYTE*)p, cb);
        }
    }

    void Dereference() noexcept
    {
        if (m_pBlock)
        {
            if (m_pBlock->cRef.fetch_sub(1, std::memory_order_acq_rel) == 1)
                Deallocate(m_pBlock);
            m_pBlock = nullptr;
        }
    }

    void PrepareWrite(size_t cbRequiredCapacity) noexcept
    {
        const auto bShared = (m_pBlock->cRef.load(std::memory_order_acquire) > 1);
        const auto bTooSmall = (m_pBlock->cbCapacity < cbRequiredCapacity);
        if (!bShared && !bTooSmall)
            return;
        const auto cbNew = std::max(m_pBlock->cbCapacity * 3 / 2, cbRequiredCapacity);
        const auto p = Allocate(cbNew, m_pBlock->cbSize);
        if (m_pBlock->cbSize > 0)
            memcpy(p->Data(), m_pBlock->Data(), m_pBlock->cbSize);
        Dereference();
        m_pBlock = p;
    }
public:
    CCowByteBuffer() = default;

    ~CCowByteBuffer()
    {
        Dereference();
    }

    CCowByteBuffer(const CCowByteBuffer& x) noexcept
        : m_pBlock(x.m_pBlock)
    {
        m_pBlock->cRef.fetch_add(1, std::memory_order_relaxed);
    }

    CCowByteBuffer& operator=(const CCowByteBuffer& x) noexcept
    {
        if (this != &x)
        {
            Dereference();
            m_pBlock = x.m_pBlock;
            m_pBlock->cRef.fetch_add(1, std::memory_order_relaxed);
        }
        return *this;
    }

    CCowByteBuffer(CCowByteBuffer&& x) noexcept
    {
        std::swap(m_pBlock, x.m_pBlock);
    }

    CCowByteBuffer& operator=(CCowByteBuffer&& x) noexcept
    {
        std::swap(m_pBlock, x.m_pBlock);
        return *this;
    }

    void Reserve(size_t cbCapacity) noexcept
    {
        if (cbCapacity <= Capacity())
            PrepareWrite(Capacity());
    }

    void ReSize(size_t cbSize) noexcept
    {
        PrepareWrite(cbSize);
        m_pBlock->cbSize = cbSize;
    }

    void PushBack(PCBYTE p, size_t cb) noexcept
    {
        PrepareWrite(m_pBlock->cbSize + cb);
        memcpy(m_pBlock->Data() + m_pBlock->cbSize, p, cb);
        m_pBlock->cbSize += cb;
    }

    EckInlineNdCe BYTE operator[](size_t i) const noexcept
    {
        EckAssert(i < m_pBlock->cbSize);
        return m_pBlock->Data()[i];
    }

    EckInlineNdCe PCBYTE Data() const noexcept { return m_pBlock->Data(); }
    EckInlineNdCe BYTE* LockForWrite() noexcept
    {
        PrepareWrite(m_pBlock->cbSize);
        return m_pBlock->Data();
    }

    EckInlineNdCe BOOL IsEmpty() const { return !m_pBlock->cbSize; }

    EckInlineNdCe size_t Size() const { return m_pBlock->cbSize; }
    EckInlineNdCe size_t Capacity() const { return m_pBlock->cbCapacity; }

    EckInlineNd size_t GetReferenceCount() const
    {
        return m_pBlock->cRef.load(std::memory_order_relaxed);
    }

    EckInlineNdCe std::span<const BYTE> ToSpan() const noexcept { return { Data(), Size() }; }
};
ECK_NAMESPACE_END