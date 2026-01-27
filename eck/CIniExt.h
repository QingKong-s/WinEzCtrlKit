#pragma once
#include "AutoPtrDef.h"
#include "CRefStr.h"
#include "StringConvert.h"

#include <map>

ECK_NAMESPACE_BEGIN
enum class IniResult
{
    Ok,						// 成功
    SecRBracketNotFound,	// 节右括号"]"未找到
    SecEmptyName,			// 节名为空
    SecIllegalChar,			// 方括号周围存在非法字符
    SecContainerNotMatch,	// 容器未闭合
    SecDuplicate,			// 节名重复
    KvSepNotFound,			// 键值分隔符"="未找到
    KvEmptyKey,				// 键名为空
    EscapeAtEnd,			// 转义字符"\"后面没有字符

    Max,
};

enum : UINT
{
    INIE_EF_NONE = 0,
    INIE_EF_HAS_COMMENTS = 1u << 0,	// 条目后面存在注释
    INIE_EF_IS_CONTAINER = 1u << 1,	// 容器（仅用于节）
};

enum : UINT
{
    INIE_IF_NONE = 0,					// 无特殊选项
    INIE_IF_IGNORE_COMMENTS = 1u << 0,	// 待解析内容中没有注释
    INIE_IF_DISABLE_EXT = 1u << 1,		// 禁用扩展语法
    INIE_IF_ESCAPE = 1u << 2,			// 启用转义
    INIE_IF_KEEP_SPACE = 1u << 3,		// 保留等号周围的空白符

    INIE_IF_EOL_BEFORE_SECTION = 1u << 4,		// 节之前有换行符
    INIE_IF_END_CONTAINER_WITH_NAME = 1u << 5,	// 容器结束时标注名称
};

using TChar_ = WCHAR;
constexpr inline bool IsOrderedMap = false;
constexpr inline bool IsAllowMultiKeys = false;
constexpr inline bool IsCaseSensitive = true;
class CIniExtMut
{
public:
    using TChar = TChar_;
    using TStrView = std::basic_string_view<TChar>;
    using TStr = CRefStrT<TChar>;

    struct FCmpCaseInsensitive
    {
        EckInlineNd bool operator()(TStrView x1, TStrView x2) const noexcept
        {
            return TcsCompareLen2I(x1.data(), x1.size(), x2.data(), x2.size()) < 0;
        }
    };
    using FCmpCaseSensitive = std::less<TStrView>;

    using FMapCmp = std::conditional_t<IsCaseSensitive,
        FCmpCaseSensitive,
        FCmpCaseInsensitive>;

    template<class K, class V>
    using TMap = std::conditional_t<IsAllowMultiKeys,
        std::multimap<K, V, FMapCmp>,
        std::map<K, V, FMapCmp>>;

    template<class K, class V>
    using TUnorderedMap = std::conditional_t<IsAllowMultiKeys,
        std::unordered_multimap<K, V>,
        std::unordered_map<K, V>>;

    struct Entry
    {
        TStr rsName{};		// 【禁止外部修改】名称，对于节，为节名，对于键值对，为键名
        UINT Id{};			// 【禁止外部修改】ID
        UINT uFlags{};		// INIE_EF_常量
        TStr rsComment{};	// 条目后方的注释

        constexpr std::strong_ordering operator<=>(const Entry& x) const noexcept
        {
            return Id <=> x.Id;
        }

        EckInlineNdCe auto& GetName() const noexcept { return rsName; }
        EckInlineNdCe auto GetId() const noexcept { return Id; }
    };

    struct Value : Entry
    {
        CRefStrT<TChar> rsValue{};
    };

    using TValueMap = std::conditional_t<IsOrderedMap || !IsCaseSensitive,
        TMap<TStrView, Value>,
        TUnorderedMap<TStrView, Value>>;

    struct Section;

    using TSectionMap = std::conditional_t<IsOrderedMap || !IsCaseSensitive,
        TMap<TStrView, Section>,
        TUnorderedMap<TStrView, Section>>;

    struct Section : Entry
    {
        TValueMap Val{};	// 【禁止外部修改】所有值
        TSectionMap Child{};// 【禁止外部修改】子节

        EckInline void ForEachKv(auto&& Fn)
        {
            for (auto& e : Val)
                Fn(e.second);
        }

