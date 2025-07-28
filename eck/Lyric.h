#pragma once
#include "CRefStr.h"
#include "CFile.h"
#include "AutoPtrDef.h"
#include "NativeWrapper.h"

ECK_NAMESPACE_BEGIN
struct LRCTIMELABEL
{
	PCWSTR pszLabel;
	int pos1;
	int pos2;
};

enum class LrcEncoding
{
	Auto,
	GB2312,
	UTF8,
	UTF16LE,
	UTF16BE
};

struct LRCINFO
{
	PWSTR pszLrc{};// 歌词
	PWSTR pszTranslation{};// 翻译，指向pszLrc的中间，可能为NULL
	int cchTotal{};
	int cchLrc{};
	float fTime{};
	float fDuration{};

	LRCINFO() = default;
	constexpr LRCINFO(PWSTR pszLrc_, PWSTR pszTranslation_,
		int cchTotal_, int cchLrc_, float fTime_, float fDuration_)
		:pszLrc{ pszLrc_ }, pszTranslation{ pszTranslation_ }, cchTotal{ cchTotal_ }, cchLrc{ cchLrc_ }
		, fTime{ fTime_ }, fDuration{ fDuration_ } {
	}

	LRCINFO(const LRCINFO& li)
	{
		memcpy(this, &li, sizeof(LRCINFO));
		pszLrc = (PWSTR)malloc(Cch2CbW(cchTotal));
		EckAssert(pszLrc);
		pszTranslation = pszLrc + cchLrc;
		wcscpy(pszLrc, li.pszLrc);
	}

	LRCINFO(LRCINFO&& li) noexcept
	{
		memcpy(this, &li, sizeof(LRCINFO));
		ZeroMemory(&li, sizeof(LRCINFO));
	}

	LRCINFO& operator=(const LRCINFO& li)
	{
		free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
		pszLrc = (PWSTR)malloc(Cch2CbW(cchTotal));
		EckAssert(pszLrc);
		pszTranslation = pszLrc + cchLrc;
		wcscpy(pszLrc, li.pszLrc);
		return *this;
	}

	LRCINFO& operator=(LRCINFO&& li) noexcept
	{
		free(pszLrc);
		memcpy(this, &li, sizeof(LRCINFO));
		ZeroMemory(&li, sizeof(LRCINFO));
		return *this;
	}

	~LRCINFO()
	{
		free(pszLrc);
	}
};

struct LRCLABEL
{
	CRefStrW rsKey{};
	CRefStrW rsValue{};
};

ECK_PRIV_NAMESPACE_BEGIN
inline void ParseLrc_ProcTimeLabel(std::vector<LRCINFO>& Result, std::vector<LRCLABEL>& Label,
	const std::vector<LRCTIMELABEL>& TimeLabel, PWSTR pszLrc, int cchLrc)
{
#pragma warning (push)
#pragma warning (disable: 6387)// 可能是NULL
#pragma warning (disable: 6053)// 可能未添加终止NULL
	int M, S, MS;
	float fTime;
	EckCounter(TimeLabel.size(), i)
	{
		auto& Label = TimeLabel[i];

		// 取分钟
		StrToIntExW(Label.pszLabel, STIF_DEFAULT, &M);
		fTime = (float)M * 60.f;
		// 取秒
		StrToIntExW(Label.pszLabel + Label.pos1 + 1, STIF_DEFAULT, &S);
		fTime += (float)S;
		// 取毫秒
		if (Label.pos2 > 0)
		{
			StrToIntExW(Label.pszLabel + Label.pos2 + 1, STIF_DEFAULT, &MS);
			const float fScale = (MS < 100 ? 100.f : 1000.f);
			fTime += ((float)MS / fScale);
		}

		if (fTime < 0)
			continue;

		// 删首尾空
		const auto pBegin = LTrimStr(pszLrc, cchLrc);
		const auto pEnd = RTrimStr(pszLrc, cchLrc);
		const auto cchReal = (pEnd > pBegin) ? int(pEnd - pBegin) : 0;
		if (cchReal)
		{
			const auto pTemp = (PWSTR)malloc(Cch2CbW(cchReal));
			EckCheckMem(pTemp);
			Result.emplace_back(pTemp, nullptr, cchReal, cchReal, fTime, 0.f);
			TcsCopyLenEnd(pTemp, pBegin, cchReal);
		}
		else
			Result.emplace_back(nullptr, nullptr, 0, 0, fTime, 0.f);
	}
#pragma warning (pop)
}

