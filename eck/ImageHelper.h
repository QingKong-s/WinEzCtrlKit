#pragma once
#include "Utility.h"
#include "CStreamView.h"
#include "ComPtr.h"
#include "NativeWrapper.h"
#if !ECK_OPT_NO_GDIPLUS
#include "GpPtr.h"
#endif // !ECK_OPT_NO_GDIPLUS
#include "CSrwLock.h"
#include "AutoPtrDef.h"

ECK_NAMESPACE_BEGIN
enum class ImageType
{
    Unknown,
    Bmp,
    Gif,
    Jpeg,
    Png,
    Tiff,
    Wdp,
    Dds,

    MaxValue
};

constexpr inline PCWSTR GpImageMime[]
{
    L"",
    L"image/bmp",
    L"image/gif",
    L"image/jpeg",
    L"image/png",
    L"image/tiff",
    L"image/vnd.ms-photo",// wdp
    L"image/vnd.ms-dds",// dds
};

const inline GUID WicEncoderGuid[]
{
    GUID_NULL,
    GUID_ContainerFormatBmp,
    GUID_ContainerFormatGif,
    GUID_ContainerFormatJpeg,
    GUID_ContainerFormatPng,
    GUID_ContainerFormatTiff,
    GUID_ContainerFormatWmp,
    GUID_ContainerFormatDds,
};

static_assert(
    (int)ImageType::MaxValue == ARRAYSIZE(GpImageMime) &&
    ARRAYSIZE(GpImageMime) == ARRAYSIZE(WicEncoderGuid));


#pragma region Wic
// 32bppPBGRA
constexpr inline GUID DefaultWicPixelFormat
ECK_GUID(0x6fddc324, 0x4e03, 0x4bfe, 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x10);

EckInline const GUID& WicImageTypeToGuid(ImageType e)
{
    return WicEncoderGuid[(size_t)e];
}

EckInline HRESULT WicCreateDecoder(
    _Out_ IWICBitmapDecoder*& pDecoder,
    _In_z_ PCWSTR pszFile,
    DWORD dwAccess = GENERIC_READ) noexcept
{
    return g_pWicFactory->CreateDecoderFromFilename(
        pszFile, nullptr, dwAccess,
        WICDecodeMetadataCacheOnDemand, &pDecoder);
}
EckInline HRESULT WicCreateDecoder(
    _Out_ IWICBitmapDecoder*& pDecoder,
    _In_ IStream* pStream) noexcept
{
    return g_pWicFactory->CreateDecoderFromStream(
        pStream, nullptr,
        WICDecodeMetadataCacheOnDemand, &pDecoder);
}

// 从解码器加载所有帧
// 若要保留帧，回调函数必须对得到的位图源添加引用
inline HRESULT WicLoadSourceMultiFrame(
    std::invocable<IWICBitmapSource*> auto&& Fn,
    _In_ IWICBitmapDecoder* pDecoder,
    const GUID& guidFmt = DefaultWicPixelFormat) noexcept
{
    HRESULT hr;
    UINT cFrame;
    pDecoder->GetFrameCount(&cFrame);
    GUID guidSrcFmt;
    EckCounter(cFrame, i)
    {
        ComPtr<IWICBitmapFrameDecode> pFrameDecoder;
        if (FAILED(hr = pDecoder->GetFrame(i, &pFrameDecoder)))
            return hr;

        ComPtr<IWICBitmapSource> pSource;
        pFrameDecoder->GetPixelFormat(&guidSrcFmt);
        if (guidSrcFmt == guidFmt)
            pSource.Attach(pFrameDecoder.Detach());
        else
        {
            ComPtr<IWICFormatConverter> pConverter;
            if (FAILED(hr = g_pWicFactory->CreateFormatConverter(&pConverter)))
                return hr;
            hr = pConverter->Initialize(
                pFrameDecoder.Get(),
                guidFmt,
                WICBitmapDitherTypeNone,
                nullptr, 0.0f,
                WICBitmapPaletteTypeMedianCut);
            if (FAILED(hr))
                return hr;
            pSource.Attach(pConverter.Detach());
        }

        EckCanCallbackContinue(Fn(pSource.Get()))
            break;
    }
    return S_OK;
}

