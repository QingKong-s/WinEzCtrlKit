#pragma once
#include "CRefStr.h"
#include "CFile.h"
#include "AutoPtrDef.h"
#include "NativeWrapper.h"
#include "CUnknown.h"

#define ECK_LYRIC_NAMESPACE_BEGIN	namespace Lyric {
#define ECK_LYRIC_NAMESPACE_END		}

#if 1
#define EckLrcValidateHeap()		_CrtCheckMemory()
#else
#define EckLrcValidateHeap()		{}
#endif

ECK_NAMESPACE_BEGIN
ECK_LYRIC_NAMESPACE_BEGIN
enum class Result
{
	Ok,
	TlInvalidChar,		// 字段中含有非法字符
	TlFieldEmpty,		// 字段为空
	TlInvalidMsLength,	// 毫秒长度不正确
	TlUnexpectedEnd,	// 意外结尾
};


struct WordTime
{
	PCWCH pszWord;
	int cchWord;
	float fTime;
	float fDuration;
};

struct Line
{
	PCWCH pszLrc{};
	PCWCH pszTranslation{};
	USHORT cchLrc{};
	USHORT cchTranslation{};
	USHORT cchWordTotal{};
	BOOLEAN bMAlloc{};
	float fTime{};
	float fDuration{};
	std::vector<WordTime> vWordTime;

	Line() = default;
	Line(Line&& x) noexcept
	{
		std::swap(pszLrc, x.pszLrc);
		std::swap(pszTranslation, x.pszTranslation);
		std::swap(cchLrc, x.cchLrc);
		std::swap(cchTranslation, x.cchTranslation);
		std::swap(cchWordTotal, x.cchWordTotal);
		std::swap(bMAlloc, x.bMAlloc);
		std::swap(fTime, x.fTime);
		std::swap(fDuration, x.fDuration);
		std::swap(vWordTime, x.vWordTime);
	}
	Line& operator=(Line&& x) noexcept
	{
		std::swap(pszLrc, x.pszLrc);
		std::swap(pszTranslation, x.pszTranslation);
		std::swap(cchLrc, x.cchLrc);
		std::swap(cchTranslation, x.cchTranslation);
		std::swap(cchWordTotal, x.cchWordTotal);
		std::swap(bMAlloc, x.bMAlloc);
		std::swap(fTime, x.fTime);
		std::swap(fDuration, x.fDuration);
		std::swap(vWordTime, x.vWordTime);
		return *this;
	}
	~Line()
	{
		if (bMAlloc)
			free((void*)pszLrc);
	}

	constexpr std::partial_ordering operator<=>(const Line& x) const
	{
		return fTime <=> x.fTime;
	}
};

struct Label
{
	PCWCH pszKey;
	PCWCH pszValue;
	int cchKey;
	int cchValue;
};

struct LineWord
{
	int cchTotal;
	float fTime;

	constexpr std::partial_ordering operator<=>(const Line& x) const
	{
		return fTime <=> x.fTime;
	}
};

class CLyric final : public CRefObj<CLyric>
{
private:
	CRefStrW m_rsLyric{};
	std::vector<Label> m_vLabel{};
	std::vector<Line> m_vLine{};

	BOOLEAN m_bDiscardEmptyLine{};		// 丢弃空白行
	BOOLEAN m_bDiscardEmptyWord{ TRUE };// 丢弃空白字
	BOOLEAN m_bRawLine{};				// 不进行排序合并操作