        EckInline void ForEachChild(auto&& Fn)
        {
            for (auto& e : Child)
                Fn(e.second);
        }
    };

    struct Comment : Entry {};

    using TSecIter = typename TSectionMap::iterator;
    using TSecCstIter = typename TSectionMap::const_iterator;

    using TKvIter = typename TValueMap::iterator;
    using TKvCstIter = typename TValueMap::const_iterator;

    template<class TIter>
    struct CtxBase
    {
        TIter it;
        BOOL bInvalid;

        EckInlineNd auto GetIterator() const noexcept { return it; }

        EckInlineNd auto operator->() const noexcept { return &(GetIterator()->second); }
        EckInlineNd auto& operator*() const noexcept { return GetIterator()->second; }
        EckInlineNd auto& Data() const noexcept { return GetIterator()->second; }

        EckInlineNdCe BOOL IsValid() const noexcept { return !bInvalid; }
        EckInlineNdCe operator BOOL() const noexcept { return IsValid(); }
    };

    template<class TIter>
    struct CtxBaseKv : CtxBase<TIter>
    {
        EckInlineNd BOOL IsEmpty() const noexcept
        {
            return !this->IsValid() || this->Data().rsValue.IsEmpty();
        }

        EckInline void GetString(TStr& rs, TStrView svDef = {}) noexcept
        {
            if (IsEmpty())
                rs.Assign(svDef);
            else
                rs.Assign(this->Data().rsValue.Data(), this->Data().rsValue.Size());
        }

        template<std::integral T>
        T GetIntT(T nDef = 0, BOOL bHex = FALSE) noexcept
        {
            if (IsEmpty())
                return nDef;
            else
            {
                T i;
                TcsToInt(this->Data().rsValue.Data(),
                    this->Data().rsValue.Size(), i, bHex ? 16 : 0);
                return i;
            }
        }

        EckInline int GetInt(int nDef = 0, BOOL bHex = FALSE) noexcept
        {
            return GetIntT<int>(nDef, bHex);
        }
        EckInline INT64 GetInt64(INT64 nDef = 0, BOOL bHex = FALSE) noexcept
        {
            return GetIntT<INT64>(nDef, bHex);
        }

        EckInline double GetDouble(double dDef = 0.0) noexcept
        {
            if (IsEmpty())
                return dDef;
            else
                return _wtof(this->Data().rsValue.Data());
        }

        EckInline BOOL GetBool(BOOL bDef = FALSE) noexcept
        {
            if (IsEmpty())
                return bDef;
            else
            {
                const auto& rs = this->Data().rsValue;
                if (rs.CompareI(L"true", 4) == 0 || rs == L"1" || rs == L"真")
                    return TRUE;
                else
                    return FALSE;
            }
        }

        template<class T>
        EckInline T GetEnum(T Def = T{}) noexcept
        {
            if (IsEmpty())
                return Def;
            else
            {
                if (sizeof(T) > 4)
                    return (T)GetInt64();
                else
                    return (T)GetInt();
            }
        }

        template<class T>
        EckInline T GetEnumCheck(T Min, T MaxPlusOne, T Def = T{}) noexcept
        {
            using TUnderlying = UnderlyingType_T<T>;
            return (T)std::clamp<TUnderlying>(TUnderlying(GetEnum<T>(Def)),
                TUnderlying(Min), TUnderlying(MaxPlusOne) - 1);
        }
    };

    using CtxSec = CtxBase<TSecIter>;
    using CtxSecCst = CtxBase<TSecCstIter>;
    using CtxKv = CtxBaseKv<TKvIter>;
    using CtxKvCst = CtxBaseKv<TKvCstIter>;

private:
    TSectionMap m_Root{};
    std::vector<Comment> m_vComment{};// uFlags和rsComment无效
    EolType m_eEolType{ EolType::CRLF };
    UINT m_uId{};

    constexpr static BOOL EscapeChar(TChar& ch) noexcept
    {
        switch (ch)
        {
        case 'n':	ch = '\n';	break;
        case 'r':	ch = '\r';	break;
        case 't':	ch = '\t';	break;
        case '0':	ch = '\0';	break;
        case '\\':	break;
        case ';':	break;
        case '[':	break;
        case ']':	break;
        case '=':	break;
        default:	return FALSE;
        }
        return TRUE;
    }

    EckInlineNdCe static BOOL IsBreakLineChar(TChar ch) noexcept
    {
        return ch == '\n' || ch == '\r';
    }