inline HRESULT WicLoadSource(
    _Out_ IWICBitmapSource*& pSource_,
    _In_ IWICBitmapDecoder* pDecoder,
    int cxNew = -1, int cyNew = -1,
    const GUID& guidFmt = DefaultWicPixelFormat,
    WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear) noexcept
{
    pSource_ = nullptr;
    HRESULT hr;
    ComPtr<IWICBitmapFrameDecode> pFrameDecoder;
    if (FAILED(hr = pDecoder->GetFrame(0, &pFrameDecoder)))
        return hr;

    ComPtr<IWICBitmapSource> pSource;
    GUID guidSrcFmt;
    pFrameDecoder->GetPixelFormat(&guidSrcFmt);
    if (guidSrcFmt == guidFmt)
        pSource.Attach(pFrameDecoder.Detach());
    else
    {
        ComPtr<IWICFormatConverter> pConverter;
        if (FAILED(hr = g_pWicFactory->CreateFormatConverter(&pConverter)))
            return hr;
        hr = pConverter->Initialize(
            pFrameDecoder.Get(),
            guidFmt,
            WICBitmapDitherTypeNone,
            nullptr, 0.0f,
            WICBitmapPaletteTypeMedianCut);
        if (FAILED(hr))
            return hr;
        pSource.Attach(pConverter.Detach());
    }

    if (cxNew > 0 && cyNew > 0)
    {
        ComPtr<IWICBitmapScaler> pScaler;
        if (FAILED(hr = g_pWicFactory->CreateBitmapScaler(&pScaler)))
            return hr;
        hr = pScaler->Initialize(pSource.Get(), cxNew, cyNew, eInterMode);
        if (FAILED(hr))
            return hr;
        pSource.Attach(pScaler.Detach());
    }
    pSource_ = pSource.Detach();
    return S_OK;
}

inline HRESULT WicLoadSource(
    _Out_ IWICBitmapSource*& pSource,
    _In_z_ PCWSTR pszFile,
    int cxNew = -1, int cyNew = -1,
    const GUID& guidFmt = DefaultWicPixelFormat,
    WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear) noexcept
{
    ComPtr<IWICBitmapDecoder> pDecoder;
    const auto hr = WicCreateDecoder(pDecoder.RefOf(), pszFile);
    if (FAILED(hr))
    {
        pSource = nullptr;
        return hr;
    }
    return WicLoadSource(pSource, pDecoder.Get(), cxNew, cyNew, guidFmt, eInterMode);
}

inline HRESULT WicLoadSource(
    _Out_ IWICBitmapSource*& pSource,
    _In_ IStream* pStream,
    int cxNew = -1, int cyNew = -1,
    const GUID& guidFmt = DefaultWicPixelFormat,
    WICBitmapInterpolationMode eInterMode = WICBitmapInterpolationModeLinear) noexcept
{
    ComPtr<IWICBitmapDecoder> pDecoder;
    const auto hr = WicCreateDecoder(pDecoder.RefOf(), pStream);
    if (FAILED(hr))
    {
        pSource = nullptr;
        return hr;
    }
    return WicLoadSource(pSource, pDecoder.Get(), cxNew, cyNew, guidFmt, eInterMode);
}

inline HRESULT WicSaveSource(
    _In_ IStream* pStream,
    _In_ IWICBitmapSource* pSource,
    ImageType eType = ImageType::Png) noexcept
{
    HRESULT hr;
    ComPtr<IWICBitmapEncoder> pEncoder;
    if (FAILED(hr = g_pWicFactory->CreateEncoder(
        WicImageTypeToGuid(eType), nullptr, &pEncoder)))
        return hr;
    if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
        return hr;
    ComPtr<IWICBitmapFrameEncode> pFrame;
    if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, nullptr)))
        return hr;
    if (FAILED(hr = pFrame->Initialize(nullptr)))
        return hr;
    if (FAILED(hr = pFrame->WriteSource(pSource, nullptr)))
        return hr;
    if (FAILED(hr = pFrame->Commit()))
        return hr;
    return pEncoder->Commit();
}