	Result LrcParseLabelWorker(PCWCH& p, PCWCH pEnd,
		BOOL* pbAddLrc, BOOL* pbAddLabel, BOOL bAngleBracket)
	{
		if (pbAddLrc) *pbAddLrc = FALSE;
		if (pbAddLabel) *pbAddLabel = FALSE;
		const auto chBracketL = bAngleBracket ? L'<' : L'[';
		const auto chBracketR = bAngleBracket ? L'>' : L']';
		enum class State
		{
			Init,

			M,
			S,
			Ms,

			Key,
			Value,
		};
		State eState = State::Init;
		PCWCH pLast{ p };
		float fTime{};
		int n;
		while (p < pEnd)
		{
			const auto ch = *p++;
			switch (eState)
			{
			case State::Init:
				if (iswalpha(ch))
					if (bAngleBracket)
						return Result::TlInvalidChar;
					else
						eState = State::Key;// [ar:xxx]等
				else if (iswdigit(ch))
					eState = State::M;	// [00:00.000]等
				else
					return Result::TlInvalidChar;
				break;
			case State::M:
				if (ch == '>' && bAngleBracket)// TRC支持
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					auto& Line = m_vLine.back();
					Line.vWordTime.emplace_back().fTime = Line.fTime + (float)n / 1000.f;
					if (pbAddLrc) *pbAddLrc = TRUE;
					return Result::Ok;
				}
				else if (ch == ':')
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					fTime = (float)n * 60.f;
					eState = State::S;
					pLast = p;
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::S:
				if (ch == '.' || ch == ':' || ch == chBracketR)
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					fTime += (float)n;
					if (ch == chBracketR)
					{
						if (bAngleBracket)
							m_vLine.back().vWordTime.emplace_back().fTime = fTime;
						else
							m_vLine.emplace_back().fTime = fTime;
						if (pbAddLrc) *pbAddLrc = TRUE;
						return Result::Ok;
					}
					else
					{
						pLast = p;
						eState = State::Ms;
					}
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::Ms:
				if (ch == chBracketR)
				{
					const auto cch = p - pLast - 1;
					if (cch <= 0)
						return Result::TlFieldEmpty;
					else if (cch != 1 && cch != 2 && cch != 3)
						return Result::TlInvalidMsLength;
					TcsToInt(pLast, cch, n, 10);
					fTime += (float)n / (cch == 1 ? 10.f :
						cch == 2 ? 100.f : 1000.f);
					if (bAngleBracket)
						m_vLine.back().vWordTime.emplace_back().fTime = fTime;
					else
						m_vLine.emplace_back().fTime = fTime;
					if (pbAddLrc) *pbAddLrc = TRUE;
					return Result::Ok;
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::Key:
				EckAssert(!bAngleBracket);
				if (ch == ':')
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;

					auto& e = m_vLabel.emplace_back();
					e.pszKey = pLast;
					e.cchKey = int(p - pLast - 1);
					eState = State::Value;
					pLast = p;
					if (pbAddLabel) *pbAddLabel = TRUE;
				}
				else if (ch == chBracketR || ch == chBracketL ||
					ch == '\r' || ch == '\n')
					return Result::TlUnexpectedEnd;
				break;
			case State::Value:
				EckAssert(!bAngleBracket);
				if (ch == chBracketR)
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					auto& e = m_vLabel.back();
					e.pszValue = pLast;
					e.cchValue = int(p - pLast - 1);
					return Result::Ok;
				}
				else if (ch == chBracketL || ch == '\r' || ch == '\n')
					return Result::TlUnexpectedEnd;
				break;
			}
		}
		return Result::TlUnexpectedEnd;
	}

	Result LrcParseLabel(PCWCH& p, PCWCH pEnd,
		BOOL* pbAddLrc = nullptr, BOOL* pbAddLabel = nullptr)
	{
		return LrcParseLabelWorker(p, pEnd, pbAddLrc, pbAddLabel, FALSE);
	}

	Result LrcParseWordTime(PCWCH& p, PCWCH pEnd, BOOL* pbAddLrc = nullptr)
	{
		return LrcParseLabelWorker(p, pEnd, pbAddLrc, nullptr, TRUE);
	}

	void LrcpWordEnd(PCWCH p0, PCWCH p1, BOOL bAddLrc)
	{
		auto& e = *(m_vLine.back().vWordTime.rbegin() + bAddLrc);
		e.cchWord = int(p1 - p0 - 1);
		if (!e.cchWord && m_bDiscardEmptyWord)
			m_vLine.back().vWordTime.pop_back();
		else
		{
			m_vLine.back().cchWordTotal += e.cchWord;
			e.pszWord = (e.cchWord ? p0 : nullptr);
			DbgCutString(e);
		}
	}
	void LrcpWordEndEmpty(BOOL bAddLrc)
	{
		if (m_bDiscardEmptyWord)
			if (bAddLrc)
				m_vLine.back().vWordTime.erase((m_vLine.back().vWordTime.rbegin() + 1).base());
			else
				m_vLine.back().vWordTime.pop_back();
	}

	void MgpCopyWordAsSentence(const std::vector<WordTime>& vWordTime, PWCH p)
	{
		for (const auto& e : vWordTime)
		{
			TcsCopyLen(p, e.pszWord, e.cchWord);
			p += e.cchWord;
		}
	}

	BOOL MgpIsWordTime(const Line& x) { return !!x.vWordTime.size(); }

	void MgpMerge(const std::vector<size_t>& vNeedDelIndex, size_t cLine, Line& TopItem)
	{
		if (!cLine)
			return;
		const auto bWordTime = MgpIsWordTime(TopItem);
		if (bWordTime)
			TopItem.cchLrc = TopItem.cchWordTotal;
		int cchText{};
		for (auto it = vNeedDelIndex.rbegin();
			it != vNeedDelIndex.rbegin() + cLine; ++it)
		{
			const auto i = *it;
			const auto& e = m_vLine[i];
			const auto bWordTime = MgpIsWordTime(e);
			if (bWordTime)
			{
				if (e.cchWordTotal)
					cchText += (1/*\n*/ + e.cchWordTotal);
			}
			else
			{
				if (e.cchLrc)
					cchText += (1/*\n*/ + e.cchLrc);
				if (e.cchTranslation)
					cchText += (1/*\n*/ + e.cchTranslation);
			}
		}
		const auto cbBuf = Cch2CbW(cchText +
			TopItem.cchLrc + 1/*\0*/ + 1/*\n*/ + TopItem.cchTranslation + 1/*\0*/);
		PWCH pszText = (PWCH)malloc(cbBuf);
		EckCheckMem(pszText);
		PWCH p = pszText;
		if (TopItem.bMAlloc)
			free((void*)TopItem.pszLrc);
		TopItem.bMAlloc = TRUE;
		// 第一项正文
		TopItem.pszLrc = p;
		if (bWordTime)
			MgpCopyWordAsSentence(TopItem.vWordTime, p);
		else
			TcsCopyLen(p, TopItem.pszLrc, TopItem.cchLrc);
		p += TopItem.cchLrc;
		*p++ = L'\0';
		EckLrcValidateHeap();
		// 第一项翻译
		if (TopItem.cchTranslation)
		{
			EckAssert(!bWordTime);
			TcsCopyLen(p, TopItem.pszTranslation, TopItem.cchTranslation);
			TopItem.pszTranslation = p;
			p += TopItem.cchTranslation;
			EckLrcValidateHeap();
		}
		else
			TopItem.pszTranslation = p + 1;
		// 其他项
		for (auto it = vNeedDelIndex.rbegin();
			it != vNeedDelIndex.rbegin() + cLine; ++it)
		{
			const auto i = *it;
			const auto& e = m_vLine[i];
			const auto bWordTime = MgpIsWordTime(e);
			if (bWordTime)
			{
				if (e.cchWordTotal)
				{
					*p++ = L'\n';
					MgpCopyWordAsSentence(e.vWordTime, p);
					p += e.cchWordTotal;
					EckLrcValidateHeap();
				}
			}
			else
			{
				if (e.cchLrc)
				{
					*p++ = L'\n';
					TcsCopyLen(p, e.pszLrc, e.cchLrc);
					p += e.cchLrc;
					EckLrcValidateHeap();
				}
				if (e.cchTranslation)
				{
					EckAssert(!bWordTime);
					*p++ = L'\n';
					TcsCopyLen(p, e.pszTranslation, e.cchTranslation);
					p += e.cchTranslation;
					EckLrcValidateHeap();
				}
			}
		}
		if (TopItem.pszTranslation < p)
		{
			TopItem.cchTranslation = USHORT(p - TopItem.pszTranslation);
			*((PWCH)TopItem.pszTranslation + TopItem.cchTranslation) = L'\0';
		}
		else
		{
			TopItem.pszTranslation = nullptr;
			TopItem.cchTranslation = 0;
		}
	}

	void MgpSortAndMerge()
	{
		std::stable_sort(m_vLine.begin(), m_vLine.end());
		std::vector<float> vLastTime{};
		std::vector<size_t> vNeedDelIndex{};
		size_t idxTop{};// 连续相同时间的第一项索引
		size_t cLineSameTime{};
		vLastTime.reserve(5);
		vNeedDelIndex.reserve(10);
		EckCounter(m_vLine.size(), i)
		{
			auto& e = m_vLine[i];
			if (vLastTime.size() && i)
			{
				if (FloatEqual(vLastTime[0], e.fTime))
				{
					auto& TopItem = m_vLine[i - vLastTime.size()];// 索引i将合并到该项
					++cLineSameTime;
					vNeedDelIndex.emplace_back(i);
				}
				else
				{
					MgpMerge(vNeedDelIndex, cLineSameTime, m_vLine[idxTop]);
					vLastTime.clear();
					idxTop = i;
					cLineSameTime = 0;
				}
			}
			vLastTime.emplace_back(e.fTime);
		}
		MgpMerge(vNeedDelIndex, cLineSameTime, m_vLine[idxTop]);
		for (auto it = vNeedDelIndex.rbegin(); it < vNeedDelIndex.rend(); ++it)
			m_vLine.erase(m_vLine.begin() + *it);
	}

	void DbgCutString(const Line& x)
	{
		if (x.pszLrc)
			*((PWCH)x.pszLrc + x.cchLrc) = 0;
		if (x.pszTranslation)
			*((PWCH)x.pszTranslation + x.cchTranslation) = 0;
	}
	void DbgCutString(const WordTime& x)
	{
		if (x.pszWord)
			*((PWCH)x.pszWord + x.cchWord) = 0;
	}
