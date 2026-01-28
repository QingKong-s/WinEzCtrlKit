#pragma once
#include "ECK.h"

#include <Pdh.h>

ECK_NAMESPACE_BEGIN
class CProcessorUsage
{
private:
    PDH_HQUERY m_hQuery{};
    PDH_HCOUNTER m_hCounter{};
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CProcessorUsage);
    ~CProcessorUsage()
    {
        Close();
    }

    PDH_STATUS Create() noexcept
    {
        PDH_STATUS pdhs;
        pdhs = PdhOpenQueryW(nullptr, 0, &m_hQuery);
        if (pdhs != ERROR_SUCCESS)
            return pdhs;
        pdhs = PdhAddCounterW(m_hQuery, LR"(\Processor(_Total)\% Processor Time)", 0, &m_hCounter);
        if (pdhs != ERROR_SUCCESS)
        {
            PdhCloseQuery(m_hQuery);
            m_hQuery = nullptr;
            return pdhs;
        }
        return ERROR_SUCCESS;
    }

    EckInline PDH_STATUS Collect() noexcept
    {
        return PdhCollectQueryData(m_hQuery);
    }

    EckInline double GetUsage(PDH_STATUS* ppdhs = nullptr) noexcept
    {
        PDH_FMT_COUNTERVALUE Value{};
        PDH_STATUS pdhs = PdhGetFormattedCounterValue(m_hCounter, PDH_FMT_DOUBLE, nullptr, &Value);
        if (ppdhs)
            *ppdhs = pdhs;
        return Value.doubleValue;
    }

    EckInline PDH_STATUS GetUsage(UINT uFmt, PDH_FMT_COUNTERVALUE& Value) noexcept
    {
        return PdhGetFormattedCounterValue(m_hCounter, uFmt, nullptr, &Value);
    }

    void Close() noexcept
    {
        if (m_hCounter)
            PdhRemoveCounter(m_hCounter);
        if (m_hQuery)
            PdhCloseQuery(m_hQuery);
        m_hCounter = nullptr;
        m_hQuery = nullptr;
    }
};
ECK_NAMESPACE_END