inline HRESULT WicSaveSource(
    CByteBuffer& rb,
    _In_ IWICBitmapSource* pSource,
    ImageType eType = ImageType::Png) noexcept
{
    CStreamView Stream{ rb };
    const auto hr = WicSaveSource(&Stream, pSource, eType);
    Stream.AssertReference(1);
    return hr;
}

inline HRESULT WicSaveSource(
    _In_z_ PCWSTR pszFile,
    _In_ IWICBitmap* pBitmap,
    ImageType eType = ImageType::Png) noexcept
{
    HRESULT hr;
    ComPtr<IWICStream> pStream;
    if (FAILED(hr = g_pWicFactory->CreateStream(&pStream)))
        return hr;
    if (FAILED(hr = pStream->InitializeFromFilename(pszFile, GENERIC_WRITE)))
        return hr;
    return WicSaveSource(pStream.Get(), pBitmap, eType);
}

// 位图源的像素格式原则上必须是32bppPBGRA，但考虑到完全不透明图像，
// 支持32bppBGRA的写入，调用方负责进行必要的检查以防止透明图像被错误处理。
// 不支持其他像素格式，返回WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT。
// 此函数总创建自上而下DIB节
inline HRESULT WicCreateDibSection(
    _Out_ HBITMAP& hDib,
    _In_ IWICBitmapSource* pSource) noexcept
{
    hDib = nullptr;
    HRESULT hr;
    UINT cx, cy;
    hr = pSource->GetSize(&cx, &cy);
    if (FAILED(hr))
        return hr;
    GUID guidFmt;
    hr = pSource->GetPixelFormat(&guidFmt);
    if (FAILED(hr))
        return hr;

    if (guidFmt != GUID_WICPixelFormat32bppPBGRA &&
        guidFmt != GUID_WICPixelFormat32bppBGRA)
        return WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = (LONG)cx;
    bmi.bmiHeader.biHeight = -(LONG)cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    void* pDibBits;
    const auto hBitmap = CreateDIBSection(
        nullptr, &bmi, DIB_RGB_COLORS, &pDibBits, nullptr, 0);
    if (!hBitmap)
        return HRESULT_FROM_WIN32(NaGetLastError());
    hr = pSource->CopyPixels(nullptr, cx * 4, cx * cy * 4, (BYTE*)pDibBits);
    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return hr;
    }
    hDib = hBitmap;
    return S_OK;
}

inline HRESULT WicCreateIcon(
    _Out_ HICON& hIcon,
    _In_ IWICBitmapSource* pSource,
    HBITMAP hbmMask = nullptr) noexcept
{
    hIcon = nullptr;
    HRESULT hr;
    HBITMAP hbmColor;
    hr = WicCreateDibSection(hbmColor, pSource);
    if (FAILED(hr))
        return hr;

    BOOL bDeleteMask{};
    if (!hbmMask)
    {
        UINT cx, cy;
        pSource->GetSize(&cx, &cy);
        const auto cbBuf = (((cx * 1 * 1 + 15) >> 4) << 1) * cy;
        const auto pWhiteBuffer = std::make_unique<BYTE[]>(cbBuf);
        memset(pWhiteBuffer.get(), 0xFF, cbBuf);
        hbmMask = CreateBitmap((int)cx, (int)cy, 1, 1, pWhiteBuffer.get());
        if (!hbmMask)
        {
            DeleteObject(hbmColor);
            return HRESULT_FROM_WIN32(NaGetLastError());
        }
        bDeleteMask = TRUE;
    }
    const ICONINFO ii
    {
        .fIcon = TRUE,
        .hbmMask = hbmMask,
        .hbmColor = hbmColor
    };
    hIcon = CreateIconIndirect((ICONINFO*)&ii);
    hr = HRESULT_FROM_WIN32(NaGetLastError());
    DeleteObject(hbmColor);
    if (bDeleteMask)
        DeleteObject(hbmMask);
    return hIcon ? S_OK : hr;
}
#pragma endregion Wic

