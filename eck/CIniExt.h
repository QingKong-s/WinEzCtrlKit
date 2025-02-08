#pragma once
#include "AutoPtrDef.h"
#include "CRefStr.h"

#include <map>

ECK_NAMESPACE_BEGIN
using TChar_ = WCHAR;
constexpr inline bool IsOrderedMap = false;
constexpr inline bool IsAllowMultiKeys = false;
constexpr inline bool IsCaseSensitive = false;
class CIniExtMut
{
public:
	enum class Result
	{
		Ok,						// 成功
		SecRBracketNotFound,	// 节右括号"]"未找到
		SecEmptyName,			// 节名为空
		SecIllegalChar,			// 方括号周围存在非法字符
		SecContainerNotMatch,	// 容器未闭合
		SecDuplicate,			// 节名重复
		KvSepNotFound,			// 键值分隔符"="未找到
		KvEmptyKey,				// 键名为空
	};

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

	enum : UINT
	{
		EF_NONE = 0,
		EF_HAS_COMMENTS = 1u << 0,	// 条目后面存在注释
		EF_IS_CONTAINER = 1u << 1,	// 容器（仅用于节）
	};

	enum : UINT
	{
		IF_NONE = 0,				// 无特殊选项
		IF_IGNORE_COMMENTS = 1u << 0,	// 待解析内容中没有注释
		IF_DISABLE_EXT = 1u << 1,	// 禁用扩展语法
		IF_ESCAPE = 1u << 2,		// 启用转义
		IF_KEEP_SPACE = 1u << 3,	// 保留等号周围的空白符
	};

	struct Entry
	{
		TStr rsName{};		// 【禁止外部修改】名称，对于节，为节名，对于键值对，为键名
		UINT Id{};			// 【禁止外部修改】ID
		UINT uFlags{};		// EF_常量
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

	template<class TBase>
	struct CtxBase
	{
		BOOL bValid{};

		EckInlineNd auto operator->() const noexcept { return &(static_cast<const TBase*>(this)->it->second); }
		EckInlineNd auto& operator*() const noexcept { return static_cast<const TBase*>(this)->it->second; }

		EckInlineNdCe BOOL IsValid() const noexcept { return bValid; }
		EckInlineNdCe operator BOOL() const noexcept { return IsValid(); }
	};

	struct CtxSec : CtxBase<CtxSec>
	{
		TSecIter it{};
	};
	struct CtxSecCst : CtxBase<CtxSecCst>
	{
		TSecCstIter it{};
	};
	struct CtxKv : CtxBase<CtxKv>
	{
		TKvIter it{};
	};
	struct CtxKvCst : CtxBase<CtxKvCst>
	{
		TKvCstIter it{};
	};
private:
	TSectionMap m_Root{};
	TChar m_szBreakLine[3]{ '\r','\n','\0' };
	BYTE m_cchBreakLine{ 2 };
	std::vector<Comment> m_vComment{};// uFlags和rsComment无效
	UINT m_uId{};

	constexpr static BOOL EscapeChar(TChar& ch)
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

	EckInlineNdCe static BOOL IsBreakLineChar(TChar ch)
	{
		return ch == '\n' || ch == '\r';
	}

	EckInlineNdCe static BOOL IsSpaceChar(TChar ch)
	{
		return IsBreakLineChar(ch) || ch == '\t' || ch == ' ' || ch == '\0';
	}

	EckInlineNdCe static BOOL IsCommentChar(TChar ch)
	{
		return ch == ';';
	}

	EckInlineNdCe static BOOL IsSpaceOrCommentChar(TChar ch)
	{
		return IsSpaceChar(ch) || ch == ';';
	}

	EckInlineNdCe static BOOL IsBreakLineOrCommentChar(TChar ch)
	{
		return ch == '\n' || ch == '\r' || ch == ';';
	}

	static Result EscapeSectionName(TStr& rs, const TChar*& psz, size_t cch)
	{
		rs.Clear();
		const auto pszEnd = psz + cch;
		for (; psz != pszEnd; ++psz)
		{
			const auto ch = *psz;
			if (ch == '\\')// 转义字符
			{
				auto ch2 = *psz++;
				if (EscapeChar(ch2))
					rs.PushBackChar(ch2);
				else
					rs.PushBackChar(ch).PushBackChar(ch2);
			}
			else if (ch == ']')// 节末尾
			{
				if (psz + 1 < pszEnd && !IsBreakLineOrCommentChar(*(psz + 1)))
					return Result::SecIllegalChar;
				return Result::Ok;
			}
			else// 常规字符
				rs.PushBackChar(ch);
		}
		return Result::SecRBracketNotFound;
	}