    EckInlineNdCe static BOOL IsSpaceChar(TChar ch) noexcept
    {
        return IsBreakLineChar(ch) || ch == '\t' || ch == ' ' || ch == '\0';
    }

    EckInlineNdCe static BOOL IsCommentChar(TChar ch) noexcept
    {
        return ch == ';';
    }

    EckInlineNdCe static BOOL IsSpaceOrCommentChar(TChar ch) noexcept
    {
        return IsSpaceChar(ch) || ch == ';';
    }

    EckInlineNdCe static BOOL IsBreakLineOrCommentChar(TChar ch) noexcept
    {
        return ch == '\n' || ch == '\r' || ch == ';';
    }
    EckInlineNdCe static BOOL IsBomChar(TChar ch) noexcept
    {
        if constexpr (sizeof(TChar) == sizeof(WCHAR))
            return ch == 0xFEFF;
        else
            return ch == 0xFF || ch == 0xFE || ch == 0xEF;
    }
    EckInlineNdCe static BOOL IsSpaceCharHeader(TChar ch) noexcept
    {
        return IsSpaceChar(ch) || IsBomChar(ch);
    }

    // 调用前：psz指向[
    // 调用后：psz指向]的下一个位置
    static IniResult UnescapeSectionName(TStr& rs, const TChar*& psz, size_t cch) noexcept
    {
        const auto pszEnd = psz + cch;
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                if (psz + 1 >= pszEnd)
                    return IniResult::EscapeAtEnd;
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rs.PushBackChar(ch2);
                else
                    rs.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == ']')// 节末尾
            {
                if (psz + 1 < pszEnd && !IsBreakLineOrCommentChar(*(psz + 1)))
                    return IniResult::SecIllegalChar;
                ++psz;
                return IniResult::Ok;
            }
            else// 常规字符
                rs.PushBackChar(ch);
        }
        return IniResult::SecRBracketNotFound;
    }

    // 调用前：psz指向[
    // 调用后：psz指向]的下一个位置
    static IniResult ScanSectionName(TStr& rs, const TChar*& psz, size_t cch) noexcept
    {
        const auto pR = TcsCharLen(psz, cch, ']');
        if (pR)
        {
            if (pR + 1 < psz + cch && !IsBreakLineOrCommentChar(*(pR + 1)))
                return IniResult::SecIllegalChar;
            rs.Assign(psz, int(pR - psz));
            psz = pR + 1;
            return IniResult::Ok;
        }
        else
            return IniResult::SecRBracketNotFound;
    }

    // 调用前：psz指向键的第一个字符
    // 调用后：psz指向值最后一个字符的下一个位置
    static IniResult UnescapeKeyValue(TStr& rsKey, TStr& rsVal,
        const TChar*& psz, size_t cch, BOOL bKeepSpace) noexcept
    {
        const auto pOrg = psz;
        const auto pszEnd = psz + cch;
        // 键
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                if (psz + 1 >= pszEnd)
                    return IniResult::EscapeAtEnd;
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rsKey.PushBackChar(ch2);
                else
                    rsKey.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == '=')// 键末尾
                break;
            else// 常规字符
                rsKey.PushBackChar(ch);
        }
        if (*psz != '=')
            return IniResult::KvSepNotFound;
        ++psz;
        if (!bKeepSpace)
        {
            rsKey.RTrim();
            psz = LTrimStr(psz, int(pszEnd - psz));
        }
        // 值
        for (; psz != pszEnd; ++psz)
        {
            const auto ch = *psz;
            if (ch == '\\')// 转义字符
            {
                auto ch2 = *++psz;
                if (EscapeChar(ch2))
                    rsVal.PushBackChar(ch2);
                else
                    rsVal.PushBackChar(ch).PushBackChar(ch2);
            }
            else if (ch == ';' || ch == '\n' || ch == '\r')// 值末尾
                break;
            else// 常规字符
                rsVal.PushBackChar(ch);
        }
        return IniResult::Ok;
    }

    // 调用前：psz指向键的第一个字符
    // 调用后：psz指向值最后一个字符的下一个位置
    static IniResult ScanKeyValue(TStr& rsKey, TStr& rsVal,
        const TChar*& psz, size_t cch, BOOL bKeepSpace) noexcept
    {
        const auto pOrg = psz;
        auto pR = TcsCharLen(psz, cch, '=');
        if (pR)
        {
            rsKey.Assign(psz, int(pR - psz));
            psz = pR + 1;
            cch = cch - (psz - pOrg);
        }
        else
            return IniResult::SecRBracketNotFound;
        if (!bKeepSpace)
        {
            rsKey.RTrim();
            if (IsBreakLineOrCommentChar(*psz))
                return IniResult::Ok;
            const auto pL = LTrimStr(psz, (int)cch);
            cch -= (pL - psz);
            psz = pL;
        }

        constexpr static TChar ValEnd[]{ ';','\n','\r' };
        pR = TcsChrFirstOf(psz, cch, EckArrAndLen(ValEnd));
        if (!pR)// 若找不到值结束符，则包括到结尾
            pR = psz + cch;
        rsVal.Assign(psz, int(pR - psz));
        psz = pR;
        return IniResult::Ok;
    }

    static void ScanComments(TStr& rs, const TChar*& psz, size_t cch) noexcept
    {
        constexpr static TChar ValEnd[]{ '\n','\r' };
        auto pR = TcsChrFirstOf(psz, cch, EckArrAndLen(ValEnd));
        if (!pR)
            pR = psz + cch;
        rs.PushBack(psz, int(pR - psz));
        psz = pR;
    }

    // 转义svOrg，并尾插到rsOut中
    static void EscapeString(TStrView svOrg, TStr& rsOut) noexcept
    {
        for (const auto ch : svOrg)
        {
            switch (ch)
            {
            case '\n':	rsOut.PushBackChar('\\').PushBackChar('n');	break;
            case '\r':	rsOut.PushBackChar('\\').PushBackChar('r');	break;
            case '\t':	rsOut.PushBackChar('\\').PushBackChar('t');	break;
            case '\0':	rsOut.PushBackChar('\\').PushBackChar('0');	break;
            case '\\':	rsOut.PushBackChar('\\').PushBackChar('\\');	break;
            case ';':	rsOut.PushBackChar('\\').PushBackChar(';');	break;
            case '[':	rsOut.PushBackChar('\\').PushBackChar('[');	break;
            case ']':	rsOut.PushBackChar('\\').PushBackChar(']');	break;
            case '=':	rsOut.PushBackChar('\\').PushBackChar('=');	break;
            default:	rsOut.PushBackChar(ch);	break;
            }
        }
    }

    static auto EmplacePair(auto Ret) noexcept
    {
        if constexpr (IsAllowMultiKeys)
            return std::make_pair(Ret, true);
        else
            return Ret;
    }

    static auto EmplaceIterator(auto Ret) noexcept
    {
        if constexpr (IsAllowMultiKeys)
            return Ret;
        else
            return Ret.first;
    }

    void ForEachEntry(Section& Sec, auto&& Fn) noexcept
    {
        for (auto& Val : Sec.Val)
            Fn(Val.second);
        for (auto& Child : Sec.Child)
        {
            Fn(Child.second);
            ForEachEntry(Child.second, Fn);
        }
    }

    void ForEachEntry(auto&& Fn) noexcept
    {
        for (auto& Sec : m_Root)
        {
            Fn(Sec.second);
            ForEachEntry(Sec.second, Fn);
        }
        for (auto& Comment : m_vComment)
            Fn(Comment);
    }

    void OnCreateEntry(UINT Id, TStrView svName) noexcept
    {
        ForEachEntry([this, Id](Entry& e)
            {
                if (e.Id >= Id)
                    e.Id += 1;
            });
    }

    void OnDeleteEntry(UINT Id) noexcept
    {
        ForEachEntry([this, Id](Entry& e)
            {
                if (e.Id > Id)
                    e.Id -= 1;
            });
        --m_uId;
    }

    CtxSec IntSetSection(TSectionMap& Map, const CtxSec& Sec, TStrView svName) noexcept
    {
        auto New{ std::move(*Sec) };
        Map.erase(Sec.it);
        New.rsName.Assign(svName);
        const auto sv = New.rsName.ToStringView();
        return { EmplaceIterator(Map.emplace(sv, std::move(New))) };
    }

    CtxSec IntCreateSection(TSectionMap& Map, UINT Id, TStrView svName, UINT uFlags) noexcept
    {
        TStr rsName{};
        if (svName.size() < TStr::LocalBufferSize)
            rsName.Reserve(TStr::EnsureNotLocalBufferSize);
        rsName.Assign(svName);
        const auto sv = rsName.ToStringView();
        if (Id == UINT_MAX)
            Id = m_uId++;
        OnCreateEntry(Id, svName);
        return { EmplaceIterator(Map.emplace(sv, Section{
            std::move(rsName),
            Id,
            uFlags,
            })) };
    }

    void IntDeleteSection(TSectionMap& Map, const CtxSec& Sec) noexcept
    {
        OnDeleteEntry(Sec.it->second.Id);
        Map.erase(Sec.it);
    }


    CtxKv IntSetKeyName(TValueMap& Map, const CtxKv& Kv, TStrView svName) noexcept
    {
        auto New{ std::move(*Kv) };
        Map.erase(Kv.it);
        New.rsName.Assign(svName);
        const auto sv = New.rsName.ToStringView();
        return { EmplaceIterator(Map.emplace(sv, std::move(New))) };
    }

    CtxKv IntCreateKeyValue(TValueMap& Map, UINT Id,
        TStrView svName, TStrView svValue, UINT uFlags) noexcept
    {
        TStr rsName{};
        if (svName.size() < TStr::LocalBufferSize)
            rsName.Reserve(TStr::EnsureNotLocalBufferSize);
        rsName.Assign(svName);
        if (Id != UINT_MAX)
            Id = m_uId++;
        OnCreateEntry(Id, svName);
        const auto sv = rsName.ToStringView();
        return { EmplaceIterator(Map.emplace(sv, Value{
            std::move(rsName),
            Id,
            uFlags,
            {},
            svValue })) };
    }

    void IntDeleteKeyValue(TValueMap& Map, TKvIter it) noexcept
    {
        OnDeleteEntry(it->second.Id);
        Map.erase(it);
    }

    void PushBackEol(TStr& rs) const noexcept
    {
        switch (m_eEolType)
        {
        case EolType::CRLF:
            rs.PushBackChar('\r').PushBackChar('\n');
            break;
        case EolType::LF:
            rs.PushBackChar('\n');
            break;
        case EolType::CR:
            rs.PushBackChar('\r');
            break;
        }
    }