#pragma region Gdiplus
#if !ECK_OPT_NO_GDIPLUS
inline GpStatus GpCreateGdiBitmap(
    _Out_ HBITMAP& hBitmap,
    _In_reads_bytes_(cbData) PCVOID pData,
    size_t cbData) noexcept
{
    hBitmap = nullptr;
    GpStatus gps;
    CStreamView Stream{ pData, cbData };
    {
        GpPtr<GpBitmap> pBitmap;
        gps = GdipCreateBitmapFromStream(&Stream, &pBitmap);
        if (gps != Gdiplus::Ok)
            return gps;
        gps = GdipCreateHBITMAPFromBitmap(pBitmap.Get(), &hBitmap, 0);
    }
    Stream.AssertReference(1);
    return gps;
}

inline GpStatus GpCreateGdiBitmap(
    _Out_ HBITMAP& hBitmap,
    _In_z_ PCWSTR pszFile) noexcept
{
    hBitmap = nullptr;
    GpPtr<GpBitmap> pBitmap;
    const auto gps = GdipCreateBitmapFromFile(pszFile, &pBitmap);
    if (gps != Gdiplus::Ok)
        return gps;
    return GdipCreateHBITMAPFromBitmap(pBitmap.Get(), &hBitmap, 0);
}

inline GpStatus GpCreateIcon(
    _Out_ HICON& hIcon,
    _In_reads_bytes_(cbData) PCVOID pData,
    size_t cbData) noexcept
{
    hIcon = nullptr;
    GpStatus gps;
    CStreamView Stream{ pData, cbData };
    {
        GpPtr<GpBitmap> pBitmap;
        gps = GdipCreateBitmapFromStream(&Stream, &pBitmap);
        if (gps != Gdiplus::Ok)
            return gps;
        gps = GdipCreateHICONFromBitmap(pBitmap.Get(), &hIcon);
    }
    Stream.AssertReference(1);
    return gps;
}

inline GpStatus GpCreateIcon(
    _Out_ HICON& hIcon,
    _In_z_ PCWSTR pszFile) noexcept
{
    hIcon = nullptr;
    GpPtr<GpBitmap> pBitmap;
    const auto gps = GdipCreateBitmapFromFile(pszFile, &pBitmap);
    if (gps != Gdiplus::Ok)
        return gps;
    return GdipCreateHICONFromBitmap(pBitmap.Get(), &hIcon);
}

inline GpStatus GpScaleBitmap(
    _Out_ GpBitmap*& pNewBitmap,
    _In_ GpBitmap* pBitmap,
    int cxNew, int cyNew,
    Gdiplus::InterpolationMode eInter = Gdiplus::InterpolationModeDefault) noexcept
{
    pNewBitmap = nullptr;
    if (cxNew < 0 && cyNew < 0)
        return Gdiplus::InvalidParameter;
    UINT cxOrg, cyOrg;
    GdipGetImageWidth(pBitmap, &cxOrg);
    GdipGetImageHeight(pBitmap, &cyOrg);
    if (cxNew < 0)
        cxNew = (UINT64)cxOrg * (UINT64)cyNew / (UINT64)cyOrg;
    if (cyNew < 0)
        cyNew = (UINT64)cyOrg * (UINT64)cxNew / (UINT64)cxOrg;

    if (cxNew == cxOrg && cyNew == cyOrg)
        return GdipCloneImage(pBitmap, (GpImage**)&pNewBitmap);

    Gdiplus::PixelFormat ePixFmt;
    GdipGetImagePixelFormat(pBitmap, &ePixFmt);
    auto gps = GdipCreateBitmapFromScan0(
        cxNew, cyNew, 0, ePixFmt, nullptr, &pNewBitmap);
    if (gps != Gdiplus::Ok)
    {
        gps = GdipCreateBitmapFromScan0(
            cxNew, cyNew, 0, PixelFormat32bppPARGB, nullptr, &pNewBitmap);
        if (gps != Gdiplus::Ok)
            return gps;
    }

    GpPtr<GpGraphics> pGraphics;
    GdipGetImageGraphicsContext(pNewBitmap, &pGraphics);
    GdipSetInterpolationMode(pGraphics.Get(), eInter);
    GdipDrawImageRectRectI(pGraphics.Get(), pBitmap,
        0, 0, cxNew, cyNew,
        0, 0, cxOrg, cyOrg,
        Gdiplus::UnitPixel, nullptr, nullptr, nullptr);
    return Gdiplus::Ok;
}

