#pragma once
#include "UiElement.h"
#include "CReferenceCounted.h"

#define ECK_EUI_NAMESPACE_BEGIN namespace Eui {
#define ECK_EUI_NAMESPACE_END   }

ECK_NAMESPACE_BEGIN
ECK_EUI_NAMESPACE_BEGIN
using namespace UiBasic::Declaration;
enum : UINT
{
    DES_NO_CLIP = 1u << 31,// BeginPaint不应剪辑
};

enum : UINT
{
    // 以下三组标志互斥，且组内互斥

    RDF_GDI_GEO = 1u << 0,  // 总使用GDI绘制图形
    RDF_GDIX_GEO = 1u << 1, // 总使用GDI+绘制图形

    RDF_GDI_TEXT = 1u << 2, // 总使用GDI绘制文本
    RDF_GDIX_TEXT = 1u << 3,// 总使用GDI+绘制文本

    // 仅必要时使用GDI+（如抗锯齿），其余情况总使用GDI
    RDF_PREFER_GDI = 1u << 4,

    // 渲染缓冲的Alpha通道必须正确
    // 应用程序不应同时设置任何使用GDI的标志
    RDF_ALPHA = 1u << 5,
};

enum : UINT
{
    NMC_COMMAND,
};

struct PAINTINFO
{
    RECT rcOldClip;
    RECT rcClipInClient;
};

class CImage final : public CReferenceCountedT<CImage>
{
private:
    int m_cxGdiBitmap{}, m_cyGdiBitmap{};
    HBITMAP m_hbm{};
    GpImage* m_pImg{};
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CImage);
    ~CImage()
    {
        if (m_hbm)
            DeleteObject(m_hbm);
        if (m_pImg)
            GdipDisposeImage(m_pImg);
    }

    EckInlineNdCe HBITMAP GdiDetach() noexcept
    {
        const auto t = m_hbm;
        m_hbm = nullptr;
        return t;
    }
    EckInline void GdiAttach(HBITMAP h) noexcept
    {
        std::swap(h, m_hbm);
        if (h)
            DeleteObject(h);
        if (m_hbm)
        {
            BITMAP bm;
            GetObjectW(m_hbm, sizeof(bm), &bm);
            m_cxGdiBitmap = bm.bmWidth;
            m_cyGdiBitmap = bm.bmHeight;
        }
    }

    EckInlineNdCe GpImage* GpDetach() noexcept
    {
        const auto t = m_pImg;
        m_pImg = nullptr;
        return t;
    }
    EckInline void GpAttach(GpImage* p) noexcept
    {
        std::swap(p, m_pImg);
        if (p)
            GdipDisposeImage(p);
    }

    EckInlineNdCe auto GdiGet() const noexcept { return m_hbm; }
    EckInlineNdCe auto GdiGetWidth() const noexcept { return m_cxGdiBitmap; }
    EckInlineNdCe auto GdiGetHeight() const noexcept { return m_cyGdiBitmap; }
    EckInlineNdCe auto GpGet() const noexcept { return m_pImg; }
};

class CFont final : public CReferenceCountedT<CFont>
{
private:
    HFONT m_hFont{};
    GpFont* m_pFont{};
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CFont);
    ~CFont()
    {
        if (m_hFont)
            DeleteObject(m_hFont);
        if (m_pFont)
            GdipDeleteFont(m_pFont);
    }

    EckInlineNdCe HFONT GdiDetach() noexcept
    {
        const auto t = m_hFont;
        m_hFont = nullptr;
        return t;
    }
    EckInline void GdiAttach(HFONT h) noexcept
    {
        std::swap(h, m_hFont);
        if (h)
            DeleteObject(h);
    }

    EckInlineNdCe GpFont* GpDetach() noexcept
    {
        const auto t = m_pFont;
        m_pFont = nullptr;
        return t;
    }
    EckInline void GpAttach(GpFont* p) noexcept
    {
        std::swap(p, m_pFont);
        if (p)
            GdipDeleteFont(p);
    }

    EckInlineNdCe auto GdiGet() const noexcept { return m_hFont; }
    EckInlineNdCe auto GpGet() const noexcept { return m_pFont; }
};
ECK_EUI_NAMESPACE_END
ECK_NAMESPACE_END