public:
    IniResult Load(const TChar* pszIni, size_t cchIni = -1, UINT uFlags = INIE_IF_NONE) noexcept
    {
        Clear();
        if (cchIni < 0)
            cchIni = TcsLen(pszIni);
        const auto pszEnd = pszIni + cchIni;
        enum class State
        {
            Section,
            Key,
        };

        struct ITEM
        {
            TSecIter it{};
            BOOL bContainer{};// 该节是否为容器
        };

        const BOOL bKeepSpace = (uFlags & INIE_IF_KEEP_SPACE);
        const BOOL bEscape = (uFlags & INIE_IF_ESCAPE);

        State eState{ State::Section };
        TStr rsComment{};
        std::vector<ITEM> stSec{};// 栈顶为当前节
        stSec.reserve(16);
        stSec.emplace_back();// Dummy
        Entry* pLastEntry{};
        while (pszIni < pszEnd)
        {
            const auto ch = *pszIni++;
            if (IsBreakLineChar(ch))// 到行尾时合并项目后面的注释
            {
                if (pLastEntry && !rsComment.IsEmpty())
                {
                    pLastEntry->rsComment = std::move(rsComment);
                    rsComment.Clear();
                    pLastEntry->uFlags |= INIE_EF_HAS_COMMENTS;
                }
                pLastEntry = nullptr;
                continue;
            }
            else if (IsSpaceChar(ch))
                continue;
            if (ch == ';')// 注释
            {
                if (!rsComment.IsEmpty())
                    PushBackEol(rsComment);
                ScanComments(rsComment, pszIni, pszEnd - pszIni);
                continue;
            }
            else// 到非注释字符时，合并不在项目后面的注释
            {
                if (!rsComment.IsEmpty())
                {
                    m_vComment.emplace_back(Comment{ std::move(rsComment), m_uId++ });
                    rsComment.Clear();
                }
            }

            switch (eState)
            {
            case State::Section:
            {
                if (ch == '[')
                {
                    if (pszIni >= pszEnd)
                        return IniResult::SecRBracketNotFound;
                    BOOL bContainer;
                    if (*pszIni == '>')
                    {
                        ++pszIni;
                        bContainer = TRUE;
                    }
                    else
                        bContainer = FALSE;
                    TStr rsName{};
                    const auto r = (bEscape ?
                        UnescapeSectionName(rsName, pszIni, pszEnd - pszIni) :
                        ScanSectionName(rsName, pszIni, pszEnd - pszIni));
                    if (r != IniResult::Ok)
                        return r;
                    if (rsName.IsEmpty())
                        return IniResult::SecEmptyName;
                    if (rsName.Front() == '<')// 闭合容器节
                    {
                        if (rsName.Size() > 1)// 如果不止"<"一个字符，则校验当前栈顶
                        {
                            if (rsName.SubStringView(1, rsName.Size() - 1) !=
                                stSec.back().it->first)
                                return IniResult::SecContainerNotMatch;
                        }
                        stSec.pop_back();
                        continue;
                    }
                    if (rsName.IsLocal())
                        rsName.Reserve(24);
                    if (!stSec.back().bContainer)
                        stSec.pop_back();
                    EckAssert(!rsName.IsLocal());
                    auto& Map = (stSec.empty() ? m_Root : stSec.back().it->second.Child);
                    const auto sv = rsName.ToStringView();
                    const UINT uFlags = (bContainer ? INIE_EF_IS_CONTAINER : INIE_EF_NONE) |
                        (rsComment.IsEmpty() ? INIE_EF_NONE : INIE_EF_HAS_COMMENTS);
                    const auto Ret = EmplacePair(Map.emplace(
                        sv,
                        Section{
                            std::move(rsName),
                            m_uId++,
                            uFlags,
                        }));
                    if (bContainer && !Ret.second)
                        return IniResult::SecDuplicate;
                    stSec.emplace_back(Ret.first, bContainer);
                    pLastEntry = &Ret.first->second;
                    eState = State::Key;
                }
                else if (!IsSpaceCharHeader(ch))
                    return IniResult::SecIllegalChar;
            }
            break;

            case State::Key:
            {
                --pszIni;
                if (ch == '[')
                {
                    eState = State::Section;
                    continue;
                }
                TStr rsKey{}, rsVal{};
                const auto r = (bEscape ?
                    UnescapeKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace) :
                    ScanKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace));
                if (r != IniResult::Ok)
                    return r;
                if (rsKey.IsEmpty())
                    return IniResult::KvEmptyKey;
                if (rsKey.IsLocal())
                    rsKey.Reserve(24);
                EckAssert(!rsKey.IsLocal());
                auto& Map = stSec.back().it->second.Val;
                const auto sv = rsKey.ToStringView();
                const auto it = EmplaceIterator(Map.emplace(sv, Value{
                    std::move(rsKey),
                    m_uId++,
                    (rsComment.IsEmpty() ? INIE_EF_NONE : INIE_EF_HAS_COMMENTS),
                    {},
                    std::move(rsVal) }));
                pLastEntry = &it->second;
            }
            break;
            }
        }
        if (!rsComment.IsEmpty())
        {
            m_vComment.emplace_back(Comment{ std::move(rsComment), m_uId++ });
            rsComment.Clear();
        }
        return IniResult::Ok;
    }

    IniResult Save(TStr& rsOut, UINT uFlags = INIE_IF_NONE) const noexcept
    {
        const auto bEscape = (uFlags & INIE_IF_ESCAPE);
        struct STACK
        {
            TSecCstIter it{};
            BOOL bExtended{};
        };
        std::vector<STACK> sSec{};
        sSec.reserve(m_uId);
        std::vector<TKvCstIter> vKv{};
        vKv.reserve(m_uId);
        struct STACK2
        {
            TStrView svName{};
            size_t cChild{};
        };
        std::vector<STACK2> sContainer{};
        TStr rsSpace{};

        auto Fn_PushBackSection = [&](const TSectionMap& Map)
            {
                if (Map.empty())
                    return;
                const BOOL bInsertingChild = sSec.empty() ? FALSE : sSec.back().bExtended;
                size_t i{ bInsertingChild ? sSec.size() - 1 : sSec.size() };
                sSec.resize(sSec.size() + Map.size());
                if (bInsertingChild)
                {
                    sSec.back() = sSec[i];
                    sSec[i].bExtended = FALSE;
                }
                const auto itSecBegin = sSec.begin() + i;
                for (auto it = Map.begin(); it != Map.end(); ++it, ++i)
                    sSec[i].it = it;
                const auto itSecEnd = sSec.end();
                std::sort(itSecBegin, itSecEnd, [](const STACK& x1, const STACK& x2)
                    {
                        return x1.it->second.Id > x2.it->second.Id;// 倒排序，便于后续弹出
                    });
            };

        Fn_PushBackSection(m_Root);
        while (!sSec.empty())
        {
            auto& e = sSec.back().it->second;
            if ((e.uFlags & INIE_EF_IS_CONTAINER) && !sSec.back().bExtended)
            {
                sContainer.emplace_back(sSec.back().it->first, e.Child.size() + 1);
                rsSpace.PushBackChar(' ').PushBackChar(' ');
                sSec.back().bExtended = TRUE;
                Fn_PushBackSection(e.Child);
            }
            else
            {
                rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                rsOut.PushBackChar('[');
                if (e.uFlags & INIE_EF_IS_CONTAINER)
                    rsOut.PushBackChar('>');
                if (bEscape)
                    EscapeString(e.rsName.ToStringView(), rsOut);
                else
                    rsOut.PushBack(e.rsName);
                rsOut.PushBackChar(']');
                PushBackEol(rsOut);
                for (auto it = e.Val.begin(); it != e.Val.end(); ++it)
                    vKv.emplace_back(it);
                std::sort(vKv.begin(), vKv.end(), [](const TKvCstIter& x1, const TKvCstIter& x2)
                    {
                        return x1->second.Id < x2->second.Id;
                    });
                for (const auto& f : vKv)
                {
                    rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                    if (bEscape)
                    {
                        EscapeString(f->second.rsName.ToStringView(), rsOut);
                        rsOut.PushBackChar('=');
                        EscapeString(f->second.rsValue.ToStringView(), rsOut);
                    }
                    else
                        rsOut
                        .PushBack(f->second.rsName)
                        .PushBackChar('=')
                        .PushBack(f->second.rsValue);
                    PushBackEol(rsOut);
                }
                vKv.clear();
                if (uFlags & INIE_IF_EOL_BEFORE_SECTION)
                    PushBackEol(rsOut);
                sSec.pop_back();

                while (!sContainer.empty() && --sContainer.back().cChild == 0)
                {
                    rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
                    rsOut.PushBackChar('[').PushBackChar('<');
                    if (uFlags & INIE_IF_END_CONTAINER_WITH_NAME)
                        if (bEscape)
                            EscapeString(sContainer.back().svName, rsOut);
                        else
                            rsOut.PushBack(sContainer.back().svName);
                    rsOut.PushBackChar(']');
                    PushBackEol(rsOut);
                    sContainer.pop_back();
                    rsSpace.PopBack().PopBack();
                }
            }
        }
        return IniResult::Ok;
    }

    EckInlineNd CtxSec GetSection(TStrView svName) noexcept
    {
        const auto it = m_Root.find(svName);
        return { it, it == m_Root.end() };
    }
    EckInlineNd CtxSecCst GetSection(TStrView svName) const noexcept
    {
        const auto it = m_Root.find(svName);
        return { it, it == m_Root.end() };
    }

    EckInlineNd CtxSec GetSection(const CtxSec& Sec, TStrView svName) const noexcept
    {
        const auto it = Sec->Child.find(svName);
        return { it, it == Sec->Child.end() };
    }
    EckInlineNd CtxSecCst GetSection(const CtxSecCst& Sec, TStrView svName) const noexcept
    {
        const auto it = Sec->Child.find(svName);
        return { it, it == Sec->Child.end() };
    }

    EckInline CtxSec SetSection(const CtxSec& Sec, TStrView svName) noexcept
    {
        return IntSetSection(m_Root, Sec, svName);
    }
    EckInline CtxSec SetSection(const CtxSec& Sec,
        const CtxSec& SecParent, TStrView svName) noexcept
    {
        return IntSetSection(SecParent->Child, Sec, svName);
    }

    EckInline CtxSec CreateSection(TStrView svName,
        UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX) noexcept
    {
        return IntCreateSection(m_Root, Id, svName, uFlags);
    }
    EckInline CtxSec CreateSection(const CtxSec& SecParent,
        TStrView svName, UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX) noexcept
    {
        return IntCreateSection(SecParent->Child, Id, svName, uFlags);
    }

    EckInline void DeleteSection(const CtxSec& Sec) noexcept { IntDeleteSection(m_Root, Sec); }
    EckInline void DeleteSection(const CtxSec& Sec, const CtxSec& SecParent) noexcept
    {
        IntDeleteSection(SecParent->Child, Sec);
    }

    EckInlineNd CtxKv GetKeyValue(const CtxSec& Sec, TStrView svName) noexcept
    {
        if (!Sec)
            return { TKvIter{},TRUE };
        const auto it = Sec->Val.find(svName);
        return { it, it == Sec->Val.end() };
    }
    EckInlineNd CtxKvCst GetKeyValue(const CtxSecCst& Sec, TStrView svName) const noexcept
    {
        if (!Sec)
            return { TKvCstIter{},TRUE };
        const auto it = Sec->Val.find(svName);
        return { it, it == Sec->Val.end() };
    }

    EckInlineNd CtxKv GetKeyValue(TStrView svSection, TStrView svName) noexcept
    {
        return GetKeyValue(GetSection(svSection), svName);
    }
    EckInlineNd CtxKvCst GetKeyValue(TStrView svSection, TStrView svName) const noexcept
    {
        return GetKeyValue(GetSection(svSection), svName);
    }

    EckInline CtxKv SetKeyName(const CtxKv& Kv, const CtxSec& Sec, TStrView svName) noexcept
    {
        return IntSetKeyName(Sec->Val, Kv, svName);
    }
    EckInline CtxKv SetKeyName(TStrView svName, TStrView svSection, TStrView svNewName) noexcept
    {
        auto Sec = GetSection(svSection);
        if (!Sec)
            Sec = CreateSection(svSection);
        auto Kv = GetKeyValue(Sec, svName);
        if (!Kv)
            return CreateKeyValue(Sec, svNewName, {});
        return SetKeyName(Kv, Sec, svNewName);
    }

    EckInline CtxKv CreateKeyValue(const CtxSec& Sec, TStrView svName,
        TStrView svValue, UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX) noexcept
    {
        return IntCreateKeyValue(Sec->Val, Id, svName, svValue, uFlags);
    }

    EckInline void SetKeyValue(const CtxSec& Sec,
        TStrView svName, TStrView svValue, UINT uFlags = INIE_EF_NONE) noexcept
    {
        auto Kv = GetKeyValue(Sec, svName);
        if (!Kv)
            Kv = CreateKeyValue(Sec, svName, svValue, uFlags);
        Kv->rsValue.Assign(svValue);
    }

    EckInline void DeleteKeyValue(const CtxSec& Sec, TStrView svName) noexcept
    {
        const auto it = Sec->Val.find(svName);
        if (it != Sec->Val.end())
            IntDeleteKeyValue(Sec->Val, it);
    }