namespace Priv
{
    class GpCodecCache
    {
    private:
        static inline UniquePtr<DelMA<Gdiplus::ImageCodecInfo[]>> s_pEncoders{};
        static inline UINT s_cEncoders{};
        static inline CSrwLock s_Lock{};
    public:
        static GpStatus ReloadEncoders() noexcept
        {
            GpStatus gps;
            UINT cEncoders, cbEncoders;
            gps = GdipGetImageEncodersSize(&cEncoders, &cbEncoders);
            if (gps != Gdiplus::Ok)
                return gps;

            auto pEncoders{ CrtMakeTrivialUnique<Gdiplus::ImageCodecInfo[]>(cbEncoders) };
            gps = GdipGetImageEncoders(cEncoders, cbEncoders, pEncoders.get());
            if (gps != Gdiplus::Ok)
                return gps;

            CSrwWriteGuard _{ s_Lock };
            s_pEncoders = std::move(pEncoders);
            s_cEncoders = cEncoders;
            return Gdiplus::Ok;
        }

        static BOOL EncoderFromMime(
            _In_z_ PCWSTR pszType, _Out_ CLSID& clsid) noexcept
        {
            clsid = {};
            s_Lock.EnterRead();
            if (!s_pEncoders)
            {
                s_Lock.LeaveRead();
                if (ReloadEncoders() != Gdiplus::Ok)
                    return FALSE;
                s_Lock.EnterRead();
            }

            BOOL b{};
            EckCounter(s_cEncoders, i)
            {
                if (!wcscmp(s_pEncoders[i].MimeType, pszType))
                {
                    clsid = s_pEncoders[i].Clsid;
                    b = TRUE;
                    break;
                }
            }
            s_Lock.LeaveRead();
            return b;
        }
    };

}

inline GpStatus GpGetEncoderClsidFromMime(
    _In_z_ PCWSTR pszType, _Out_ CLSID& clsid) noexcept
{
    if (!Priv::GpCodecCache::EncoderFromMime(pszType, clsid))
        return Gdiplus::UnknownImageFormat;
    return Gdiplus::Ok;
}

EckInlineNdCe PCWSTR GpImageTypeToMime(ImageType e) noexcept
{
    return GpImageMime[(size_t)e];
}

inline GpStatus GpSaveImage(
    _In_ IStream* pStream,
    _In_ GpImage* pImage,
    ImageType eType = ImageType::Png) noexcept
{
    CLSID clsid;
    const auto gps = GpGetEncoderClsidFromMime(GpImageTypeToMime(eType), clsid);
    if (gps != Gdiplus::Ok)
        return gps;
    return GdipSaveImageToStream(pImage, pStream, &clsid, nullptr);
}

inline GpStatus GpSaveImage(
    CByteBuffer& rb,
    _In_ Gdiplus::GpImage* pImage,
    ImageType eType = ImageType::Png) noexcept
{
    CStreamView Stream{ rb };
    const auto gps = GpSaveImage(&Stream, pImage, eType);
    Stream.AssertReference(1);
    return gps;
}