	static Result ScanSectionName(TStr& rs, const TChar*& psz, size_t cch)
	{
		const auto pR = TcsCharLen(psz, cch, ']');
		if (pR)
		{
			if (pR + 1 < psz + cch && !IsBreakLineOrCommentChar(*(pR + 1)))
				return Result::SecIllegalChar;
			rs.DupString(psz, int(pR - psz));
			psz = pR + 1;
			return Result::Ok;
		}
		else
			return Result::SecRBracketNotFound;
	}

	static Result EscapeKeyValue(TStr& rsKey, TStr& rsVal,
		const TChar*& psz, size_t cch, BOOL bKeepSpace)
	{
		const auto pOrg = psz;
		const auto pszEnd = psz + cch;
		// 键
		for (; psz != pszEnd; ++psz)
		{
			const auto ch = *psz;
			if (ch == '\\')// 转义字符
			{
				auto ch2 = *psz++;
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
		if (psz == pszEnd)
			return Result::KvSepNotFound;
		if (!bKeepSpace)
		{
			rsKey.RTrim();
			if (const auto p = LTrimStr(psz, int(pszEnd - psz)); p)
				psz = p;
		}
		// 值
		for (; psz != pszEnd; ++psz)
		{
			const auto ch = *psz;
			if (ch == '\\')// 转义字符
			{
				auto ch2 = *psz++;
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
		return Result::Ok;
	}

	static Result ScanKeyValue(TStr& rsKey, TStr& rsVal,
		const TChar*& psz, size_t cch, BOOL bKeepSpace)
	{
		const auto pOrg = psz;
		auto pR = TcsCharLen(psz, cch, '=');
		if (pR)
		{
			rsKey.DupString(psz, int(pR - psz));
			if (!bKeepSpace)
				rsKey.RTrim();
			psz = pR + 1;
		}
		else
			return Result::SecRBracketNotFound;
		if (!bKeepSpace)
			psz = LTrimStr(psz, int(psz - pOrg));
		constexpr static TChar ValEnd[]{ ';','\n','\r' };
		pR = TcsChrFirstOf(psz, cch, EckArrAndLen(ValEnd));
		if (!pR)// 若找不到值结束符，则包括到结尾
			pR = psz + cch;
		rsVal.DupString(psz, int(pR - psz));
		psz = pR;
		return Result::Ok;
	}

	static void ScanComments(TStr& rs, const TChar*& psz, size_t cch)
	{
		constexpr static TChar ValEnd[]{ '\n','\r' };
		auto pR = TcsChrFirstOf(psz, cch, EckArrAndLen(ValEnd));
		if (!pR)
			pR = psz + cch;
		rs.PushBack(psz, int(pR - psz));
		psz = pR;
	}

	static auto EmplacePair(auto Ret)
	{
		if constexpr (IsAllowMultiKeys)
			return std::make_pair(Ret, true);
		else
			return Ret;
	}

	static auto EmplaceIter(auto Ret)
	{
		if constexpr (IsAllowMultiKeys)
			return Ret;
		else
			return Ret.first;
	}

	void ForEntry(Section& Sec, auto&& Fn)
	{
		for (auto& Val : Sec.Val)
			Fn(Val.second);
		for (auto& Child : Sec.Child)
		{
			Fn(Child.second);
			ForEntry(Child.second, Fn);
		}
	}

	void ForEntry(auto&& Fn)
	{
		for (auto& Sec : m_Root)
		{
			Fn(Sec.second);
			ForEntry(Sec.second, Fn);
		}
		for (auto& Comment : m_vComment)
			Fn(Comment);
	}

	void OnCreateEntry(UINT Id)
	{
		ForEntry([this, Id](Entry& e)
			{
				if (e.Id >= Id)
					e.Id += 1;
			});
	}

	void OnDeleteEntry(UINT Id)
	{
		ForEntry([this, Id](Entry& e)
			{
				if (e.Id > Id)
					e.Id -= 1;
			});
		--m_uId;
	}


	CtxSec IntSetSection(TSectionMap& Map, const CtxSec& Sec, TStrView svName)
	{
		auto New{ std::move(*Sec) };
		Map.erase(Sec.it);
		New.rsName.DupString(svName);
		const auto sv = New.rsName.ToStringView();
		return { TRUE,EmplaceIter(Map.emplace(sv, std::move(New))) };
	}

	CtxSec IntCreateSection(TSectionMap& Map, UINT Id, TStrView svName, UINT uFlags)
	{
		TStr rsName{};
		if (svName.size() < TStr::LocalBufferSize)
			rsName.Reserve(TStr::EnsureNotLocalBufferSize);
		rsName.DupString(svName);
		const auto sv = rsName.ToStringView();
		if (Id != UINT_MAX)
			OnCreateEntry(Id);
		else
			Id = m_uId++;
		return { TRUE,EmplaceIter(Map.emplace(sv, Section{
			std::move(rsName),
			Id,
			uFlags,
			})) };
	}

	void IntDeleteSection(TSectionMap& Map, const CtxSec& Sec)
	{
		OnDeleteEntry(Sec.it->second.Id);
		Map.erase(Sec.it);
	}


	CtxKv IntSetKeyName(TValueMap& Map, const CtxKv& Kv, TStrView svName)
	{
		auto New{ std::move(*Kv) };
		Map.erase(Kv.it);
		New.rsName.DupString(svName);
		const auto sv = New.rsName.ToStringView();
		return { TRUE,EmplaceIter(Map.emplace(sv, std::move(New))) };
	}

	CtxKv IntCreateKeyValue(TValueMap& Map, UINT Id,
		TStrView svName, TStrView svValue, UINT uFlags)
	{
		TStr rsName{};
		if (svName.size() < TStr::LocalBufferSize)
			rsName.Reserve(TStr::EnsureNotLocalBufferSize);
		rsName.DupString(svName);
		if (Id != UINT_MAX)
			OnCreateEntry(Id);
		else
			Id = m_uId++;
		const auto sv = rsName.ToStringView();
		return { TRUE,EmplaceIter(Map.emplace(sv, Value{
			std::move(rsName),
			Id,
			uFlags,
			{},
			svValue })) };
	}

	void IntDeleteKeyValue(TValueMap& Map, TKvIter it)
	{
		OnDeleteEntry(it->second.Id);
		Map.erase(it);
	}
public:
	Result Load(const TChar* pszIni, size_t cchIni = -1, UINT uFlags = IF_NONE)
	{
		m_Root.clear();
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

		const BOOL bKeepSpace = (uFlags & IF_KEEP_SPACE);

		State eState{ State::Section };
		TStr rsComment{};
		std::vector<ITEM> stSec{};// 栈顶为当前节
		stSec.reserve(16);
		stSec.emplace_back();// dummy
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
					pLastEntry->uFlags |= EF_HAS_COMMENTS;
				}
				pLastEntry = nullptr;
				continue;
			}
			else if (IsSpaceChar(ch))
				continue;
			if (ch == ';')// 注释
			{
				if (!rsComment.IsEmpty())
					rsComment.PushBack(m_szBreakLine, m_cchBreakLine);
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
						return Result::SecRBracketNotFound;
					BOOL bContainer;
					if (*pszIni == '>')
					{
						++pszIni;
						bContainer = TRUE;
					}
					else
						bContainer = FALSE;
					TStr rsName{};
					const auto r = ((uFlags & IF_ESCAPE) ?
						EscapeSectionName(rsName, pszIni, pszEnd - pszIni) :
						ScanSectionName(rsName, pszIni, pszEnd - pszIni));
					if (r != Result::Ok)
						return r;
					if (rsName.IsEmpty())
						return Result::SecEmptyName;
					if (rsName.Front() == '<')// 闭合容器节
					{
						if (rsName.Size() > 1)// 如果不止"<"一个字符，则校验当前栈顶
						{
							if (rsName.SubStringView(1, rsName.Size() - 1) !=
								stSec.back().it->first)
								return Result::SecContainerNotMatch;
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
					const UINT uFlags = (bContainer ? EF_IS_CONTAINER : EF_NONE) |
						(rsComment.IsEmpty() ? EF_NONE : EF_HAS_COMMENTS);
					const auto Ret = EmplacePair(Map.emplace(
						sv,
						Section{
							std::move(rsName),
							m_uId++,
							uFlags,
						}));
					if (bContainer && !Ret.second)
						return Result::SecDuplicate;
					stSec.emplace_back(Ret.first, bContainer);
					pLastEntry = &Ret.first->second;
					eState = State::Key;
				}
				else
					return Result::SecIllegalChar;
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
				const auto r = ((uFlags & IF_ESCAPE) ?
					EscapeKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace) :
					ScanKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace));
				if (r != Result::Ok)
					return r;
				if (rsKey.IsEmpty())
					return Result::KvEmptyKey;
				if (rsKey.IsLocal())
					rsKey.Reserve(24);
				EckAssert(!rsKey.IsLocal());
				auto& Map = stSec.back().it->second.Val;
				const auto sv = rsKey.ToStringView();
				const auto it = EmplaceIter(Map.emplace(sv, Value{
					std::move(rsKey),
					m_uId++,
					(rsComment.IsEmpty() ? EF_NONE : EF_HAS_COMMENTS),
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
		return Result::Ok;
	}

	EckInlineNd CtxSec GetSection(TStrView svName)
	{
		const auto it = m_Root.find(svName);
		return { it != m_Root.end(),it };
	}
	EckInlineNd CtxSecCst GetSection(TStrView svName) const
	{
		const auto it = m_Root.find(svName);
		return { it != m_Root.end(),it };
	}

	EckInlineNd CtxSec GetSection(const CtxSec& Sec, TStrView svName) const
	{
		const auto it = Sec->Child.find(svName);
		return { it != Sec->Child.end(),it };
	}
	EckInlineNd CtxSecCst GetSection(const CtxSecCst& Sec, TStrView svName) const
	{
		const auto it = Sec->Child.find(svName);
		return { it != Sec->Child.end(),it };
	}

	EckInline CtxSec SetSection(const CtxSec& Sec, TStrView svName) { return IntSetSection(m_Root, Sec, svName); }
	EckInline CtxSec SetSection(const CtxSec& Sec, const CtxSec& SecParent, TStrView svName)
	{
		return IntSetSection(SecParent->Child, Sec, svName);
	}

	EckInline CtxSec CreateSection(TStrView svName, UINT uFlags = EF_NONE, UINT Id = UINT_MAX)
	{
		return IntCreateSection(m_Root, Id, svName, uFlags);
	}
	EckInline CtxSec CreateSection(const CtxSec& SecParent, TStrView svName, UINT uFlags = EF_NONE, UINT Id = UINT_MAX)
	{
		return IntCreateSection(SecParent->Child, Id, svName, uFlags);
	}

	EckInline void DeleteSection(const CtxSec& Sec) { IntDeleteSection(m_Root, Sec); }
	EckInline void DeleteSection(const CtxSec& Sec, const CtxSec& SecParent)
	{
		IntDeleteSection(SecParent->Child, Sec);
	}

	EckInlineNd CtxKv GetKeyValue(const CtxSec& Sec, TStrView svName)
	{
		const auto it = Sec->Val.find(svName);
		return { it != Sec->Val.end(),it };
	}
	EckInlineNd CtxKvCst GetKeyValue(const CtxSecCst& Sec, TStrView svName) const
	{
		const auto it = Sec->Val.find(svName);
		return { it != Sec->Val.end(),it };
	}

	EckInlineNd CtxKv GetKeyValue(TStrView svSection, TStrView svName)
	{
		const auto Sec = GetSection(svSection);
		if (!Sec)
			return {};
		return GetKeyValue(Sec, svName);
	}
	EckInlineNd CtxKvCst GetKeyValue(TStrView svSection, TStrView svName) const
	{
		const auto Sec = GetSection(svSection);
		if (!Sec)
			return {};
		return GetKeyValue(Sec, svName);
	}

	EckInline CtxKv SetKeyName(const CtxKv& Kv, const CtxSec& Sec, TStrView svName)
	{
		return IntSetKeyName(Sec->Val, Kv, svName);
	}
	EckInline CtxKv SetKeyName(TStrView svName, TStrView svSection, TStrView svNewName)
	{
		auto Sec = GetSection(svSection);
		if (!Sec)
			Sec = CreateSection(svSection);
		auto Kv = GetKeyValue(Sec, svName);
		if (!Kv)
			return CreateKeyValue(Sec, svNewName, {});
		return SetKeyName(Kv, Sec, svNewName);
	}

	EckInline CtxKv CreateKeyValue(const CtxSec& Sec, TStrView svName, TStrView svValue, UINT uFlags = EF_NONE, UINT Id = UINT_MAX)
	{
		return IntCreateKeyValue(Sec->Val, Id, svName, svValue, uFlags);
	}

	EckInline void SetKeyValue(const CtxSec& Sec, TStrView svName, TStrView svValue, UINT uFlags = EF_NONE)
	{
		auto Kv = GetKeyValue(Sec, svName);
		if (!Kv)
			Kv = CreateKeyValue(Sec, svName, svValue, uFlags);
		Kv->rsValue.DupString(svValue);
	}

	EckInline void DeleteKeyValue(const CtxSec& Sec, TStrView svName)
	{
		const auto it = Sec->Val.find(svName);
		if (it != Sec->Val.end())
			IntDeleteKeyValue(Sec->Val, it);
	}
};
ECK_NAMESPACE_END