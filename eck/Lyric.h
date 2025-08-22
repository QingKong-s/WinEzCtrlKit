#pragma once
#include "CUnknown.h"
#include "EncodingDetect.h"

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
	InvalidChar,
};


struct WordTime
{
	PCWCH pszWord;
	int cchWord;
	float fTime;
	float fDuration;
	BOOL bDurationProcessed;
};

struct Line
{
	PCWCH pszLrc{};
	PCWCH pszTranslation{};
	USHORT cchLrc{};
	USHORT cchTranslation{};
	USHORT cchWordTotal{};
	BOOLEAN bMAlloc{};
	BOOLEAN bDurationProcessed{};
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
	struct YKQ_VAL
	{
		float f1;
		float f2;
		float f3;
	};
	enum class BracketType
	{
		Square,		// []
		Angle,		// <>
		Parenthesis,// ()
	};
	CRefStrW m_rsLyric{};
	std::vector<Label> m_vLabel{};
	std::vector<Line> m_vLine{};

	float m_fDuration{};				// 秒
	BOOLEAN m_bDiscardEmptyLine{};		// 丢弃空白行
	BOOLEAN m_bDiscardEmptyWord{ TRUE };// 丢弃空白字
	BOOLEAN m_bRawLine{};				// 不进行排序合并操作

	WCHAR LBracketFromType(BracketType e) const
	{
		switch (e)
		{
		case BracketType::Square: return L'[';
		case BracketType::Angle: return L'<';
		case BracketType::Parenthesis: return L'(';
		default: ECK_UNREACHABLE;
		}
	}
	WCHAR RBracketFromType(BracketType e) const
	{
		switch (e)
		{
		case BracketType::Square: return L']';
		case BracketType::Angle: return L'>';
		case BracketType::Parenthesis: return L')';
		default: ECK_UNREACHABLE;
		}
	}

