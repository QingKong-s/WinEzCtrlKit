#pragma once
#include "AutoPtrDef.h"
#include "CRefStr.h"

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

		EckInlineNd auto GetIterator() const noexcept { return it; }

		EckInlineNd auto operator->() const noexcept { return &(GetIterator()->second); }
		EckInlineNd auto& operator*() const noexcept { return GetIterator()->second; }

		EckInlineNdCe BOOL IsValid() const noexcept { return GetIterator() != decltype(GetIterator()){}; }
		EckInlineNdCe operator BOOL() const noexcept { return IsValid(); }
	};

	using CtxSec = CtxBase<TSecIter>;
	using CtxSecCst = CtxBase<TSecCstIter>;
	using CtxKv = CtxBase<TKvIter>;
	using CtxKvCst = CtxBase<TKvCstIter>;

private:
	TSectionMap m_Root{};
	std::vector<Comment> m_vComment{};// uFlags和rsComment无效
	EolType m_eEolType{ EolType::CRLF };
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

	// 调用前：psz指向[
	// 调用后：psz指向]的下一个位置
	static IniResult EscapeSectionName(TStr& rs, const TChar*& psz, size_t cch)
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
	static IniResult ScanSectionName(TStr& rs, const TChar*& psz, size_t cch)
	{
		const auto pR = TcsCharLen(psz, cch, ']');
		if (pR)
		{
			if (pR + 1 < psz + cch && !IsBreakLineOrCommentChar(*(pR + 1)))
				return IniResult::SecIllegalChar;
			rs.DupString(psz, int(pR - psz));
			psz = pR + 1;
			return IniResult::Ok;
		}
		else
			return IniResult::SecRBracketNotFound;
	}

	// 调用前：psz指向键的第一个字符
	// 调用后：psz指向值最后一个字符的下一个位置
	static IniResult EscapeKeyValue(TStr& rsKey, TStr& rsVal,
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
		const TChar*& psz, size_t cch, BOOL bKeepSpace)
	{
		const auto pOrg = psz;
		auto pR = TcsCharLen(psz, cch, '=');
		if (pR)
		{
			rsKey.DupString(psz, int(pR - psz));
			psz = pR + 1;
		}
		else
			return IniResult::SecRBracketNotFound;
		if (!bKeepSpace)
		{
			rsKey.RTrim();
			if (IsBreakLineOrCommentChar(*psz))
				return IniResult::Ok;
			psz = LTrimStr(psz, int(cch - (psz - pOrg)));
		}

		constexpr static TChar ValEnd[]{ ';','\n','\r' };
		pR = TcsChrFirstOf(psz, cch, EckArrAndLen(ValEnd));
		if (!pR)// 若找不到值结束符，则包括到结尾
			pR = psz + cch;
		rsVal.DupString(psz, int(pR - psz));
		psz = pR;
		return IniResult::Ok;
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

	void OnCreateEntry(UINT Id, TStrView svName)
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
		return { EmplaceIter(Map.emplace(sv, std::move(New))) };
	}

	CtxSec IntCreateSection(TSectionMap& Map, UINT Id, TStrView svName, UINT uFlags)
	{
		TStr rsName{};
		if (svName.size() < TStr::LocalBufferSize)
			rsName.Reserve(TStr::EnsureNotLocalBufferSize);
		rsName.DupString(svName);
		const auto sv = rsName.ToStringView();
		if (Id == UINT_MAX)
			Id = m_uId++;
		OnCreateEntry(Id, svName);
		return { EmplaceIter(Map.emplace(sv, Section{
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
		return { EmplaceIter(Map.emplace(sv, std::move(New))) };
	}

	CtxKv IntCreateKeyValue(TValueMap& Map, UINT Id,
		TStrView svName, TStrView svValue, UINT uFlags)
	{
		TStr rsName{};
		if (svName.size() < TStr::LocalBufferSize)
			rsName.Reserve(TStr::EnsureNotLocalBufferSize);
		rsName.DupString(svName);
		if (Id != UINT_MAX)
			Id = m_uId++;
		OnCreateEntry(Id, svName);
		const auto sv = rsName.ToStringView();
		return { EmplaceIter(Map.emplace(sv, Value{
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

	void PushBackEol(TStr& rs) const
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
	IniResult Load(const TChar* pszIni, size_t cchIni = -1, UINT uFlags = INIE_IF_NONE)
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
						EscapeSectionName(rsName, pszIni, pszEnd - pszIni) :
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
				else
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
					EscapeKeyValue(rsKey, rsVal, pszIni, pszEnd - pszIni, bKeepSpace) :
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
				const auto it = EmplaceIter(Map.emplace(sv, Value{
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

	IniResult Save(TStr& rsOut, UINT uFlags = INIE_IF_NONE) const
	{
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

		auto PushBackSection = [&](const TSectionMap& Map)
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

		PushBackSection(m_Root);
		while (!sSec.empty())
		{
			auto& e = sSec.back().it->second;
			if ((e.uFlags & INIE_EF_IS_CONTAINER) && !sSec.back().bExtended)
			{
				sContainer.emplace_back(sSec.back().it->first, e.Child.size() + 1);
				rsSpace.PushBackChar(' ').PushBackChar(' ');
				sSec.back().bExtended = TRUE;
				PushBackSection(e.Child);
			}
			else
			{
				// TODO:转义
				rsOut.PushBack(rsSpace.Data(), rsSpace.Size() - 2);
				rsOut.PushBackChar('[');
				if (e.uFlags & INIE_EF_IS_CONTAINER)
					rsOut.PushBackChar('>');
				rsOut.PushBack(e.rsName).PushBackChar(']');
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

	EckInlineNd CtxSec GetSection(TStrView svName)
	{
		const auto it = m_Root.find(svName);
		return { it == m_Root.end() ? TSecIter{} : it };
	}
	EckInlineNd CtxSecCst GetSection(TStrView svName) const
	{
		const auto it = m_Root.find(svName);
		return { it == m_Root.end() ? TSecCstIter{} : it };
	}

	EckInlineNd CtxSec GetSection(const CtxSec& Sec, TStrView svName) const
	{
		const auto it = Sec->Child.find(svName);
		return { it == Sec->Child.end() ? TSecIter{} : it };
	}
	EckInlineNd CtxSecCst GetSection(const CtxSecCst& Sec, TStrView svName) const
	{
		const auto it = Sec->Child.find(svName);
		return { it == Sec->Child.end() ? TSecCstIter{} : it };
	}

	EckInline CtxSec SetSection(const CtxSec& Sec, TStrView svName) { return IntSetSection(m_Root, Sec, svName); }
	EckInline CtxSec SetSection(const CtxSec& Sec, const CtxSec& SecParent, TStrView svName)
	{
		return IntSetSection(SecParent->Child, Sec, svName);
	}

	EckInline CtxSec CreateSection(TStrView svName, UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX)
	{
		return IntCreateSection(m_Root, Id, svName, uFlags);
	}
	EckInline CtxSec CreateSection(const CtxSec& SecParent, TStrView svName, UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX)
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
		return { it == Sec->Val.end() ? TKvIter{} : it };
	}
	EckInlineNd CtxKvCst GetKeyValue(const CtxSecCst& Sec, TStrView svName) const
	{
		const auto it = Sec->Val.find(svName);
		return { it == Sec->Val.end() ? TKvCstIter{} : it };
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

	EckInline CtxKv CreateKeyValue(const CtxSec& Sec, TStrView svName, TStrView svValue, UINT uFlags = INIE_EF_NONE, UINT Id = UINT_MAX)
	{
		return IntCreateKeyValue(Sec->Val, Id, svName, svValue, uFlags);
	}

	EckInline void SetKeyValue(const CtxSec& Sec, TStrView svName, TStrView svValue, UINT uFlags = INIE_EF_NONE)
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
private:
	void ForSectionInOrderHelper(std::vector<TSecIter>& vSec, TSectionMap& Map)
	{
		for (auto it = m_Root.begin(); it != m_Root.end(); ++it)
		{
			vSec[it->second.Id] = it;
			ForSectionInOrderHelper(vSec, it->second.Child);
		}
	}
public:
	void ForSectionInOrder(auto&& Fn, const CtxSec& Sec = {})
	{
		std::vector<TSecIter> vSec{ m_uId };
		ForSectionInOrderHelper(vSec, Sec ? Sec->Child : m_Root);
		const auto itEnd = std::remove(vSec.begin(), vSec.end(), TSecIter{});
		for (auto it = vSec.begin(); it != itEnd; ++it)
			Fn(*it);
	}

	void ForValueInOrder(auto&& Fn, const CtxSec& Sec)
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
};
ECK_NAMESPACE_END