inline BOOL ParseLrc_IsTimeLabelLegal(PCWSTR pszLabel, int cchLabel,
	_Out_ int& posFirstDiv, _Out_ int& posSecondDiv, _Out_ BOOL& bMS)
{
	posFirstDiv = -1;
	posSecondDiv = -1;
	int pos1, pos2;
	pos1 = FindStr(pszLabel, L":");
	if (pos1 <= 0 || pos1 >= cchLabel - 1)
		return FALSE;// 没冒号、冒号在开头、冒号在结尾、冒号超过结尾都不合法

	pos2 = FindStr(pszLabel, L":", pos1 + 1);
	if (pos2 == pos1 + 1)
		return FALSE;// 两个冒号挨着也不合法

	bMS = TRUE;
	if (pos2 < 0 || pos2 >= cchLabel)
	{
		pos2 = FindStr(pszLabel, L".", pos1 + 1);
		if (pos2 < 0 || pos2 >= cchLabel)
		{
			bMS = FALSE;// [分:秒]
			pos2 = cchLabel;
		}
		// else [分:秒.毫秒]
	}
	// else [分:秒:毫秒]

	posFirstDiv = pos1;
	if (bMS)
		posSecondDiv = pos2;

	// 测试第一个字段
	if (pos1 == 0)
		return FALSE;// 没有第一个字段

	for (int i = 0; i < pos1; ++i)
		if (!iswdigit(pszLabel[i]))
			return FALSE;// 第一个字段不是数字
	// 测试第二个字段
	if (pos2 <= pos1 + 1)
		return FALSE;// 没有第二个字段

	for (int i = pos1 + 1; i < pos2; ++i)
		if (!iswdigit(pszLabel[i]))
			return FALSE;// 第二个字段不是数字
	// 测试第三个字段
	if (bMS)
	{
		for (int i = pos2 + 1; i < cchLabel; ++i)
			if (!iswdigit(pszLabel[i]))
				return FALSE;// 第三个字段不是数字
	}
	return TRUE;
}
ECK_PRIV_NAMESPACE_END

/// <summary>
/// 解析LRC。
/// 支持压缩LRC，支持无换行LRC，支持歌词内嵌入中括号
/// </summary>
/// <param name="p">LRC字节流</param>
/// <param name="cbMem">字节流长度，若为0则表示p指向文件名</param>
/// <param name="Result">解析结果</param>
/// <param name="Label">非歌词标签的解析结果</param>
/// <param name="eTextEncoding">默认文本编码</param>
/// <param name="fTotalTime">歌曲总时间，用于计算歌词时间</param>
/// <returns>HRESULT</returns>
inline HRESULT ParseLrc(_In_reads_bytes_(cbMem) PCVOID p, SIZE_T cbMem,
	_Inout_ std::vector<LRCINFO>& Result, _Inout_ std::vector<LRCLABEL>& Label,
	LrcEncoding eTextEncoding, float fTotalTime)
{
	Result.clear();
	Label.clear();
#pragma region 读入数据
	BYTE* pFileData;
	if (cbMem)
	{
		if (cbMem < 5)
			return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
		pFileData = (BYTE*)VAlloc(cbMem + 2);
		if (!pFileData)
			return E_OUTOFMEMORY;
		memcpy(pFileData, p, cbMem);

		*(pFileData + cbMem) = '\0';
		*(pFileData + cbMem + 1) = '\0';
	}
	else
	{
		CFile File((PCWSTR)p, OPEN_EXISTING, GENERIC_READ, FILE_SHARE_READ);
		if (File.GetHandle() == INVALID_HANDLE_VALUE)
			return HRESULT_FROM_WIN32(GetLastError());// TODO:换用NT API
		cbMem = File.GetSize32();
		if (cbMem < 5)
			return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);

		pFileData = (BYTE*)VAlloc(cbMem + 2);
		if (!pFileData)
			return E_OUTOFMEMORY;
		File.Read(pFileData, (DWORD)cbMem);
		File.Close();
		*(pFileData + cbMem) = '\0';
		*(pFileData + cbMem + 1) = '\0';
	}