private:
    void IntForEachSectionInOrder(std::vector<TSecIter>& vSec, TSectionMap& Map) noexcept
    {
        for (auto it = m_Root.begin(); it != m_Root.end(); ++it)
        {
            vSec[it->second.Id] = it;
            IntForEachSectionInOrder(vSec, it->second.Child);
        }
    }
public:
    void ForEachSectionInOrder(auto&& Fn, const CtxSec& Sec = {}) noexcept
    {
        std::vector<TSecIter> vSec{ m_uId };
        IntForEachSectionInOrder(vSec, Sec ? Sec->Child : m_Root);
        const auto itEnd = std::remove(vSec.begin(), vSec.end(), TSecIter{});
        for (auto it = vSec.begin(); it != itEnd; ++it)
            Fn(*it);
    }

    void ForEachValueInOrder(auto&& Fn, const CtxSec& Sec) noexcept
    {
        auto& Val = Sec->Val;
        std::vector<TKvIter> vVal{};
        vVal.reserve(Val.size());
        for (auto it = Val.begin(); it != Val.end(); ++it)
            vVal.emplace_back(it);
        std::sort(vVal.begin(), vVal.end(), [](const TKvIter& x1, const TKvIter& x2)
            {
                return x1->second.Id < x2->second.Id;
            });
        for (auto e : vVal)
            Fn(e);
    }

    EckInlineNd BOOL IsEmpty() const noexcept { return m_Root.empty(); }

    void Clear() noexcept
    {
        m_Root.clear();
        m_vComment.clear();
        m_uId = 0;
    }
};
ECK_NAMESPACE_END