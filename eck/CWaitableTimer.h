#pragma once
#include "CWaitableObject.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CWaitableTimer : public CWaitableObject
{
public:
    CWaitableTimer()
    {
        OBJECT_ATTRIBUTES oa;
        InitObjAttr(oa);
        NtCreateTimer(&m_hObject, TIMER_ALL_ACCESS, &oa, SynchronizationTimer);
    }

    CWaitableTimer(std::wstring_view svName, BOOL bManualReset = FALSE, BOOL bInheritHandle = FALSE)
    {
        const auto us{ StringViewToNtString(svName) };
        OBJECT_ATTRIBUTES oa;
        InitObjAttr(oa, (svName.empty() ? nullptr : &us), (bInheritHandle ? OBJ_INHERIT : 0));
        NtCreateTimer(&m_hObject, TIMER_ALL_ACCESS, &oa,
            (bManualReset ? NotificationTimer : SynchronizationTimer));
    }

    EckInline NTSTATUS SetDueTime100ns(LONGLONG ll)
    {
        ll *= -1;
        return NtSetTimer(m_hObject, (LARGE_INTEGER*)&ll,
            nullptr, nullptr, FALSE, 0, nullptr);
    }

    EckInline NTSTATUS SetDueTimeAndPeriod100ns(LONGLONG ll, LONG lPeriod)
    {
        ll *= -1;
        return NtSetTimer(m_hObject, (LARGE_INTEGER*)&ll,
            nullptr, nullptr, FALSE, lPeriod, nullptr);
    }

    EckInline NTSTATUS SetDueTime(LONG ll)
    {
        return SetDueTime100ns(ll * 10000ll);
    }

    EckInline NTSTATUS SetDueTimeAndPeriod(LONG ll, LONG lPeriod)
    {
        return SetDueTimeAndPeriod100ns(ll * 10000ll, lPeriod);
    }

    EckInline NTSTATUS Cancel(BOOLEAN* pbPrevState = nullptr)
    {
        return NtCancelTimer(m_hObject, pbPrevState);
    }

    NTSTATUS QueryState(_Out_ BOOLEAN& bState, _Out_ LONGLONG& llRemainingTime)
    {
        TIMER_BASIC_INFORMATION	tbi;
        ULONG cbRet;
        const auto nts = NtQueryTimer(m_hObject, TimerBasicInformation,
            &tbi, sizeof(tbi), &cbRet);
        if (NT_SUCCESS(nts))
        {
            bState = tbi.TimerState;
            llRemainingTime = tbi.RemainingTime.QuadPart;
        }
        else
        {
            bState = FALSE;
            llRemainingTime = 0;
        }
        return nts;
    }
};
ECK_NAMESPACE_END