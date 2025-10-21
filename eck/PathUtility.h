#pragma once
#include "StringUtility.h"

ECK_NAMESPACE_BEGIN
constexpr std::wstring_view
IllegalPathCharW{ LR"(\/:*?"<>|)" },
IllegalPathCharWithDotW{ LR"(\/:*?"<>|.)" };
constexpr std::string_view
IllegalPathCharA{ R"(\/:*?"<>|)" },
IllegalPathCharWithDotA{ R"(\/:*?"<>|.)" };

inline void PazLegalize(_In_z_ ccpIsStdCharPtr auto pszPath,
    ccpIsStdChar auto chReplace = '_', BOOL bReplaceDot = FALSE)
{
    if constexpr (std::is_same_v<RemoveStdCharPtr_T<decltype(pszPath)>, char>)
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharA : IllegalPathCharWithDotA };
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
    else
    {
        const auto IllegalChars{ bReplaceDot ? IllegalPathCharW : IllegalPathCharWithDotW };
        auto p{ pszPath };
        while (p = TcsChrFirstOf(p, IllegalChars.data()))
            *p++ = chReplace;
    }
}
ECK_NAMESPACE_END