inline GpStatus GpSaveImage(
    _In_z_ PCWSTR pszFile,
    _In_ Gdiplus::GpImage* pImage,
    ImageType eType = ImageType::Png) noexcept
{
    CLSID clsid;
    const auto gps = GpGetEncoderClsidFromMime(GpImageTypeToMime(eType), clsid);
    if (gps != Gdiplus::Ok)
        return gps;
    return GdipSaveImageToFile(pImage, pszFile, &clsid, nullptr);
}
#endif// !ECK_OPT_NO_GDIPLUS
#pragma endregion Gdiplus

#if !ECK_OPT_NO_D2D
inline HRESULT D2DLoadBitmap(
    _Out_ ID2D1Bitmap*& pD2DBitmap,
    _In_z_ PCWSTR pszFile,
    _In_ ID2D1RenderTarget* pRT,
    int cxNew = -1, int cyNew = -1) noexcept
{
    pD2DBitmap = nullptr;
    HRESULT hr;
    ComPtr<IWICBitmapDecoder> pDecoder;
    if (FAILED(hr = WicCreateDecoder(pDecoder.RefOf(), pszFile)))
        return hr;
    ComPtr<IWICBitmapSource> pBitmap;
    if (FAILED(hr = WicLoadSource(pBitmap.RefOf(), pDecoder.Get(), cxNew, cyNew)))
        return hr;
    return pRT->CreateBitmapFromWicBitmap(pBitmap.Get(), &pD2DBitmap);
}
#endif// !ECK_OPT_NO_D2D

namespace Priv
{
    inline BYTE* GdiSaveDibHeader(
        CByteBuffer& rb,
        size_t cbStride,
        int cx, int cy,
        WORD cBitPerPixel = 32,
        UINT cPaletteEntry = 0,
        UINT cPaletteImportant = 0) noexcept
    {
        const size_t cbTotal =
            sizeof(BITMAPFILEHEADER) +
            sizeof(BITMAPINFOHEADER) +
            sizeof(RGBQUAD) * cPaletteEntry +
            cbStride * Abs(cy);
        const auto pfh = (BITMAPFILEHEADER*)rb.PushBackNoExtra(cbTotal);
        const auto pih = (BITMAPINFOHEADER*)(pfh + 1);
        // 制文件头
        pfh->bfType = 0x4D42;// BM
        pfh->bfSize = (DWORD)cbTotal;
        pfh->bfReserved1 = pfh->bfReserved2 = 0;
        pfh->bfOffBits =
            sizeof(BITMAPFILEHEADER) +
            sizeof(BITMAPINFOHEADER) +
            (cPaletteEntry * sizeof(RGBQUAD));
        // 制信息头
        pih->biSize = sizeof(BITMAPINFOHEADER);
        pih->biWidth = cx;
        pih->biHeight = cy;
        pih->biPlanes = 1;
        pih->biBitCount = cBitPerPixel;
        pih->biCompression = BI_RGB;
        pih->biSizeImage = DWORD(cbStride * Abs(cy));
        pih->biXPelsPerMeter = pih->biYPelsPerMeter = 3780;
        pih->biClrUsed = cPaletteEntry;
        pih->biClrImportant = cPaletteImportant;
        return (BYTE*)(pih + 1);
    }
}

inline BOOL GdiSaveDib(
    CByteBuffer& rb,
    void* pBits,
    size_t cbStride,
    int cx, int cy,
    WORD cBitPerPixel = 32,
    _In_reads_opt_(cPaletteEntry) const RGBQUAD* pPalette = nullptr,
    UINT cPaletteEntry = 0,
    UINT cPaletteImportant = 0) noexcept
{
    if (cPaletteEntry && !pPalette)
        return FALSE;
    auto p = Priv::GdiSaveDibHeader(rb, cbStride, cx, cy,
        cBitPerPixel, cPaletteEntry, cPaletteImportant);
    // 制颜色表
    const size_t cbPalette = sizeof(RGBQUAD) * cPaletteEntry;
    if (cbPalette)
    {
        memcpy(p, pPalette, cbPalette);
        p += cbPalette;
    }
    // 制像素
    memcpy(p, pBits, cbStride * Abs(cy));
    return TRUE;
}

