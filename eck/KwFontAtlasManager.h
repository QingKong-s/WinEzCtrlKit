#pragma once
#include "RefPtr.h"
#include "KwMirroredAtlas.h"

ECK_NAMESPACE_BEGIN
KW2D_NAMESPACE_BEGIN
class CFontAtlasManager
{
public:
    using TId = CMirroredAtlas::TId;

    constexpr static TId IdInvalid = CMirroredAtlas::IdInvalid;
    constexpr static TId IdEmpty = IdInvalid - 1;

    constexpr static float SubPixelSnapCount = 4.f;// 包括0，不包括1

    struct SUBPIXEL
    {
        float xOrg;
        BYTE nSubPixel;
    };
private:
    class GlyphKey
    {
    private:
        BYTE nSubPixel;
        BYTE eRenderingMode;
        USHORT idxGlyph;
        UINT cyEm;
    public:
        static GlyphKey Make(
            UINT16 idxGlyph,
            float cyEm,
            DWRITE_RENDERING_MODE eRenderingMode,
            BYTE nSubPixel) noexcept
        {
            GlyphKey Key;
            Key.nSubPixel = nSubPixel;
            Key.eRenderingMode = (BYTE)eRenderingMode;
            Key.idxGlyph = idxGlyph;
            Key.cyEm = UINT(cyEm * 100.f);
            return Key;
        }

        EckInlineNdCe ULONGLONG Key() const noexcept { return *(ULONGLONG*)this; }

        EckInlineNdCe std::strong_ordering operator<=>(const GlyphKey& r) const noexcept
        {
            return Key() <=> r.Key();
        }
        EckInlineNdCe bool operator==(const GlyphKey& r) const noexcept
        {
            return Key() == r.Key();
        }
    };
    static_assert(sizeof(GlyphKey) == sizeof(ULONGLONG));

    struct Glyph
    {
        GlyphKey Key;
        CMirroredAtlas::TId idAtlasRect;
        short x, y;
        USHORT cx, cy;
        UINT nTouch;
    };

    using TGlyphIterator = CTrivialBuffer<Glyph>::TIterator;

    struct FontFace
    {
        // WARNING 此字段仅用作键，随时可能失效，因此绝对不能访问
        IDWriteFontFace* pFontFace{};
        CTrivialBuffer<Glyph> vGlyph{};
        UINT nTouch{};
        UINT nMaxGlyphTouch{};

        EckInlineNdCe std::strong_ordering operator<=>(const FontFace& r) const noexcept
        {
            return pFontFace <=> r.pFontFace;
        }

        void GlyphInsert(TGlyphIterator it, GlyphKey Key,
            TId id, const ::RECT& rcTexBounds) noexcept
        {
#ifdef _DEBUG
            if (it != vGlyph.end())
                EckAssert(it->Key != Key);
#endif
            vGlyph.Insert(size_t(it - vGlyph.begin()),
                {
                    Key, id,
                    (short)rcTexBounds.left,
                    (short)rcTexBounds.top,
                    USHORT(rcTexBounds.right - rcTexBounds.left),
                    USHORT(rcTexBounds.bottom - rcTexBounds.top),
                    nTouch
                });
        }

        TGlyphIterator GlyphFind(GlyphKey Key, _Out_ TId& id) noexcept
        {
            const auto it = std::lower_bound(
                vGlyph.begin(), vGlyph.end(), Key,
                [](const Glyph& e, GlyphKey Key) -> bool
                {
                    return e.Key < Key;
                });
            if (it != vGlyph.end() && it->Key == Key)
            {
                id = it->idAtlasRect;
                it->nTouch = nTouch;
            }
            else
                id = CMirroredAtlas::IdInvalid;
            return it;
        }
    };

    using TFontFaceIterator = std::vector<FontFace>::iterator;


    RefPtr<CMirroredAtlas> m_pAtlas{};
    CTrivialBuffer<BYTE> m_vTemp{};
    std::vector<FontFace> m_vFontFace{};
    UINT m_cTouch{};
    UINT m_cDiscardThreshold{ 1000 };

