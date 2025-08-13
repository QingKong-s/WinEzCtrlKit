#pragma once
#include "CRefStr.h"
#include "CFile.h"
#include "AutoPtrDef.h"
#include "NativeWrapper.h"
#include "CUnknown.h"

#define ECK_LYRIC_NAMESPACE_BEGIN	namespace Lyric {
#define ECK_LYRIC_NAMESPACE_END		}
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

	BOOLEAN m_bDiscardEmptyLine{};
	BOOLEAN m_bDiscardEmptyWord{ TRUE };

	Result LrcParseLabelWorker(PCWCH& p, PCWCH pEnd,
		BOOL* pbAddLrc, BOOL* pbAddLabel, BOOL bAngleBracket)
	{
		if (pbAddLrc) *pbAddLrc = FALSE;
		if (pbAddLabel) *pbAddLabel = FALSE;
		const auto chBracketL = bAngleBracket? L'<' : L'[';
		const auto chBracketR = bAngleBracket? L'>' : L']';
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

	void SortAndMerge()
	{
		std::stable_sort(m_vLine.begin(), m_vLine.end());
		EckCounter(m_vLine.size(), i)
		{
			auto& e = m_vLine[i];
			if (e.cchWordTotal)
			{
				if (e.bMAlloc)
					free((void*)e.pszLrc);
				e.bMAlloc = TRUE;
				e.pszLrc = (PCWCH)malloc(Cch2CbW(e.cchWordTotal));
				EckCheckMem(e.pszLrc);
				e.cchLrc = e.cchWordTotal;
				PWCH p = (PWCH)e.pszLrc;
				for (const auto& w : e.vWordTime)
				{
					TcsCopyLen(p, w.pszWord, w.cchWord);
					p += w.cchWord;
				}
				*p = L'\0';
			}
		}
		// 合并时间相同的行
		std::vector<float> vLastTime{};
		std::vector<size_t> vNeedDelIndex{};
		vLastTime.reserve(5);
		vNeedDelIndex.reserve(m_vLine.size() / 2);
		EckCounter(m_vLine.size(), i)
		{
			auto& e = m_vLine[i];
			if (vLastTime.size() && i)
			{
				if (FloatEqual(vLastTime[0], e.fTime))
				{
					auto& TopItem = m_vLine[i - vLastTime.size()];// 索引i将合并到该项
					const auto b1 = (TopItem.cchLrc || TopItem.cchTranslation);
					const auto b2 = (e.cchLrc || e.cchTranslation);
					if (!b2)
						goto NoMerge;
					if (b1)// 有第一个和第二个
					{
						PWCH p, p1;
						const auto cbNew = Cch2CbW(TopItem.cchLrc + 1 + e.cchLrc + 1 +
							TopItem.cchTranslation + 1 + e.cchTranslation);
						// 原文\0翻译
						if (TopItem.bMAlloc)
						{
							p = (PWCH)realloc((void*)TopItem.pszLrc, cbNew);
							EckCheckMem(p);
						}
						else
						{
							p = (PWCH)malloc(cbNew);
							EckCheckMem(p);
							TopItem.bMAlloc = TRUE;
							TcsCopyLenEnd(p, TopItem.pszLrc, TopItem.cchLrc);
							TcsCopyLen(p + TopItem.cchLrc + 1,
								TopItem.pszTranslation, TopItem.cchTranslation);
						}
						// 原文\0翻译\n
						p1 = p + TopItem.cchLrc + 1 + TopItem.cchTranslation;
						if (TopItem.cchTranslation)
							*p1++ = L'\n';
						// 原文\0翻译\n翻译
						if (e.cchLrc)
						{
							TcsCopyLen(p1, e.pszLrc, e.cchLrc);
							p1 += e.cchLrc;
						}
						// 原文\0翻译\n翻译\n翻译
						if (e.cchTranslation)
						{
							*p1++ = L'\n';
							TcsCopyLen(p1, e.pszTranslation, e.cchTranslation);
							p1 += e.cchTranslation;
						}
						//
						*p1 = L'\0';

						TopItem.pszLrc = (PCWCH)p;
						TopItem.pszTranslation = (PCWCH)p + TopItem.cchLrc + 1;
						TopItem.cchTranslation = USHORT(p1 - TopItem.pszTranslation);
					}
					else// 只有第二个，移动之
						TopItem = std::move(e);
				NoMerge:
					vNeedDelIndex.emplace_back(i);
				}
				else
					vLastTime.clear();
			}
			vLastTime.emplace_back(e.fTime);
		}

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
		SortAndMerge();
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