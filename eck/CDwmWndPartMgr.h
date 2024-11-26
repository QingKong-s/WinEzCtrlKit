/*
* WinEzCtrlKit Library
*
* CDwmWndPartMgr.h ： DWM主题标准窗口部件
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "ECK.h"

#include <vssym32.h>

ECK_NAMESPACE_BEGIN
enum DWMWINDOWPART
{
#pragma region 亮色
	// 按钮背景
	BUTTONACTIVECAPTION = 3,
	BUTTONINACTIVECAPTION,

	BUTTONACTIVECAPTIONEND,
	BUTTONINACTIVECAPTIONEND,

	BUTTONACTIVECLOSE,
	BUTTONINACTIVECLOSE,
	BUTTONACTIVECLOSEALONE,
	BUTTONINACTIVECLOSEALONE,
	// 按钮图标
	DWMWB_LIGHTBEGIN = 11,
	BUTTONCLOSEGLYPH96 = 11,
	BUTTONCLOSEGLYPH120,
	BUTTONCLOSEGLYPH144,
	BUTTONCLOSEGLYPH192,

	BUTTONHELPGLYPH96,
	BUTTONHELPGLYPH120,
	BUTTONHELPGLYPH144,
	BUTTONHELPGLYPH192,

	BUTTONMAXGLYPH96,
	BUTTONMAXGLYPH120,
	BUTTONMAXGLYPH144,
	BUTTONMAXGLYPH192,

	BUTTONMINGLYPH96,
	BUTTONMINGLYPH120,
	BUTTONMINGLYPH144,
	BUTTONMINGLYPH192,

	BUTTONRESTOREGLYPH96,
	BUTTONRESTOREGLYPH120,
	BUTTONRESTOREGLYPH144,
	BUTTONRESTOREGLYPH192,
#pragma endregion 亮色
#pragma region 暗色
	// 按钮背景
	BUTTONACTIVECAPTIONDARK = 88,
	BUTTONINACTIVECAPTIONDARK,

	BUTTONACTIVECAPTIONENDDARK,
	BUTTONINACTIVECAPTIONENDDARK,
	// 按钮图标
	DWMWB_DARKBEGIN = 64,
	BUTTONCLOSEGLYPH96DARK = 64,
	BUTTONCLOSEGLYPH120DARK,
	BUTTONCLOSEGLYPH144DARK,
	BUTTONCLOSEGLYPH192DARK,

	BUTTONHELPGLYPH96DARK,
	BUTTONHELPGLYPH120DARK,
	BUTTONHELPGLYPH144DARK,
	BUTTONHELPGLYPH192DARK,

	BUTTONMAXGLYPH96DARK,
	BUTTONMAXGLYPH120DARK,
	BUTTONMAXGLYPH144DARK,
	BUTTONMAXGLYPH192DARK,

	BUTTONMINGLYPH96DARK,
	BUTTONMINGLYPH120DARK,
	BUTTONMINGLYPH144DARK,
	BUTTONMINGLYPH192DARK,

	BUTTONRESTOREGLYPH96DARK,
	BUTTONRESTOREGLYPH120DARK,
	BUTTONRESTOREGLYPH144DARK,
	BUTTONRESTOREGLYPH192DARK,
#pragma endregion 暗色
};

struct DWMW_PART_INFO
{
	RECT rcInAtlas;		// 在图集中的位置
	MARGINS mgSizing;	// 九宫信息
	MARGINS mgContent;	// 内容边距
	int cSubImage;		// 子图个数
};

struct DWMW_GET_PART_EXTRA
{
	int idxGlyph;
	int idxBkg;
	const DWMW_PART_INFO* pGlyph;
	const DWMW_PART_INFO* pBkg;
};

enum class DwmWndPart : BYTE
{
	Close,
	Help,
	Max,
	Min,
	Restore,
	
	Invalid,// 用于命中测试
	Extra	// 用于命中测试
};

enum class DwmWPartState : BYTE
{
	Normal,
	Hot,
	Pressed,
	Disabled
};

class CDwmWndPartMgr
{
private:
	constexpr static int PartCount = 91;
	std::vector<DWMW_PART_INFO> m_Item{ PartCount };
	HTHEME m_hTheme{};
	HINSTANCE m_hInstStyle{};

	BOOL m_bSelfOpenTheme{};

	void QueryAtlasInfo(int iPart, DWMW_PART_INFO& Info)
	{
		GetThemeRect(m_hTheme, iPart, 0, TMT_ATLASRECT, &Info.rcInAtlas);
		GetThemeMargins(m_hTheme, nullptr, iPart, 0, TMT_CONTENTMARGINS, nullptr, &Info.mgContent);
		GetThemeMargins(m_hTheme, nullptr, iPart, 0, TMT_SIZINGMARGINS, nullptr, &Info.mgSizing);
		GetThemeInt(m_hTheme, iPart, 0, TMT_IMAGECOUNT, &Info.cSubImage);
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CDwmWndPartMgr);

	void Clear()
	{
		if (m_bSelfOpenTheme)
		{
			CloseThemeData(m_hTheme);
			FreeLibrary(m_hInstStyle);
			m_bSelfOpenTheme = FALSE;
		}
	}

	void AnalyzeTheme(HTHEME hTheme)
	{
		Clear();
		m_hTheme = hTheme;
		for (int i = 0; i < PartCount; ++i)
			QueryAtlasInfo(i, m_Item[i]);
	}

	HRESULT AnalyzeDefaultTheme()
	{
		Clear();
		PWSTR pszWinDir;
		HRESULT hr;
		if (SUCCEEDED(hr = SHGetKnownFolderPath(FOLDERID_Windows, KF_FLAG_DEFAULT, nullptr, &pszWinDir)))
		{
			constexpr WCHAR szFile[]{ LR"(\Resources\Themes\aero\aero.msstyles)" };
			const size_t cchWinDir = wcslen(pszWinDir);
			const size_t cchBuf = cchWinDir + ARRAYSIZE(szFile) + 1;
#pragma warning(suppress:6255)
			const auto pszBuf = (PWSTR)_alloca(cchBuf * sizeof(WCHAR));
			wmemcpy(pszBuf, pszWinDir, cchWinDir);
			wmemcpy(pszBuf + cchWinDir, szFile, ARRAYSIZE(szFile));
			*(pszBuf + cchBuf - 1) = L'\0';
			m_hInstStyle = LoadLibraryExW(pszBuf, 0, LOAD_LIBRARY_AS_DATAFILE);
			if (!m_hInstStyle)
				return HRESULT_FROM_WIN32(GetLastError());
			m_hTheme = OpenThemeData(nullptr, L"DWMWINDOW");
			if (!m_hTheme)
			{
				FreeLibrary(m_hInstStyle);
				m_hInstStyle = nullptr;
				return HRESULT_FROM_WIN32(GetLastError());
			}
			for (int i = 0; i < PartCount; ++i)
				QueryAtlasInfo(i, m_Item[i]);
			CoTaskMemFree(pszWinDir);
			m_bSelfOpenTheme = TRUE;
			return S_OK;
		}
		else
			return hr;
	}

	~CDwmWndPartMgr()
	{
		Clear();
	}

	EckInline HRESULT GetData(PCVOID* pp, DWORD* pcb) const
	{
		return GetThemeStream(m_hTheme, 0, 0, TMT_DISKSTREAM, (void**)pp, pcb, m_hInstStyle);
	}

	/// <summary>
	/// 取部件矩形
	/// </summary>
	/// <param name="rc">结果矩形</param>
	/// <param name="rcBkg">结果背景矩形</param>
	/// <param name="ePart">部件类别</param>
	/// <param name="eState">部件状态</param>
	/// <param name="bDark">是否暗色</param>
	/// <param name="bActive">标题栏是否激活</param>
	/// <param name="iDpi">DPI，将向上舍入到最近的DPI</param>
	/// <param name="pExtra">返回额外信息，可选</param>
	/// <returns>成功返回TRUE，失败返回FALSE</returns>
	BOOL GetPartRect(RECT& rc, RECT& rcBkg, DwmWndPart ePart, DwmWPartState eState,
		BOOL bDark, BOOL bActive, int iDpi, DWMW_GET_PART_EXTRA* pExtra = nullptr) const
	{
		if (g_NtVer.uBuild < WINVER_1809 && bDark)
			return FALSE;
		// 暂不支持Win7及以下版本
		if (g_NtVer.uMajor < 6)
			return FALSE;
		if (g_NtVer.uMajor == 6 && g_NtVer.uMinor < 2)
			return FALSE;
		//--------图标-------
		int idxDpiVer;
		if (iDpi > 144)
			idxDpiVer = 3;
		else if (iDpi > 120)
			idxDpiVer = 2;
		else if (iDpi > 96)
			idxDpiVer = 1;
		else
			idxDpiVer = 0;

		const int idxGlyph =
			(bDark ? DWMWB_DARKBEGIN : DWMWB_LIGHTBEGIN) +
			(int)ePart * 4 +
			idxDpiVer +
			(g_NtVer.uMajor == 6 && (g_NtVer.uMinor == 2 || g_NtVer.uMinor == 3) ? 1 : 0);
		const auto& e = m_Item[idxGlyph];
		// 图标都没有Margin，无需处理
		const int cy = e.rcInAtlas.bottom - e.rcInAtlas.top;
		rc.left = e.rcInAtlas.left;
		rc.right = e.rcInAtlas.right;
		rc.top = e.rcInAtlas.top + cy * (int)eState / e.cSubImage;
		rc.bottom = rc.top + cy / e.cSubImage;
		//---------背景-------
		int idxBkg;
		if (ePart == DwmWndPart::Close)
		{
			if (bActive)
				idxBkg = BUTTONACTIVECLOSE;
			else
				idxBkg = BUTTONINACTIVECLOSE;
		}
		else
		{
			if (bActive)
				idxBkg = (bDark ? BUTTONACTIVECAPTIONDARK : BUTTONACTIVECAPTION);
			else
				idxBkg = (bDark ? BUTTONINACTIVECAPTIONDARK : BUTTONINACTIVECAPTION);
		}
		const auto& f = m_Item[idxBkg];
		RECT rcReal{ f.rcInAtlas };
		rcReal.left += f.mgContent.cxLeftWidth;
		rcReal.right -= f.mgContent.cxRightWidth;
		rcReal.top += f.mgContent.cyTopHeight;
		rcReal.bottom -= f.mgContent.cyBottomHeight;
		const int cy2 = rcReal.bottom - rcReal.top;
		rcBkg.left = rcReal.left;
		rcBkg.right = rcReal.right;
		rcBkg.top = rcReal.top + cy2 * (int)eState / f.cSubImage;
		rcBkg.bottom = rcBkg.top + cy2 / f.cSubImage;
		if (pExtra)
		{
			pExtra->idxGlyph = idxGlyph;
			pExtra->idxBkg = idxBkg;
			pExtra->pGlyph = &e;
			pExtra->pBkg = &f;
		}
		return TRUE;
	}

	EckInline constexpr const auto& AtPart(int idx) const { return m_Item[idx]; }

	EckInline constexpr HTHEME GetHTheme() const { return m_hTheme; }
};
ECK_NAMESPACE_END