    TFontFaceIterator FontFaceFind(IDWriteFontFace* pFontFace) noexcept
    {
        EckAssert(pFontFace);
        const auto it = std::lower_bound(
            m_vFontFace.begin(), m_vFontFace.end(), pFontFace,
            [](const FontFace& e, IDWriteFontFace* pFontFace) -> bool
            {
                return e.pFontFace < pFontFace;
            });
        if (it != m_vFontFace.end() && it->pFontFace == pFontFace)
            return it;
        else
            return m_vFontFace.emplace(it, pFontFace);
    }

    HRESULT MskpCreateGrayscale(
        IDWriteGlyphRunAnalysis* pAnalysis,
        const RECT& rcTexBounds,
        _Out_ TId& id) noexcept
    {
        const auto cx = UINT(rcTexBounds.right - rcTexBounds.left);
        const auto cy = UINT(rcTexBounds.bottom - rcTexBounds.top);
        m_vTemp.ReSize(cx * cy);

        const auto hr = pAnalysis->CreateAlphaTexture(
            DWRITE_TEXTURE_ALIASED_1x1,
            &rcTexBounds,
            m_vTemp.Data(),
            (UINT)m_vTemp.Size());
        if (FAILED(hr))
        {
            id = CMirroredAtlas::IdInvalid;
            return hr;
        }

        CMirroredAtlas::TCoord x, y;
        id = m_pAtlas->Allocate(cx + 2, cy + 2, x, y, TRUE);
        ++x, ++y;
        auto& Tex = m_pAtlas->GetMirroredTexture();

        EckCounter(cy, j)
        {
            EckCounter(cx, i)
            {
                *Tex.CpuData<UINT>(x + i, y + j) =
                    UINT(0x00FF'FFFF | (m_vTemp[j * cx + i] << 24));
            }
        }
        return S_OK;
    }
    HRESULT MskpCreateClearType(
        IDWriteGlyphRunAnalysis* pAnalysis,
        const RECT& rcTexBounds,
        _Out_ TId& id) noexcept
    {
        const auto cx = UINT(rcTexBounds.right - rcTexBounds.left);
        const auto cy = UINT(rcTexBounds.bottom - rcTexBounds.top);
        m_vTemp.ReSize(cx * cy * 3);

        const auto hr = pAnalysis->CreateAlphaTexture(
            DWRITE_TEXTURE_CLEARTYPE_3x1,
            &rcTexBounds,
            m_vTemp.Data(),
            (UINT)m_vTemp.Size());
        if (FAILED(hr))
        {
            id = CMirroredAtlas::IdInvalid;
            return hr;
        }

        CMirroredAtlas::TCoord x, y;
        id = m_pAtlas->Allocate(cx + 2, cy + 2, x, y, TRUE);
        ++x, ++y;
        auto& Tex = m_pAtlas->GetMirroredTexture();

        EckCounter(cy, j)
        {
            EckCounter(cx, i)
            {
                const auto idxTexel = (j * cx + i) * 3;
                const auto r = m_vTemp[idxTexel + 0];
                const auto g = m_vTemp[idxTexel + 1];
                const auto b = m_vTemp[idxTexel + 2];
                const auto byGray = BYTE(0.2126f * r + 0.7152f * g + 0.0722f * b);

                *Tex.CpuData<UINT>(x + i, y + j) =
                    UINT(0x00FF'FFFF | (byGray << 24));
            }
        }
        return S_OK;
    }

    HRESULT CreateGlyphAnalysis(
        _Out_ IDWriteGlyphRunAnalysis*& pGlyphRunAnalysis,
        _Out_ RECT& rcTexBounds,
        _Out_ BOOL& bClearType,
        float xOrg,
        float fScale,
        UINT16 idxInRun,
        DWRITE_RENDERING_MODE eRenderingMode,
        DWRITE_MEASURING_MODE eMeasuringMode,
        const DWRITE_GLYPH_RUN& Run) noexcept
    {
        HRESULT hr;
        const DWRITE_GLYPH_RUN SingleGlyph
        {
            .fontFace = Run.fontFace,
            .fontEmSize = Run.fontEmSize * fScale,
            .glyphCount = 1,
            .glyphIndices = &Run.glyphIndices[idxInRun],
            .isSideways = Run.isSideways,
            .bidiLevel = Run.bidiLevel
        };

#if ECK_OPT_DWRITE_V2
        if (g_pDwFactory2)
        {
            bClearType = FALSE;
            hr = g_pDwFactory2->CreateGlyphRunAnalysis(
                &SingleGlyph,
                nullptr,
                eRenderingMode,
                eMeasuringMode,
                DWRITE_GRID_FIT_MODE_DISABLED,
                DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE,
                xOrg, 0,
                &pGlyphRunAnalysis);
        }
        else
#endif// ECK_OPT_DWRITE_V2
        {
            bClearType = (eRenderingMode != DWRITE_RENDERING_MODE_ALIASED);
            hr = g_pDwFactory->CreateGlyphRunAnalysis(
                &SingleGlyph,
                1.f,
                nullptr,
                eRenderingMode,
                eMeasuringMode,
                xOrg, 0,
                &pGlyphRunAnalysis);
        }
        if (FAILED(hr))
            return hr;

        return pGlyphRunAnalysis->GetAlphaTextureBounds(
            bClearType ? DWRITE_TEXTURE_CLEARTYPE_3x1 : DWRITE_TEXTURE_ALIASED_1x1,
            &rcTexBounds);
    }
public:
    CFontAtlasManager() = default;
    CFontAtlasManager(RefPtr<CMirroredAtlas> pAtlas) noexcept
        : m_pAtlas{ std::move(pAtlas) }
    {}

    static SUBPIXEL SnapSubPixel(_Inout_ float& ox) noexcept
    {
        auto fDecimal = modff(ox, &ox);
        const auto bNeg = (fDecimal < 0);
        if (bNeg)
            fDecimal = -fDecimal;
        const auto fSubPixel = roundf(fDecimal * SubPixelSnapCount);
        if (fSubPixel >= SubPixelSnapCount)
        {
            if (bNeg)
                ox -= 1.f;
            else
                ox += 1.f;
            return { 0, 0 };
        }
        else
        {
            const auto x = fSubPixel / SubPixelSnapCount;
            return { bNeg ? -x : x, (BYTE)fSubPixel };
        }
    }

    /// <summary>
    /// 分析字形度量并将掩码缓存到图集中。
    /// <para/>
    /// 对于一个字形运行，调用方在渲染时将其中的每个字形传递到此函数
    /// <para/>
    /// WARNING 若DW指示当前字形边界无效，则函数成功且返回IdEmpty，
    /// 此时rcGlyph为空矩形，调用方跳过此字形的渲染
    /// </summary> 
    /// <param name="idAtlasRect">返回图集ID</param>
    /// <param name="rcGlyph">返回DW指示的字形边界，逻辑坐标。
    /// 此边界基于(SubPixel(ox), 0)基线原点和EM尺寸计算，调用方负责累加水平推进量并添加字形偏移
    /// </param>
    /// <param name="ox">
    /// 输入字形位置，逻辑坐标。函数将此值吸附到子像素网格并传递到DW，
    /// 函数返回后参数值已更改，调用方必须使用更改后的值进行后续计算
    /// </param>
    /// <param name="fScale">DPI缩放因子</param>
    /// <param name="idxInRun">字形运行中当前字形的索引</param>
    /// <param name="eRenderingMode">渲染模式，
    /// 若支持IDWriteFactory2(取决于编译选项和操作系统)，函数总使用灰度抗锯齿，否则总使用ClearType
    /// </param>
    /// <param name="eMeasuringMode">测量模式</param>
    /// <param name="Run">DW字形运行信息</param>
    /// <returns>HRESULT</returns>
    HRESULT RealizeGlyphAlphaMask(
        _Out_ TId& idAtlasRect,
        _Out_ Rect& rcGlyph,
        _Inout_ float& ox,
        float fScale,
        UINT16 idxInRun,
        DWRITE_RENDERING_MODE eRenderingMode,
        DWRITE_MEASURING_MODE eMeasuringMode,
        const DWRITE_GLYPH_RUN& Run) noexcept
    {
        ox *= fScale;
        const auto [xSnap, nSubPixel] = SnapSubPixel(ox);
        ox /= fScale;
        // 查找缓存
        const auto Key = GlyphKey::Make(
            Run.glyphIndices[idxInRun],
            Run.fontEmSize * fScale,
            eRenderingMode,
            nSubPixel);
        const auto itFontFace = FontFaceFind(Run.fontFace);
        itFontFace->nTouch = m_cTouch;
        const auto it = itFontFace->GlyphFind(Key, idAtlasRect);
        if (idAtlasRect != CMirroredAtlas::IdInvalid)
        {
            rcGlyph.left = (float)it->x / fScale;
            rcGlyph.top = (float)it->y / fScale;
            rcGlyph.right = float(it->x + it->cx) / fScale;
            rcGlyph.bottom = float(it->y + it->cy) / fScale;
            return S_OK;
        }
        // 若无缓存，则创建字形分析并生成掩码

        HRESULT hr;
        ComPtr<IDWriteGlyphRunAnalysis> pGlyphRunAnalysis;
        BOOL bClearType;
        RECT rcTexBounds;

        hr = CreateGlyphAnalysis(
            pGlyphRunAnalysis.RefOf(),
            rcTexBounds,
            bClearType,
            xSnap,
            fScale,
            idxInRun,
            eRenderingMode,
            eMeasuringMode,
            Run);

        rcGlyph.left = (float)rcTexBounds.left / fScale;
        rcGlyph.top = (float)rcTexBounds.top / fScale;
        rcGlyph.right = (float)rcTexBounds.right / fScale;
        rcGlyph.bottom = (float)rcTexBounds.bottom / fScale;

        if (IsRectEmpty(rcTexBounds))
        {
            itFontFace->GlyphInsert(it, Key, IdEmpty, rcTexBounds);
            idAtlasRect = IdEmpty;
            return S_OK;
        }

        TId id;
        if (bClearType)
            hr = MskpCreateClearType(pGlyphRunAnalysis.Get(), rcTexBounds, id);
        else
            hr = MskpCreateGrayscale(pGlyphRunAnalysis.Get(), rcTexBounds, id);
        idAtlasRect = id;
        if (FAILED(hr))
            return hr;

        itFontFace->GlyphInsert(it, Key, id, rcTexBounds);
        return S_OK;
    }

    void DiscardLeastRecentlyUsed(UINT cThreshold) noexcept
    {
        for (auto itFace = m_vFontFace.begin(); itFace != m_vFontFace.end(); )
        {
            if (itFace->nTouch + cThreshold < m_cTouch)
            {
                for (const auto& e : itFace->vGlyph)
                    if (e.idAtlasRect != IdEmpty)
                        m_pAtlas->Discard(e.idAtlasRect);
                itFace = m_vFontFace.erase(itFace);
            }
            else
            {
                auto& vGlyph = itFace->vGlyph;
                size_t j = 0;
                for (size_t i = 0; i < vGlyph.Size(); ++i)
                {
                    auto& e = vGlyph[i];
                    if (e.idAtlasRect != IdEmpty &&
                        e.nTouch + cThreshold < m_cTouch)
                        m_pAtlas->Discard(e.idAtlasRect);
                    else
                    {
                        if (j != i)
                            vGlyph[j] = e;
                        j++;
                    }
                }
                vGlyph.ReSize(j);
                ++itFace;
            }
        }
    }

    EckInlineNd auto GetAtlas() const noexcept { return m_pAtlas; }
    EckInline void SetAtlas(RefPtr<CMirroredAtlas> pAtlas) noexcept
    {
        m_pAtlas = std::move(pAtlas);
    }

    EckInlineCe void TouchFrame(UINT cThreshold) noexcept
    {
        if (++m_cTouch > m_cDiscardThreshold)
            DiscardLeastRecentlyUsed(cThreshold);
    }
    EckInlineCe void SetDiscardThreshold(UINT c) noexcept { m_cDiscardThreshold = c; }
    EckInlineNdCe UINT GetDiscardThreshold() const noexcept { return m_cDiscardThreshold; }
};
KW2D_NAMESPACE_END
ECK_NAMESPACE_END