public:
	void LoadTextStrView(std::wstring_view sv) noexcept
	{
		m_rsLyric.ReSize((int)sv.size());
		TcsCopyLen(m_rsLyric.Data(), sv.data(), (int)sv.size());
	}
	void LoadTextMove(CRefStrW&& rs) noexcept { m_rsLyric = std::move(rs); }
	NTSTATUS LoadTextFile(PCWSTR pszFileName)
	{
		// TODO
	}

	Result ParseLrc(BOOL bElrcSquareBracket = FALSE)
	{
		enum class State
		{
			Normal,
			Label,
			WordTime,
			Text,
			WordText,
		};
		Result r;
		State eState = State::Normal;
		PCWCH p{ m_rsLyric.Data() };
		const auto pEnd = p + m_rsLyric.Size();
		PCWCH pLast{};
		BOOL bAddLrc, bAddLabel;

		int cLabelContinues{};
		while (p < pEnd)
		{
			const auto ch = *p++;
			switch (eState)
			{
			case State::Normal:
				if (ch == '[')
				{
					--p;
					eState = State::Label;
				}
				break;
			case State::Label:
				r = LrcParseLabel(p, pEnd, &bAddLrc);
				if (r != Result::Ok)
					return r;
				if (bAddLrc)
					++cLabelContinues;
				if (*p == '[')
					continue;
				else if (*p == '<')// 方括号标签后的第一个尖括号
					eState = State::WordTime;
				else
				{
					pLast = p;
					eState = State::Text;
				}
				break;
			case State::WordTime:
				r = LrcParseWordTime(p, pEnd, &bAddLrc);
				if (r != Result::Ok)
					return r;
				if (bAddLrc)
					++cLabelContinues;
				if (*p == '[')
					eState = State::Label;
				else if (*p == '<')
					eState = State::WordTime;
				else
				{
					pLast = p;
					eState = State::WordText;
				}
				break;
			case State::Text:
				if (ch == '[')// 尝试解析标签
				{
					const auto pOld = p;
					r = LrcParseLabel(p, pEnd, &bAddLrc, &bAddLabel);
					if (r == Result::Ok)// 成功，文本结束
					{
						for (auto it = m_vLine.rbegin() + bAddLrc;
							it != m_vLine.rbegin() + bAddLrc + cLabelContinues; ++it)
						{
							it->cchLrc = USHORT(pOld - pLast - 1);
							it->pszLrc = (pLast ? pLast : nullptr);
							DbgCutString(*it);
						}
						cLabelContinues = 1;
						if (*p == '[')
							eState = State::Label;
						else if (*p == '<')
							eState = State::WordTime;
						else
							pLast = p;
					}
					else// 失败，回退所有修改并继续作为文本处理
					{
						p = pOld;
						if (bAddLrc)
							m_vLine.pop_back();
						if (bAddLabel)
							m_vLabel.pop_back();
					}
				}
				else if (ch == '<')
				{

				}
				else if (ch == '\r' || ch == '\n' || p == pEnd)
				{
					eState = State::Normal;
					for (auto it = m_vLine.rbegin();
						it != m_vLine.rbegin() + cLabelContinues; ++it)
					{
						it->cchLrc = USHORT(p - pLast - 1);
						it->pszLrc = (pLast ? pLast : nullptr);
						DbgCutString(*it);
					}
					cLabelContinues = 0;
				}
				break;
			case State::WordText:
				if (ch == '[')// 尝试解析标签
				{
					const auto pOld = p;
					r = LrcParseLabel(p, pEnd, &bAddLrc, &bAddLabel);
					if (r == Result::Ok)// 成功，文本结束
					{
						LrcpWordEnd(pLast, pOld, FALSE);
						cLabelContinues = 1;
						if (*p == '[')
						{
							LrcpWordEndEmpty(bAddLrc);
							eState = State::Label;
						}
						else if (*p == '<')
						{
							LrcpWordEndEmpty(bAddLrc);
							eState = State::WordTime;
						}
						else
							pLast = p;
					}
					else// 失败，回退所有修改并继续作为文本处理
					{
						p = pOld;
						if (bAddLrc)
							m_vLine.pop_back();
						if (bAddLabel)
							m_vLabel.pop_back();
					}
				}
				else if (ch == '<')
				{
					const auto pOld = p;
					r = LrcParseWordTime(p, pEnd, &bAddLrc);
					if (r == Result::Ok)
					{
						LrcpWordEnd(pLast, pOld, bAddLrc);
						cLabelContinues = 1;
						if (*p == '[')
						{
							LrcpWordEndEmpty(bAddLrc);
							eState = State::Label;
						}
						else if (*p == '<')
						{
							LrcpWordEndEmpty(bAddLrc);
							eState = State::WordTime;
						}
						else
							pLast = p;
					}
					else
					{
						p = pOld;
						if (bAddLrc)
							m_vLine.back().vWordTime.pop_back();
					}
				}
				else if (ch == '\r' || ch == '\n' || p == pEnd)
				{
					LrcpWordEnd(pLast, p, FALSE);
					eState = State::Normal;
					cLabelContinues = 0;
				}
				break;
			}
		}
		MgpSortAndMerge();
		return Result::Ok;
	}

	void ParseKrc()
	{

	}

	void ParseQrc()
	{

	}

	void ParseKsc()
	{

	}
};
ECK_NAMESPACE_END
ECK_LYRIC_NAMESPACE_END