#pragma endregion
#pragma region 判断并转换编码
	PWSTR pszOrg = (PWSTR)pFileData;
	int cchFile = (int)(cbMem / sizeof(WCHAR));

	if (memcmp(pFileData, BOM_UTF16LE, 2) == 0)
	{
		--cchFile;
		pszOrg = (PWSTR)(pFileData + 2);
	}
	else if (memcmp(pFileData, BOM_UTF16BE, 2) == 0)
	{
		--cchFile;
		pszOrg = (PWSTR)VAlloc(Cch2CbW(cchFile));
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
			(PCWSTR)pFileData + 1, cchFile, pszOrg, cchFile, NULL, NULL, 0);// 反转字节序
		VFree(pFileData);
		pFileData = (BYTE*)pszOrg;
	}
	else if (memcmp(pFileData, BOM_UTF8, 3) == 0)
	{
		int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, NULL, 0);
		pszOrg = (PWSTR)VAlloc(Cch2CbW(cchBuf));
		MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData + 3, -1, pszOrg, cchBuf);// 转换编码
		VFree(pFileData);
		pFileData = (BYTE*)pszOrg;
		cchFile = cchBuf - 1;
	}
	else// 无BOM
	{
		switch (eTextEncoding)
		{
		case LrcEncoding::Auto:
		{
			int i = IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
			if (IsTextUnicode(pFileData, (int)cbMem, &i))// 先测UTF-16BE，不然会出问题
				goto GetLrc_UTF16BE;
			else
			{
				i = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
				if (IsTextUnicode(pFileData, (int)cbMem, &i))
					goto GetLrc_UTF16LE;
				else if (IsTextUTF8((char*)pFileData, cbMem))
					goto GetLrc_UTF8;
				else
					goto GetLrc_GB2312;
			}
		}
		break;
		case LrcEncoding::GB2312:
		{
		GetLrc_GB2312:
			int cchBuf = MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VAlloc(Cch2CbW(cchBuf));
			MultiByteToWideChar(936, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// 转换编码
			VFree(pFileData);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case LrcEncoding::UTF8:
		{
		GetLrc_UTF8:
			int cchBuf = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, NULL, 0);
			pszOrg = (PWSTR)VAlloc(Cch2CbW(cchBuf));
			MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pFileData, -1, pszOrg, cchBuf);// 转换编码
			VFree(pFileData);
			pFileData = (BYTE*)pszOrg;
			cchFile = cchBuf - 1;
		}
		break;
		case LrcEncoding::UTF16LE:
		GetLrc_UTF16LE:;
			break;
		case LrcEncoding::UTF16BE:
		{
		GetLrc_UTF16BE:
			pszOrg = (PWSTR)VAlloc((cchFile + 1) * sizeof(WCHAR));
			LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV,
				(PCWSTR)pFileData, cchFile, pszOrg, cchFile, NULL, NULL, 0);// 反转字节序
			VFree(pFileData);
			pFileData = (BYTE*)pszOrg;
		}
		break;
		}
	}
	UniquePtr<DelVA<BYTE>> _(pFileData);
