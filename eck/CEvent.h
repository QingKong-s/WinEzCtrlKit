#pragma once
#include "CWaitableObject.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
class CEvent : public CWaitableObject
{
public:
    CEvent()
    {
        NtCreateEvent(&m_hObject, EVENT_ALL_ACCESS, nullptr, SynchronizationEvent, FALSE);
    }
    CEvent(std::wstring_view svName, BOOL bManualReset, BOOL bInheritHandle = FALSE)
    {
        const auto us{ StringViewToNtString(svName) };
        OBJECT_ATTRIBUTES oa;
        InitializeObjectAttributes(&oa,
            (svName.empty() ? nullptr : &us),
            (bInheritHandle ? OBJ_INHERIT : 0),
            nullptr, nullptr);
        NtCreateEvent(&m_hObject, EVENT_ALL_ACCESS, &oa,
            bManualReset ? NotificationEvent : SynchronizationEvent, FALSE);
    }

    EckInline NTSTATUS Signal(LONG* plPrevState = nullptr) { return NtSetEvent(m_hObject, plPrevState); }
    EckInline NTSTATUS NoSignal() { return NtClearEvent(m_hObject); }
    EckInline NTSTATUS NoSignal(LONG* plPrevState) { return NtResetEvent(m_hObject, plPrevState); }
};
ECK_NAMESPACE_END