inline BOOL GdiSaveDib(CByteBuffer& rb, HBITMAP hDib) noexcept
{
    DIBSECTION ds;
    if (!GetObjectW(hDib, sizeof(ds), &ds))
        return FALSE;

    auto p = Priv::GdiSaveDibHeader(
        rb, ds.dsBm.bmWidthBytes,
        ds.dsBmih.biWidth, ds.dsBmih.biHeight,
        ds.dsBmih.biBitCount,
        ds.dsBmih.biClrUsed, ds.dsBmih.biClrImportant);
    // 制颜色表
    const size_t cbPalette = sizeof(RGBQUAD) * ds.dsBmih.biClrUsed;
    if (cbPalette)
    {
        const auto hCDC = CreateCompatibleDC(nullptr);
        SelectObject(hCDC, hDib);
        GetDIBColorTable(hCDC, 0, ds.dsBmih.biClrUsed, (RGBQUAD*)p);
        DeleteDC(hCDC);
        p += cbPalette;
    }
    // 制像素
    memcpy(
        p,
        ds.dsBm.bmBits,
        ds.dsBm.bmWidthBytes * Abs(ds.dsBmih.biHeight));
    return TRUE;
}

/// <summary>
/// 保存DDB为32bppDIB。
/// 此函数总保存为自上而下DIB
/// </summary>
/// <param name="rb">追加结果</param>
/// <param name="hDdb">DDB句柄，必须保证此位图未被选入任何DC</param>
/// <param name="hDC">DC，未传入则使用屏幕DC</param>
/// <param name="bUnpremul">是否反预乘</param>
/// <returns>成功返回TRUE，失败返回FALSE，已设置LastError</returns>
inline BOOL GdiSaveDdbAs32bppDib(
    CByteBuffer& rb,
    HBITMAP hDdb,
    HDC hDC = nullptr,
    BOOL bUnpremul = FALSE) noexcept
{
    BITMAP bm;
    if (!GetObjectW(hDdb, sizeof(bm), &bm))
        return FALSE;

    auto p = Priv::GdiSaveDibHeader(
        rb, bm.bmWidth * 4,
        bm.bmWidth, -bm.bmHeight, 32);

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = bm.bmWidth;
    bi.bmiHeader.biHeight = -bm.bmHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    BOOL bDeleteDC{};
    if (!hDC)
    {
        hDC = GetDC(nullptr);
        bDeleteDC = TRUE;
    }
    const auto bRet = GetDIBits(hDC, hDdb, 0,
        bm.bmHeight, p, &bi, DIB_RGB_COLORS);
    if (bDeleteDC)
        ReleaseDC(nullptr, hDC);
    if (bRet && bUnpremul)
    {
        const auto pEnd = p + bm.bmWidth * bm.bmHeight * 4;
        for (; p < pEnd; p += 4)
        {
            const auto A = p[3];
            if (A)
            {
                p[0] = BYTE(p[0] * 255 / A);
                p[1] = BYTE(p[1] * 255 / A);
                p[2] = BYTE(p[2] * 255 / A);
            }
        }
    }
    return bRet;
}

inline HBITMAP GdiCreate32bppDibSection(
    int cx, int cy,
    _Outptr_opt_ void** pBits = nullptr) noexcept
{
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = (LONG)cx;
    bmi.bmiHeader.biHeight = -(LONG)cy;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    void* Dummy{};
    return CreateDIBSection(nullptr, &bmi,
        DIB_RGB_COLORS, pBits ? pBits : &Dummy, nullptr, 0);
}
ECK_NAMESPACE_END