#pragma endregion
#pragma region 按行分割
	std::vector<std::pair<PWSTR, int>> Lines{};
	Lines.reserve(50);

	constexpr WCHAR
		c_szDiv1[]{ L"\r\n" },// CRLF
		c_szDiv2[]{ L"\n" },// LF
		c_szDiv3[]{ L"\r" };// CR

	BOOL b1, b2, b3;

	int i1 = FindStr(pszOrg, c_szDiv1),
		i2 = FindStr(pszOrg, c_szDiv2),
		i3 = FindStr(pszOrg, c_szDiv3);

	b1 = i1 >= 0;
	b2 = i2 >= 0;
	b3 = i3 >= 0;

	PWSTR pszLine;// 每行内容
	int cchLine;// 每行内容长度

	int pos1;
	int pos2 = 0;
	int cchDiv;

	if (!b1 && !b2 && !b3)// 无换行符
		Lines.emplace_back(pszOrg, cchFile);
	else
	{
		// 适配那些混用三种换行符的傻逼文件
		if (b1)// CRLF
		{
			// 思路：iStrPos1 = min(i1, i2, i3)
			pos1 = i1;
			cchDiv = 2;
			if (b2 && i1 >= i2)// LF
			{
				pos1 = i2;
				cchDiv = 1;
			}
			if (b3 && i3 < pos1)// CR
			{
				pos1 = i3;
				cchDiv = 1;
			}
		}
		else
		{
			// 思路：iStrPos1 = min(i2, i3)
			cchDiv = 1;
			if (b2 && b3)// 没有CRLF，但CR和LF同时存在
			{
				if (i2 < i3)
					pos1 = i2;
				else
					pos1 = i3;
			}
			else if (b2)// LF
				pos1 = i2;
			else// CR
				pos1 = i3;
		}

		while (pos1)
		{
			cchLine = pos1 - pos2;
			if (cchLine > 0)
			{
				pszLine = pszOrg + pos2;
				*(pszLine + cchLine) = L'\0';
				Lines.emplace_back(pszLine, cchLine);
			}
			pos2 = pos1 + cchDiv;// 跳过换行符
			/////////////取下一换行符位置
			if (b1)
				i1 = FindStr(pszOrg, c_szDiv1, pos2);
			if (b2)
				i2 = FindStr(pszOrg, c_szDiv2, pos2);
			if (b3)
				i3 = FindStr(pszOrg, c_szDiv3, pos2);

			pos1 = 0;
			if (i1 >= 0)// CRLF
			{
				pos1 = i1;
				cchDiv = 2;
				if (i2 >= 0 && i1 >= i2)// LF
				{
					pos1 = i2;
					cchDiv = 1;
				}
				if (i3 >= 0 && i3 < pos1)// CR
				{
					pos1 = i3;
					cchDiv = 1;
				}
			}
			else
			{
				if (i2 >= 0 && i3 >= 0)// CR  LF
				{
					cchDiv = 1;
					if (i2 < i3)
						pos1 = i2;
					else
						pos1 = i3;
				}
				else if (i2 >= 0)// LF
				{
					cchDiv = 1;
					pos1 = i2;
				}
				else if (i3 >= 0)// CR
				{
					cchDiv = 1;
					pos1 = i3;
				}
			}
		}
		cchLine = cchFile - pos2;// 处理末尾一行文本
		if (cchLine > 0)
		{
			pszLine = pszOrg + pos2;
			*(pszLine + cchLine) = L'\0';
			Lines.emplace_back(pszLine, cchLine);
		}
	}