	//---------------LRC---------------
	/// <summary>
	/// 解析LRC标签
	/// </summary>
	/// <param name="p">工作指针</param>
	/// <param name="pEnd">结束位置</param>
	/// <param name="pbAddLrc">返回是否尾插一行</param>
	/// <param name="pbAddLabel">返回是否尾插非时间标签</param>
	/// <param name="bAngleBracket">是否为尖括号标签</param>
	/// <param name="bSquareBracketElrc">是否正在解析方括号扩展LRC</param>
	/// <param name="bSbElrcAddLine">若bSquareBracketElrc为TRUE，此参数指示是否尾插行，一般用于每行的第一个标签</param>
	Result LrcpParseLabelWorker(PCWCH& p, PCWCH pEnd,
		BOOL* pbAddLrc, BOOL* pbAddLabel, BOOL bAngleBracket,
		BOOL bSquareBracketElrc, BOOL bSbElrcAddLine)
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
		auto Fn_TimeLabelEnd = [=](float fTime)
			{
				if (bSquareBracketElrc)
				{
					if (bSbElrcAddLine)
						m_vLine.emplace_back().fTime = fTime;
					m_vLine.back().vWordTime.emplace_back().fTime = fTime;
				}
				else
					if (bAngleBracket)
						m_vLine.back().vWordTime.emplace_back().fTime = fTime;
					else
						m_vLine.emplace_back().fTime = fTime;
				if (pbAddLrc) *pbAddLrc = TRUE;
			};
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
						if (bSquareBracketElrc && !bSbElrcAddLine)
							return Result::TlInvalidChar;// 非时间标签不可能出现在行中间
						else
							eState = State::Key;// [ar:xxx]等
				else if (iswdigit(ch))
					eState = State::M;// [00:00.000]等
				else
					return Result::TlInvalidChar;
				break;
			case State::M:
				if (ch == chBracketR && !bSquareBracketElrc)// TRC支持
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
						Fn_TimeLabelEnd(fTime);
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
					Fn_TimeLabelEnd(fTime);
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
					auto& e = m_vLabel.back();
					if (p - pLast <= 1)
					{
						e.pszValue = nullptr;
						e.cchValue = 0;
					}
					else
					{
						e.pszValue = pLast;
						e.cchValue = int(p - pLast - 1);
						DbgCutString(e);
					}
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
		return LrcpParseLabelWorker(p, pEnd, pbAddLrc, pbAddLabel, FALSE, FALSE, FALSE);
	}

	Result LrcParseWordTime(PCWCH& p, PCWCH pEnd, BOOL* pbAddLrc = nullptr)
	{
		return LrcpParseLabelWorker(p, pEnd, pbAddLrc, nullptr, TRUE, FALSE, FALSE);
	}

	Result LrcParseWordTimeSquareBracket(PCWCH& p, PCWCH pEnd, BOOL bFirstInLine,
		BOOL* pbAddLrc = nullptr, BOOL* pbAddLabel = nullptr)
	{
		return LrcpParseLabelWorker(p, pEnd, pbAddLrc, pbAddLabel, FALSE, TRUE, bFirstInLine);
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
	//---------------YRC/KRC/QRC---------------

	Result YkqParseLabel(PCWCH& p, PCWCH pEnd, YKQ_VAL& Val,
		BracketType eBracketType, BOOL b3Value, BOOL* pbAddLabel = nullptr)
	{
		if (pbAddLabel) *pbAddLabel = FALSE;
		const auto chBracketL = LBracketFromType(eBracketType);
		const auto chBracketR = RBracketFromType(eBracketType);
		enum class State
		{
			Init,

			V1,
			V2,
			V3,

			Key,
			Value,
		};
		State eState = State::Init;
		PCWCH pLast{ p };
		int n;
		while (p < pEnd)
		{
			const auto ch = *p++;
			switch (eState)
			{
			case State::Init:
				if (iswalpha(ch) && eBracketType == BracketType::Square)
					eState = State::Key;// [ar:xxx]等
				else if (iswdigit(ch))
					eState = State::V1;// [123,456]等
				else
					return Result::TlInvalidChar;
				break;
			case State::V1:
				if (ch == ',')
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					Val.f1 = (float)n / 1000.f;
					eState = State::V2;
					pLast = p;
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::V2:
				if (ch == (b3Value ? ',' : chBracketR))
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					Val.f2 = (float)n / 1000.f;
					if (b3Value)
						eState = State::V3;
					else
						return Result::Ok;
					pLast = p;
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::V3:
				if (ch == chBracketR)
				{
					if (p - pLast <= 1)
						return Result::TlFieldEmpty;
					TcsToInt(pLast, p - pLast - 1, n, 10);
					Val.f3 = (float)n / 1000.f;
					pLast = p;
					return Result::Ok;
				}
				else if (!iswdigit(ch))
					return Result::TlInvalidChar;
				break;
			case State::Key:
				EckAssert(eBracketType == BracketType::Square);
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
				EckAssert(eBracketType == BracketType::Square);
				if (ch == chBracketR)
				{
					auto& e = m_vLabel.back();
					if (p - pLast <= 1)
					{
						e.pszValue = nullptr;
						e.cchValue = 0;
					}
					else
					{
						e.pszValue = pLast;
						e.cchValue = int(p - pLast - 1);
					}
					DbgCutString(e);
					return Result::Ok;
				}
				else if (ch == chBracketL || ch == '\r' || ch == '\n')
					return Result::TlUnexpectedEnd;
				break;
			}
		}
		return Result::TlUnexpectedEnd;
	}
	//---------------
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
		const auto bWordTime = MgpIsWordTime(TopItem);
		if (bWordTime)
			TopItem.cchLrc = TopItem.cchWordTotal;
		if (!cLine && bWordTime)
		{
			const auto p = (PWCH)malloc(Cch2CbW(TopItem.cchLrc));
			EckCheckMem(p);
			MgpCopyWordAsSentence(TopItem.vWordTime, p);
			*(p + TopItem.cchLrc) = L'\0';
			if (TopItem.bMAlloc)
				free((void*)TopItem.pszLrc);
			TopItem.bMAlloc = TRUE;
			TopItem.pszLrc = p;
			return;
		}
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
		if (bWordTime)
			MgpCopyWordAsSentence(TopItem.vWordTime, p);
		else
			TcsCopyLen(p, TopItem.pszLrc, TopItem.cchLrc);
		TopItem.pszLrc = p;
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
		if (m_vLine.empty())
			return;
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

	void MgpCalculateWordDuration(Line& e)
	{
		auto& vWord = e.vWordTime;
		if (!vWord.empty())
		{
			for (size_t j{}; j < vWord.size() - 1; ++j)
			{
				auto& f = vWord[j];
				if (!f.bDurationProcessed)
				{
					f.bDurationProcessed = TRUE;
					f.fDuration = vWord[j + 1].fTime - f.fTime;
				}
			}
			if (!vWord.back().bDurationProcessed)// 最后一个字词
			{
				vWord.back().bDurationProcessed = TRUE;
				vWord.back().fDuration = e.fTime + e.fDuration - vWord.back().fTime;
			}
		}
	}

	void MgpCalculateDuration()
	{
		if (m_vLine.empty())
			return;
		for (size_t i{}; i < m_vLine.size() - 1; ++i)
		{
			auto& e = m_vLine[i];
			e.fDuration = m_vLine[i + 1].fTime - e.fTime;
			MgpCalculateWordDuration(e);
		}
		m_vLine.back().fDuration = m_fDuration - m_vLine.back().fTime;
		MgpCalculateWordDuration(m_vLine.back());
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
	void DbgCutString(const Label& x)
	{
		if (x.pszKey)
			*((PWCH)x.pszKey + x.cchKey) = 0;
		if (x.pszValue)
			*((PWCH)x.pszValue + x.cchValue) = 0;
	}
public:
	void LoadTextStringView(std::wstring_view sv) noexcept
	{
		m_rsLyric.ReSize((int)sv.size());
		TcsCopyLen(m_rsLyric.Data(), sv.data(), (int)sv.size());
	}
	void LoadTextMove(CRefStrW&& rs) noexcept { m_rsLyric = std::move(rs); }
	NTSTATUS LoadTextFile(PCWSTR pszFileName)
	{
		m_rsLyric.Clear();
		CRefBin rb;
		NTSTATUS nts;
		EcdLoadTextFile(CP_UTF16LE, rb, pszFileName, &nts);
		if (!rb.IsEmpty())
		{
			m_rsLyric.ReSize(int(rb.Size() / sizeof(WCHAR)));
			memcpy(m_rsLyric.Data(), rb.Data(), rb.Size());
		}
		return nts;
	}

	EckInlineNdCe BOOL IsTextEmpty() const noexcept { return m_rsLyric.IsEmpty(); }

	// 解析标准LRC、尖括号逐字LRC、TRC
	// 支持压缩LRC和无换行LRC
	Result ParseLrc()
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
				r = LrcParseLabel(p, pEnd, &bAddLrc, &bAddLabel);
				if (r != Result::Ok)
					return r;
				if (bAddLrc)
					++cLabelContinues;
				if (*p == '[')
					continue;
				else if (bAddLabel)
					eState = State::Normal;
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
						else if (bAddLabel)
							eState = State::Normal;
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
				// else if (ch == '<') {} // 不应处理
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
						LrcpWordEnd(pLast, pOld, bAddLrc);// 结束当前标签的上一个标签对应的文本
						cLabelContinues = 1;
						if (*p == '[')
						{
							LrcpWordEndEmpty(FALSE);// 立即结束当前标签
							eState = State::Label;
						}
						else if (*p == '<')
						{
							LrcpWordEndEmpty(FALSE);// 立即结束当前标签
							eState = State::WordTime;
						}
						else if (bAddLabel)
							eState = State::Normal;
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
							LrcpWordEndEmpty(FALSE);
							eState = State::Label;
						}
						else if (*p == '<')
						{
							LrcpWordEndEmpty(FALSE);
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
		MgpCalculateDuration();
		return Result::Ok;
	}

	// 解析使用方括号表示逐字的LRC
	Result ParseElrcWithSquareBracket()
	{
		enum class State
		{
			Normal,
			Label,
			WordText,
		};
		Result r;
		State eState = State::Normal;
		PCWCH p{ m_rsLyric.Data() };
		const auto pEnd = p + m_rsLyric.Size();
		PCWCH pLast{ p };
		BOOL bAddLrc, bAddLabel, bNewLine{ TRUE };

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
				r = LrcParseWordTimeSquareBracket(p, pEnd, bNewLine, &bAddLrc, &bAddLabel);
				if (r != Result::Ok)
					return r;
				if (*p == '[')
					continue;
				else if (bAddLabel)
					eState = State::Normal;
				else
				{
					pLast = p;
					eState = State::WordText;
				}
				break;
			case State::WordText:
				if (ch == '[')// 尝试解析标签
				{
					const auto pOld = p;
					r = LrcParseWordTimeSquareBracket(p, pEnd, FALSE, &bAddLrc, &bAddLabel);
					if (r == Result::Ok)// 成功，文本结束
					{
						LrcpWordEnd(pLast, pOld, TRUE);
						if (*p == '[')
						{
							LrcpWordEndEmpty(FALSE);
							eState = State::Label;
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
				else if (ch == '\r' || ch == '\n' || p == pEnd)
				{
					bNewLine = TRUE;
					LrcpWordEnd(pLast, p, FALSE);
					eState = State::Normal;
				}
				break;
			}
		}
		MgpSortAndMerge();
		MgpCalculateDuration();
		return Result::Ok;
	}

	Result ParseYrc()
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
		YKQ_VAL Val;
		BOOL bAddLabel;
		PCWCH pLast{};

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
				r = YkqParseLabel(p, pEnd, Val, BracketType::Square, FALSE, &bAddLabel);
				if (r != Result::Ok)
					return r;
				if (*p == '[')
					continue;
				else if (bAddLabel)
					eState = State::Normal;
				else if (*p == '(')// 方括号标签后的第一个圆括号
				{
					eState = State::WordTime;
					auto& e = m_vLine.emplace_back();
					e.fTime = Val.f1;
					e.fDuration = Val.f2;
				}
				else
					return Result::InvalidChar;
				break;
			case State::WordTime:
			{
				r = YkqParseLabel(p, pEnd, Val, BracketType::Parenthesis, TRUE);
				if (r != Result::Ok)
					return r;
				auto& e = m_vLine.back().vWordTime.emplace_back();
				e.fTime = Val.f1;
				e.fDuration = Val.f2;
				e.bDurationProcessed = TRUE;
				if (*p == '(')
					eState = State::WordTime;
				else
				{
					pLast = p;
					eState = State::WordText;
				}
			}
			break;
			case State::WordText:
				if (ch == '(')
				{
					const auto pOld = p;
					r = YkqParseLabel(p, pEnd, Val, BracketType::Parenthesis, TRUE);
					if (r == Result::Ok)
					{
						LrcpWordEnd(pLast, pOld, FALSE);
						if (*p == '(')
						{
							LrcpWordEndEmpty(FALSE);
							eState = State::WordTime;
						}
						else
							pLast = p;
						auto& e = m_vLine.back().vWordTime.emplace_back();
						e.fTime = Val.f1;
						e.fDuration = Val.f2;
						e.bDurationProcessed = TRUE;
					}
					else
						p = pOld;
				}
				else if (ch == '\r' || ch == '\n' || p == pEnd)
				{
					LrcpWordEnd(pLast, p, FALSE);
					eState = State::Normal;
				}
				break;
			}
		}
		MgpSortAndMerge();
		return Result::Ok;
	}

	Result ParseKrc()
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
		YKQ_VAL Val;
		BOOL bAddLabel;
		PCWCH pLast{};

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
				r = YkqParseLabel(p, pEnd, Val, BracketType::Square, FALSE, &bAddLabel);
				if (r != Result::Ok)
					return r;
				if (*p == '[')
					continue;
				else if (bAddLabel)
					eState = State::Normal;
				else if (*p == '<')// 方括号标签后的第一个尖括号
				{
					eState = State::WordTime;
					auto& e = m_vLine.emplace_back();
					e.fTime = Val.f1;
					e.fDuration = Val.f2;
				}
				else
					return Result::InvalidChar;
				break;
			case State::WordTime:
			{
				r = YkqParseLabel(p, pEnd, Val, BracketType::Angle, TRUE);
				if (r != Result::Ok)
					return r;
				auto& e = m_vLine.back().vWordTime.emplace_back();
				e.fTime = m_vLine.back().fTime + Val.f1;
				e.fDuration = Val.f2;
				e.bDurationProcessed = TRUE;
				if (*p == '<')
					eState = State::WordTime;
				else
				{
					pLast = p;
					eState = State::WordText;
				}
			}
			break;
			case State::WordText:
				if (ch == '<')
				{
					const auto pOld = p;
					r = YkqParseLabel(p, pEnd, Val, BracketType::Angle, TRUE);
					if (r == Result::Ok)
					{
						LrcpWordEnd(pLast, pOld, FALSE);
						if (*p == '<')
						{
							LrcpWordEndEmpty(FALSE);
							eState = State::WordTime;
						}
						else
							pLast = p;
						auto& e = m_vLine.back().vWordTime.emplace_back();
						e.fTime = m_vLine.back().fTime + Val.f1;
						e.fDuration = Val.f2;
						e.bDurationProcessed = TRUE;
					}
					else
						p = pOld;
				}
				else if (ch == '\r' || ch == '\n' || p == pEnd)
				{
					LrcpWordEnd(pLast, p, FALSE);
					eState = State::Normal;
				}
				break;
			}
		}
		MgpSortAndMerge();
		return Result::Ok;
	}

	Result ParseQrc()
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
		YKQ_VAL Val;
		BOOL bAddLabel;
		PCWCH pLast{};

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
				r = YkqParseLabel(p, pEnd, Val, BracketType::Square, FALSE, &bAddLabel);
				if (r != Result::Ok)
					return r;
				if (*p == '[')
					continue;
				else if (bAddLabel)
					eState = State::Normal;
				else
				{
					auto& e = m_vLine.emplace_back();
					e.fTime = Val.f1;
					e.fDuration = Val.f2;

					pLast = p;
					eState = State::WordText;
				}
				break;
			case State::WordText:
				if (ch == '(')
				{
					const auto pOld = p;
					r = YkqParseLabel(p, pEnd, Val, BracketType::Parenthesis, FALSE);
					if (r == Result::Ok)// 字词结束
					{
						auto& e = m_vLine.back().vWordTime.emplace_back();
						e.fTime = Val.f1;
						e.fDuration = Val.f2;
						e.bDurationProcessed = TRUE;
						LrcpWordEnd(pLast, pOld, FALSE);
						pLast = p;
					}
					else
						p = pOld;
				}
				else if (ch == '\r' || ch == '\n')
					eState = State::Normal;
				break;
			}
		}
		MgpSortAndMerge();
		return Result::Ok;
	}

	constexpr void MgClear()
	{
		m_vLine.clear();
		m_vLabel.clear();
		m_rsLyric.Clear();
	}

	EckInlineCe void MgSetDuration(float f) noexcept { m_fDuration = f; }

	EckInlineNdCe int MgGetLineCount() const noexcept { return (int)m_vLine.size(); }
	EckInlineNdCe auto& MgGetLine() noexcept { return m_vLine; }
	EckInlineNdCe auto& MgAtLine(int idx) const noexcept { return m_vLine[idx]; }
	constexpr int MgTimeToLine(float fTime, int idxCurr = -1) const noexcept
	{
		const auto cLrc = MgGetLineCount();
		if (!cLrc)
			return -1;
		if (idxCurr >= 0)// 尝试快速判断
		{
			if (idxCurr + 1 < cLrc)
			{
				if (fTime >= m_vLine[idxCurr].fTime &&
					fTime < m_vLine[idxCurr + 1].fTime)
					return idxCurr;
				else if (idxCurr + 2 < cLrc &&
					fTime >= m_vLine[idxCurr + 1].fTime &&
					fTime < m_vLine[idxCurr + 2].fTime)
					return idxCurr + 1;
			}
			else if (fTime >= m_vLine[idxCurr].fTime)
				return idxCurr;
		}
		const auto it = std::lower_bound(m_vLine.begin(), m_vLine.end(), fTime,
			[](const Line& Item, float fPos) -> bool {return Item.fTime < fPos; });
		int idx;
		if (it == m_vLine.end())
			idx = cLrc - 1;
		else if (it == m_vLine.begin())
			idx = -1;
		else
			idx = (int)std::distance(m_vLine.begin(), it - 1);
		EckAssert(idx >= -1 && idx < cLrc);
		return idx;
	}

	EckInlineNdCe auto& MgGetLabel() noexcept { return m_vLine; }
	EckInlineNdCe auto& MgAtLabel(int idx) const noexcept { return m_vLine[idx]; }
	int MgAtLabel(std::wstring_view svKey, int idxBegin = 0)
	{
		for (size_t i = idxBegin; i < m_vLabel.size(); ++i)
		{
			if (TcsEqualLen2I(svKey.data(), svKey.size(),
				m_vLabel[i].pszKey, m_vLabel[i].cchKey))
				return (int)i;
		}
		return -1;
	}
};
ECK_LYRIC_NAMESPACE_END
ECK_NAMESPACE_END