#pragma once
#include "NativeWrapper.h"

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

struct UDW_PART
{
    RECT rcInAtlas;     // 在图集中的位置
    MARGINS mgSizing;   // 九宫信息
    MARGINS mgContent;  // 内容边距
    int cSubImage;      // 子图个数
};

struct UDW_EXTRA
{
    int idxGlyph;
    int idxBkg;
    const UDW_PART* pGlyph;
    const UDW_PART* pBkg;
};

enum class UdwPart : BYTE
{
    Close,
    Help,
    Max,
    Min,
    Restore,

    Invalid,// 用于命中测试
    Extra   // 用于命中测试
};

enum class UdwState : BYTE
{
    Normal,
    Hot,
    Pressed,
    Disabled
};

class CUxDwmWindowTheme
{
public:
    constexpr static int PartCount = 91;
private:
    UDW_PART m_vItem[PartCount]{};
    HTHEME m_hTheme{};
    HINSTANCE m_hInstStyle{};

    void QueryAtlasInfomation(int iPart, _Out_ UDW_PART& Info) noexcept
    {
        GetThemeRect(m_hTheme, iPart, 0, TMT_ATLASRECT, &Info.rcInAtlas);
        GetThemeMargins(m_hTheme, nullptr, iPart, 0, TMT_CONTENTMARGINS, nullptr, &Info.mgContent);
        GetThemeMargins(m_hTheme, nullptr, iPart, 0, TMT_SIZINGMARGINS, nullptr, &Info.mgSizing);
        GetThemeInt(m_hTheme, iPart, 0, TMT_IMAGECOUNT, &Info.cSubImage);
    }
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CUxDwmWindowTheme);

    ~CUxDwmWindowTheme() { Clear(); }

    void Clear() noexcept
    {
        if (m_hTheme)
        {
            CloseThemeData(m_hTheme);
            m_hTheme = nullptr;
        }
        if (m_hInstStyle)
        {
            FreeLibrary(m_hInstStyle);
            m_hInstStyle = nullptr;
        }
    }

    BOOL LoadDefaultTheme() noexcept
    {
        Clear();
        constexpr WCHAR ThemeFile[]{ LR"(\Resources\Themes\aero\aero.msstyles)" };
        const auto pszWinDir = NaGetNtSystemRoot();
        const size_t cchWinDir = wcslen(pszWinDir);

        const size_t cchBuf = cchWinDir + ARRAYSIZE(ThemeFile);
#pragma warning(suppress:6255)
        const auto pszBuf = (PWSTR)_alloca(cchBuf * sizeof(WCHAR));
        wmemcpy(pszBuf, pszWinDir, cchWinDir);
        wmemcpy(pszBuf + cchWinDir, ThemeFile, ARRAYSIZE(ThemeFile));

        m_hInstStyle = LoadLibraryExW(pszBuf, 0, LOAD_LIBRARY_AS_DATAFILE);
        if (!m_hInstStyle)
            return FALSE;
        m_hTheme = OpenThemeData(nullptr, L"DWMWINDOW");
        if (!m_hTheme)
        {
            FreeLibrary(m_hInstStyle);
            m_hInstStyle = nullptr;
            return FALSE;
        }

        for (int i = 0; i < PartCount; ++i)
            QueryAtlasInfomation(i, m_vItem[i]);
        return TRUE;
    }

    std::span<const BYTE> GetAtlasImageData(
        _Out_opt_ HRESULT* phr = nullptr) const noexcept
    {
        void* p{};
        DWORD cb{};
        const auto hr = GetThemeStream(m_hTheme, 0, 0,
            TMT_DISKSTREAM, &p, &cb, m_hInstStyle);
        if (phr)
            *phr = hr;
        return { (PCBYTE)p, cb };
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
    BOOL GetPartRect(
        _Out_ RECT& rc,
        _Out_ RECT& rcBkg,
        UdwPart ePart,
        UdwState eState,
        BOOL bDark,
        BOOL bActive,
        int iDpi,
        _Out_opt_ UDW_EXTRA* pExtra = nullptr) const noexcept
    {
        if (// 暗色模式需要1809+
            (g_NtVersion.uBuild < WINVER_1809 && bDark) ||
            // 暂不支持Win7及以下版本
            (g_NtVersion.uMajor < 6) ||
            (g_NtVersion.uMajor == 6 && g_NtVersion.uMinor < 2))
        {
            rc = {};
            rcBkg = {};
            if (pExtra)
                *pExtra = {};
            return FALSE;
        }
        //
        // 图标
        //
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
            (g_NtVersion.uMajor == 6 && (g_NtVersion.uMinor == 2 || g_NtVersion.uMinor == 3) ? 1 : 0);
        const auto& Glyph = m_vItem[idxGlyph];
        // 图标都没有Margin，无需处理
        const int cy = Glyph.rcInAtlas.bottom - Glyph.rcInAtlas.top;
        rc.left = Glyph.rcInAtlas.left;
        rc.right = Glyph.rcInAtlas.right;
        rc.top = Glyph.rcInAtlas.top + cy * (int)eState / Glyph.cSubImage;
        rc.bottom = rc.top + cy / Glyph.cSubImage;
        //
        // 背景
        //
        int idxBkg;
        if (ePart == UdwPart::Close)
            if (bActive)
                idxBkg = BUTTONACTIVECLOSE;
            else
                idxBkg = BUTTONINACTIVECLOSE;
        else
            if (bActive)
                idxBkg = (bDark ? BUTTONACTIVECAPTIONDARK : BUTTONACTIVECAPTION);
            else
                idxBkg = (bDark ? BUTTONINACTIVECAPTIONDARK : BUTTONINACTIVECAPTION);
        const auto& Bkg = m_vItem[idxBkg];
        RECT rcReal{ Bkg.rcInAtlas };
        rcReal.left += Bkg.mgContent.cxLeftWidth;
        rcReal.right -= Bkg.mgContent.cxRightWidth;
        rcReal.top += Bkg.mgContent.cyTopHeight;
        rcReal.bottom -= Bkg.mgContent.cyBottomHeight;
        const int cy2 = rcReal.bottom - rcReal.top;
        rcBkg.left = rcReal.left;
        rcBkg.right = rcReal.right;
        rcBkg.top = rcReal.top + cy2 * (int)eState / Bkg.cSubImage;
        rcBkg.bottom = rcBkg.top + cy2 / Bkg.cSubImage;
        if (pExtra)
        {
            pExtra->idxGlyph = idxGlyph;
            pExtra->idxBkg = idxBkg;
            pExtra->pGlyph = &Glyph;
            pExtra->pBkg = &Bkg;
        }
        return TRUE;
    }

    EckInlineNdCe auto& AtPart(int idx) const noexcept { return m_vItem[idx]; }

    EckInlineNdCe HTHEME GetHTheme() const noexcept { return m_hTheme; }
};
ECK_NAMESPACE_END