#pragma endregion
#pragma region 处理每行歌词
	int pos3;
	int cchTimeLabel;

	std::vector<LRCTIMELABEL> vTimeLabel{};// 相同的时间标签

	PWSTR pszTimeLabel;
	int posTimeDiv1, posTimeDiv2;
	BOOL bMS;

	EckCounter(Lines.size(), i)
	{
		pszLine = Lines[i].first;
		vTimeLabel.clear();

		pos1 = FindChar(pszLine, L'[');// 先找左中括号
		if (pos1 < 0)// 找不到左中括号
			continue;// 到循环尾（处理下一行）

		pos2 = FindChar(pszLine, L']', pos1 + 1);// 找下一个右中括号
		if (pos2 < 0)
			continue;// 中括号错误，这时还没有时间标签被读入，因此不是歌词中间出现的中括号，应该跳过这一行
	RetryThisLine:
		pszTimeLabel = pszLine + pos1 + 1;
		if (!Priv::ParseLrc_IsTimeLabelLegal(pszTimeLabel, pos2 - pos1 - 1,
			posTimeDiv1, posTimeDiv2, bMS))
		{
			if (posTimeDiv1 > 0)// 不合法，但有冒号，视为其他标签
			{
				*(pszTimeLabel + posTimeDiv1) = L'\0';
				*(pszLine + pos2) = L'\0';
				Label.emplace_back(pszTimeLabel, pszTimeLabel + posTimeDiv1 + 1);
				// 往后找，是否还有其他标签
				pos1 = FindChar(pszLine, L'[', pos2 + 1);
				if (pos1 >= 0)
				{
					pos2 = FindChar(pszLine, L']', pos1 + 1);
					if (pos2 >= 0)
						goto RetryThisLine;
				}
			}
			continue;// 不合法，跳过这一行
		}

		EckLoop()// 行中循环取标签
		{
		GetLabelLoopBegin:
			pszTimeLabel = pszLine + pos1 + 1;
			cchTimeLabel = pos2 - pos1 - 1;
			*(pszTimeLabel + posTimeDiv1) = L'\0';
			if (posTimeDiv2 > 0)
				*(pszTimeLabel + posTimeDiv2) = L'\0';
			*(pszTimeLabel + cchTimeLabel) = L'\0';
			vTimeLabel.emplace_back(pszLine + pos1 + 1, posTimeDiv1, posTimeDiv2);
			// 当前时间标签已解析完成

			pos3 = FindChar(pszLine, L'[', pos1 + cchTimeLabel + 2);// 找下一个左中括号
		TryNewBracket:
			if (pos3 < 0)// 找不到，则本行解析完成
			{
				Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
					pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
				break;
			}
			else if (pos3 == pos2 + 1)// 找到了，而且紧跟在上一个右中括号的后面
			{
				int posNextRightBracket = FindChar(pszLine, L']', pos3);// 找下一个右中括号
				if (posNextRightBracket < 0)// 找不到，则本行解析完成
				{
					Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
						pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
					break;
				}
				else
				{
					if (Priv::ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1,
						posNextRightBracket - pos3 - 1, posTimeDiv1, posTimeDiv2, bMS))
					{
						pos1 = pos3;
						pos2 = posNextRightBracket;
						continue;// 合法，继续循环
					}
					else// 如果不合法，跳过当前左中括号重试
					{
						pos3 = FindChar(pszLine, L'[', pos3 + 1);// 找下一个左中括号
						goto TryNewBracket;
					}
				}
			}
			else// 找到了，但离上一个右中括号有一段间隔
			{
				int posNextRightBracket = FindChar(pszLine, L']', pos3);// 找下一个右中括号
				if (posNextRightBracket < 0)// 找不到，则本行解析完成，剩余部分视为歌词
				{
					Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
						pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
					break;
				}
				else
				{
					if (Priv::ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1,
						posNextRightBracket - pos3 - 1, posTimeDiv1, posTimeDiv2, bMS))
					{// 如果是合法的，则间隔部分视为歌词
						Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
							pszLine + pos2 + 1, pos3 - pos2 - 1);
						vTimeLabel.clear();
						pos1 = pos3;
						pos2 = posNextRightBracket;
						continue;// 继续循环
					}
					else// 如果不合法，跳过当前左中括号重试
					{
						EckLoop()
						{
							pos3 = FindChar(pszLine, L'[', pos3 + 1);// 找下一个左中括号
							if (pos3 < 0)
								goto NoValidTimeLabel;
							else
							{
								int posNextRightBracket = FindChar(pszLine, L']', pos3);// 找下一个右中括号
								if (posNextRightBracket < 0)
									goto NoValidTimeLabel;
								else if (Priv::ParseLrc_IsTimeLabelLegal(pszLine + pos3 + 1,
									posNextRightBracket - pos3 - 1, posTimeDiv1, posTimeDiv2, bMS))
								{
									Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
										pszLine + pos2 + 1, pos3 - pos2 - 1);
									vTimeLabel.clear();
									pos1 = pos3;
									pos2 = posNextRightBracket;
									goto GetLabelLoopBegin;
								}
							}
						}

					NoValidTimeLabel:
						Priv::ParseLrc_ProcTimeLabel(Result, Label, vTimeLabel,
							pszLine + pos2 + 1, Lines[i].second - pos2 - 1);
						break;
					}
				}
			}
		}
	}
