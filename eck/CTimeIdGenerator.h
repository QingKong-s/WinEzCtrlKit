#pragma once
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CTimeIdGenerator
{
private:
    ULONGLONG m_ullLastTick{};
    ULONG m_Sequence{};
public:
    ULONGLONG Generate() noexcept
    {
        const auto ullNow = NtGetTickCount64();
        if (ullNow == m_ullLastTick)
            ++m_Sequence;
        else
        {
            m_ullLastTick = ullNow;
            m_Sequence = 0;
        }
        // 42位时间戳，22位序列号
        return (ullNow << 22) | GetLowNBits(m_Sequence, 22);
    }

    constexpr void Reset() noexcept
    {
        m_ullLastTick = 0;
        m_Sequence = 0;
    }
};
ECK_NAMESPACE_END