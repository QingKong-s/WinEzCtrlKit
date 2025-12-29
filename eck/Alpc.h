#pragma once
#include "ECK.h"

ECK_NAMESPACE_BEGIN
constexpr inline ALPC_PORT_ATTRIBUTES AlpcDefaultPortAttributes
{
    .Flags = 0,
    .SecurityQos = {
        .Length = sizeof(SECURITY_QUALITY_OF_SERVICE),
        .ImpersonationLevel = SecurityAnonymous,
        .ContextTrackingMode = SECURITY_DYNAMIC_TRACKING,
        .EffectiveOnly = FALSE,
    },
    .MaxMessageLength = 256,
    .MemoryBandwidth = 0,
    .MaxPoolUsage = 0x2000,
    .MaxSectionSize = 0x2000,
    .MaxViewSize = 0,
    .MaxTotalSectionSize = 0x10000,
    .DupObjectTypes = 0,
};
ECK_NAMESPACE_END