#pragma endregion
#pragma region 合并时间相同的歌词
	std::stable_sort(Result.begin(), Result.end(), [](const LRCINFO& a, const LRCINFO& b)->bool
		{
			return a.fTime < b.fTime;
		});

	std::vector<float> vLastTime{};
	std::vector<size_t> vNeedDelIndex{};
	vNeedDelIndex.reserve(Lines.size() / 2);

	EckCounter(Result.size(), i)
	{
		auto& e = Result[i];
		if (vLastTime.size() != 0 && i != 0)
		{
			if (FloatEqual(vLastTime[0], e.fTime))
			{
				auto& TopItem = Result[i - vLastTime.size()];
				const int cch1 = TopItem.cchTotal, cch2 = e.cchTotal;

				if (cch2)// 有第二个
					if (cch1)
					{
#pragma warning (suppress: 6308)// realloc为NULL
						TopItem.pszLrc = (PWSTR)realloc(TopItem.pszLrc, Cch2CbW(cch1 + cch2 + 1));
						EckCheckMem(TopItem.pszLrc);
						*(TopItem.pszLrc + cch1) = L'\n';
						wmemcpy(TopItem.pszLrc + cch1 + 1, e.pszLrc, e.cchTotal + 1);
						TopItem.pszTranslation = TopItem.pszLrc + TopItem.cchLrc + 1;
						TopItem.cchTotal = cch1 + cch2 + 1;
					}
					else
					{
						TopItem.pszLrc = e.pszLrc;
						TopItem.pszTranslation = NULL;
						e.pszLrc = NULL;
						TopItem.cchLrc = e.cchLrc;
						TopItem.cchTotal = e.cchTotal;
					}
				vNeedDelIndex.push_back(i);
			}
			else
				vLastTime.clear();
		}
		vLastTime.push_back(e.fTime);
	}

	for (auto it = vNeedDelIndex.rbegin(); it < vNeedDelIndex.rend(); ++it)
		Result.erase(Result.begin() + *it);

	if (!Result.empty())
	{
		EckCounter(Result.size() - 1, i)
		{
			auto& e = Result[i];
			e.fDuration = Result[i + 1].fTime - e.fTime;

			if (!e.pszLrc)
			{
				e.pszLrc = (PWSTR)malloc(Cch2CbW(0));
#pragma warning (suppress: 6011)// 解引用NULL
				* e.pszLrc = L'\0';
			}
		}
		auto& f = Result.back();
		f.fDuration = fTotalTime - Result.back().fTime;
		if (!f.pszLrc)
		{
			f.pszLrc = (PWSTR)malloc(Cch2CbW(0));
#pragma warning (suppress: 6011)// 解引用NULL
			* f.pszLrc = L'\0';
		}
	}
#pragma endregion
	return S_OK;
}
ECK_NAMESPACE_END