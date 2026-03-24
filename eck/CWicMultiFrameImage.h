#pragma once
#include "CStreamWalker.h"
#include "MemWalker.h"
#include "Crc.h"
#include "CByteBufferStream.h"

ECK_NAMESPACE_BEGIN
enum class PngColorType : BYTE
{
    Grayscale = 0,
    Truecolor = 2,
    Indexed = 3,
    GrayscaleAlpha = 4,
    TruecolorAlpha = 6
};

struct PngIHDR
{
    BYTE Width[4];		// 宽度
    BYTE Height[4];		// 高度
    BYTE BitDepth;		// 位深度
    PngColorType ColorType;	// 颜色类型
    BYTE CompressionMethod;	// 压缩方法，仅定义0
    BYTE FilterMethod;		// 过滤方法，仅定义0
    BYTE InterlaceMethod;	// 隔行扫描，0 = 不隔行扫描，1 = Adm7隔行扫描
};

enum class PngBlendOp : BYTE
{
    Source,
    Over
};

static_assert(sizeof(PngIHDR) == 13);

class CWicMultiFrameImage
{
public:
    enum class Disposal : BYTE
    {
        Undefined,  // 未指定
        None,       // 直接叠加
        Background, // 清除为背景颜色
        Previous    // 还原为上一帧绘制前状态
    };
private:
    struct ITEM
    {
        ComPtr<IWICBitmapSource> pSource;
        int msDelay;
        int ox;
        int oy;
        int cx;
        int cy;
        Disposal eDisposal;
    };

    std::vector<ITEM> m_vFrame{};
    int m_cRepeat{};
    int m_cx{}, m_cy{};
    WICColor m_crBkg{};
    WICColor m_crTransparent{};
    BOOLEAN m_bTransparent{};
    Disposal m_eDisposal{};
    BYTE m_byAlphaThreshold{ 255 };

    BYTE PngGetDisposal(const ITEM& e) const noexcept
    {
        if (e.eDisposal == Disposal::Undefined)
            if (m_eDisposal == Disposal::Undefined)
                return 0;// None
            else
                return (BYTE)m_eDisposal - 1;
        else
            return (BYTE)e.eDisposal - 1;
    }

    BYTE GifGetDisposal(const ITEM& e) const noexcept
    {
        if (e.eDisposal == Disposal::Undefined)
            return (BYTE)m_eDisposal;
        else
            return (BYTE)e.eDisposal;
    }
public:
    ECK_DISABLE_COPY_MOVE_DEF_CONS(CWicMultiFrameImage);

    /// <summary>
    /// 添加帧
    /// </summary>
    /// <param name="pBitmap">WIC位图源</param>
    /// <param name="msDelay">以毫秒计的显示延迟</param>
    /// <param name="ox, oy">偏移</param>
    /// <param name="cx, cy">新尺寸，0表示不变</param>
    /// <param name="eDisposal">处理方式</param>
    /// <param name="eInter">缩放时使用的插值方式</param>
    /// <returns>HRESULT</returns>
    HRESULT AddFrame(
        IWICBitmapSource* pSource_, int msDelay,
        int ox = 0, int oy = 0, int cx = 0, int cy = 0,
        Disposal eDisposal = Disposal::Undefined,
        WICBitmapInterpolationMode eInter = WICBitmapInterpolationModeLinear) noexcept
    {
        HRESULT hr;
        UINT cxOrg, cyOrg;
        hr = pSource_->GetSize(&cxOrg, &cyOrg);
        if (FAILED(hr))
            return hr;
        EckAssert(cx >= 0 && cy >= 0);

        BOOL bScale{};
        if (cx && cx != (int)cxOrg)
            bScale = TRUE;
        else
            cx = (int)cxOrg;

        if (cy && cy != (int)cyOrg)
            bScale = TRUE;
        else
            cy = (int)cyOrg;

        ComPtr<IWICBitmapSource> pSource;
        if (bScale)
        {
            ComPtr<IWICBitmapScaler> pScaler;
            hr = g_pWicFactory->CreateBitmapScaler(&pScaler);
            if (FAILED(hr))
                return hr;
            hr = pScaler->Initialize(pSource_, cx, cy, eInter);
            if (FAILED(hr))
                return hr;
            pSource.Attach(pScaler.Detach());
        }
        else
            pSource = pSource_;

        m_vFrame.emplace_back(pSource.Get(), msDelay,
            ox, oy, cx, cy, eDisposal);
        m_cx = std::max(m_cx, cx);
        m_cy = std::max(m_cy, cy);
        return S_OK;
    }

    /// <summary>
    /// 添加帧。
    /// 添加WIC位图解码器中的所有帧
    /// </summary>
    /// <param name="pDecoder">WIC位图解码器</param>
    /// <param name="msDelay">以毫秒计的显示延迟</param>
    /// <param name="ox, oy">偏移</param>
    /// <param name="cx, cy">新尺寸，0表示不变</param>
    /// <param name="eDisposal">处理方式</param>
    /// <param name="eInter">缩放时使用的插值方式</param>
    /// <returns>HRESULT</returns>
    HRESULT AddFrame(
        IWICBitmapDecoder* pDecoder, int msDelay,
        int ox = 0, int oy = 0, int cx = 0, int cy = 0,
        Disposal eDisposal = Disposal::Undefined,
        WICBitmapInterpolationMode eInter = WICBitmapInterpolationModeLinear) noexcept
    {
        HRESULT hr;
        UINT cFrame;
        pDecoder->GetFrameCount(&cFrame);
        EckCounter(cFrame, i)
        {
            ComPtr<IWICBitmapFrameDecode> pFrame;
            if (FAILED(hr = pDecoder->GetFrame(i, &pFrame)))
                return hr;
            if (FAILED(hr = AddFrame(pFrame.Get(), msDelay,
                ox, oy, cx, cy, eDisposal, eInter)))
                return hr;
        }
        return S_OK;
    }

    void Clear() noexcept
    {
        m_vFrame.clear();
        m_cx = m_cy = 0;
    }

    HRESULT SaveAsGif(IStream* pStream) noexcept
    {
        HRESULT hr;
        PROPVARIANT Var{};
        ComPtr<IWICBitmapEncoder> pEncoder;
        g_pWicFactory->CreateEncoder(GUID_ContainerFormatGif, nullptr, &pEncoder);
        if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
            return hr;
        // 写循环次数
        ComPtr<IWICMetadataQueryWriter> pMdWriter;
        if (FAILED(hr = pEncoder->GetMetadataQueryWriter(&pMdWriter)))
            return hr;
        BYTE byApp[]{ 'N','E','T','S','C','A','P','E','2','.','0' };// 或ANIMEXTS1.0
        Var.vt = VT_UI1 | VT_VECTOR;
        Var.caub.cElems = sizeof(byApp);
        Var.caub.pElems = byApp;
        if (FAILED(hr = pMdWriter->SetMetadataByName(L"/appext/Application", &Var)))
            return hr;

        BYTE byData[]
        {
            0x03, 0x01,
            (BYTE)m_cRepeat, BYTE(m_cRepeat >> 8),
            0x00
        };
        Var.caub.cElems = sizeof(byData);
        Var.caub.pElems = byData;
        if (FAILED(hr = pMdWriter->SetMetadataByName(L"/appext/Data", &Var)))
            return hr;
        // 写宽高
        Var.vt = VT_UI2;
        Var.uiVal = m_cx;
        if (FAILED(hr = pMdWriter->SetMetadataByName(L"/logscrdesc/Width", &Var)))
            return hr;
        Var.uiVal = m_cy;
        if (FAILED(hr = pMdWriter->SetMetadataByName(L"/logscrdesc/Height", &Var)))
            return hr;
        // 写背景色
        if (m_crBkg)
        {
            const WICColor crPal[2]{ m_crBkg, m_crBkg };
            ComPtr<IWICPalette> pPalette;
            g_pWicFactory->CreatePalette(&pPalette);
            pPalette->InitializeCustom((WICColor*)&crPal, 2);
            if (FAILED(hr = pEncoder->SetPalette(pPalette.Get())))
                return hr;
            Var.vt = VT_BOOL;
            Var.boolVal = VARIANT_TRUE;
            hr = pMdWriter->SetMetadataByName(L"/logscrdesc/GlobalColorTableFlag", &Var);
            Var.vt = VT_UI1;
            Var.bVal = 0; // 2 ^ (0 + 1) = 2 colors
            hr = pMdWriter->SetMetadataByName(L"/logscrdesc/GlobalColorTableSize", &Var);
            Var.vt = VT_UI1;
            Var.bVal = 0;
            hr = pMdWriter->SetMetadataByName(L"/logscrdesc/BackgroundColorIndex", &Var);
        }
        // 写所有帧
        for (const auto& e : m_vFrame)
        {
            ComPtr<IWICBitmapFrameEncode> pFrameEncode;
            ComPtr<IWICMetadataQueryWriter> pMdWriter;

            if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr)))
                return hr;
            if (FAILED(hr = pFrameEncode->Initialize(nullptr)))
                return hr;
            if (FAILED(hr = pFrameEncode->GetMetadataQueryWriter(&pMdWriter)))
                return hr;
            // 写帧延迟
            Var.vt = VT_UI2;
            Var.uiVal = e.msDelay / 10;
            pMdWriter->SetMetadataByName(L"/grctlext/Delay", &Var);
            // 写偏移
            if (e.ox)
            {
                Var.uiVal = e.ox;
                pMdWriter->SetMetadataByName(L"/imgdesc/Left", &Var);
            }
            if (e.oy)
            {
                Var.uiVal = e.oy;
                pMdWriter->SetMetadataByName(L"/imgdesc/Top", &Var);
            }
            // 写擦除标志
            Var.vt = VT_UI1;
            Var.bVal = GifGetDisposal(e);
            pMdWriter->SetMetadataByName(L"/grctlext/Disposal", &Var);
            // 写入帧
            if (m_bTransparent)
            {
                ComPtr<IWICPalette> pPalette;
                g_pWicFactory->CreatePalette(&pPalette);
                // 计算调色板时必须指定是否增加颜色值为0的条目
                hr = pPalette->InitializeFromBitmap(
                    e.pSource.Get(), 256, !m_crTransparent);
                if (FAILED(hr))
                    return hr;
                UINT cColor;
                pPalette->GetColorCount(&cColor);
                if (!cColor)
                    return E_FAIL;
                BYTE idxTrans;
                BOOLEAN bHasTrans;
                if (m_crTransparent)
                {
                    UINT cActualColor;
                    const auto pcr{ std::make_unique<WICColor[]>(cColor) };
                    pPalette->GetColors(cColor, pcr.get(), &cActualColor);

                    const auto itEnd = pcr.get() + std::min(cColor, cActualColor);
                    const auto it = std::find(pcr.get(), itEnd, m_crTransparent);
                    bHasTrans = (it != itEnd);
                    idxTrans = BYTE(it - pcr.get());
                }
                else
                {
                    bHasTrans = TRUE;
                    idxTrans = BYTE(cColor - 1);
                }

                if (bHasTrans)
                {
                    Var.vt = VT_UI1;
                    Var.bVal = idxTrans;
                    pMdWriter->SetMetadataByName(L"/grctlext/TransparentColorIndex", &Var);
                    Var.vt = VT_BOOL;
                    Var.boolVal = VARIANT_TRUE;
                    pMdWriter->SetMetadataByName(L"/grctlext/TransparencyFlag", &Var);
                }

                ComPtr<IWICFormatConverter> pConverter;
                g_pWicFactory->CreateFormatConverter(&pConverter);
                hr = pConverter->Initialize(
                    e.pSource.Get(),
                    GUID_WICPixelFormat8bppIndexed,
                    WICBitmapDitherTypeNone,
                    pPalette.Get(),
                    m_crTransparent ? 0. : ((double)m_byAlphaThreshold / 255.),
                    WICBitmapPaletteTypeCustom);
                if (FAILED(hr))
                    return hr;
                pFrameEncode->SetPalette(pPalette.Get());
                if (FAILED(hr = pFrameEncode->WriteSource(pConverter.Get(), nullptr)))
                    return hr;
            }
            else
                if (FAILED(hr = pFrameEncode->WriteSource(e.pSource.Get(), nullptr)))
                    return hr;
            if (FAILED(hr = pFrameEncode->Commit()))
                return hr;
        }
        return pEncoder->Commit();
    }

    // WARNING 第一帧偏移必须为0，后续帧矩形必须完全包含在第一帧内
    HRESULT SaveAsApng(
        IStream* pStream,
        PngBlendOp eBlendOp = PngBlendOp::Source,
        BOOL bInterlace = FALSE,
        WICPngFilterOption eFilter = WICPngFilterUnspecified) noexcept
    {
        constexpr BYTE PngSignature[]{ 0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A };
        constexpr BYTE IdIHDR[]{ 0x49,0x48,0x44,0x52 };
        constexpr BYTE IdacTL[]{ 0x61,0x63,0x54,0x4C };
        constexpr BYTE IdfcTL[]{ 0x66,0x63,0x54,0x4C };
        constexpr BYTE IdfdAT[]{ 0x66,0x64,0x41,0x54 };
        constexpr BYTE IdIDAT[]{ 0x49,0x44,0x41,0x54 };
        constexpr BYTE IdIEND[]{ 0x49,0x45,0x4E,0x44 };
        CStreamWalker w{ pStream };
        w << PngSignature;
        // 写IHDR
        BYTE byBuf[36];
        const auto posIhdr = w.GetPosition();
        constexpr size_t CbIHDR = 13 + 4 + 4 + 4;
        w += CbIHDR;// 悬而未决，待后续写入
        // 写acTL
        CMemoryWalker wkChunk{ byBuf, sizeof(byBuf) };
        wkChunk << ReverseInteger(8u) << IdacTL
            << ReverseInteger((UINT)m_vFrame.size())
            << ReverseInteger((UINT)m_cRepeat);
        w.Write(byBuf, wkChunk.GetPosition())
            << ReverseInteger(CalculateCrc32(byBuf + 4, wkChunk.GetPosition() - 4));
        wkChunk.SeekToBegin();
        // 写入帧数据
        UINT uSerialNum{};
        CByteBuffer rbPng{};
        CByteBufferStream PngStream{ rbPng };
        CHAR chChunkId[4];
        UINT cbChunkData;
        BOOL bFirstFrame{ TRUE };
        BYTE* pChunkIdBegin;
        for (const auto& e : m_vFrame)
        {
            // 写fcTL
            wkChunk
                << ReverseInteger(26u) << IdfcTL
                << ReverseInteger(uSerialNum++)
                << ReverseInteger(e.cx)
                << ReverseInteger(e.cy)
                << ReverseInteger(e.ox)
                << ReverseInteger(e.oy)
                << ReverseInteger(USHORT(e.msDelay / 10))
                << ReverseInteger(0_us)// 1/100秒
                << PngGetDisposal(e)
                << eBlendOp
                ;
            w.Write(byBuf, wkChunk.GetPosition())
                << ReverseInteger(CalculateCrc32(byBuf + 4, wkChunk.GetPosition() - 4));
            wkChunk.SeekToBegin();
            // 写IDAT或fdAT
            PngStream.Seek(ToLargeInt(0), STREAM_SEEK_SET, nullptr);
            // WIC编码一帧
            HRESULT hr;
            ComPtr<IWICBitmapEncoder> pEncoder;
            ComPtr<IWICBitmapFrameEncode> pFrame;
            if (FAILED(hr = g_pWicFactory->CreateEncoder(
                GUID_ContainerFormatPng, nullptr, &pEncoder)))
                return hr;
            if (FAILED(hr = pEncoder->Initialize(
                &PngStream, WICBitmapEncoderNoCache)))
                return hr;
            if (bInterlace || (eFilter != WICPngFilterUnspecified))
            {
                ComPtr<IPropertyBag2> pPropBag;
                if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, &pPropBag)))
                    return hr;
                PROPBAG2 Prop{ .pstrName = (PWSTR)L"InterlaceOption" };
                VARIANT Var{};
                Var.vt = VT_BOOL;
                Var.bVal = (bInterlace ? VARIANT_TRUE : VARIANT_FALSE);
                pPropBag->Write(1, &Prop, &Var);
                Prop.pstrName = (PWSTR)L"FilterOption";
                Var.vt = VT_UI1;
                Var.bVal = (BYTE)eFilter;
                pPropBag->Write(1, &Prop, &Var);
                if (FAILED(hr = pFrame->Initialize(pPropBag.Get())))
                    return hr;
            }
            else
            {
                if (FAILED(hr = pEncoder->CreateNewFrame(&pFrame, nullptr)))
                    return hr;
                if (FAILED(hr = pFrame->Initialize(nullptr)))
                    return hr;
            }
            GUID guidFmt{ GUID_WICPixelFormat32bppBGRA };
            pFrame->SetPixelFormat(&guidFmt);
            if (FAILED(hr = pFrame->WriteSource(e.pSource.Get(), nullptr)))
                return hr;
            if (FAILED(hr = pFrame->Commit()))
                return hr;
            if (FAILED(hr = pEncoder->Commit()))
                return hr;
            pEncoder.Clear();
            pFrame.Clear();
            PngStream.AssertReference(1);

            CMemoryWalker wt{ rbPng.Data(), rbPng.Size() };
            wt += 8;// 跳过PNG签名
            EckLoop()
            {
                wt.ReadReversed(cbChunkData) >> chChunkId;
                const auto cbOriginalData = cbChunkData;
                if (memcmp(chChunkId, IdIHDR, 4) == 0)
                {
                    if (bFirstFrame)// 复制第一帧IHDR到流首部
                    {
                        const auto pos = w.GetPosition();
                        w.Seek(posIhdr, STREAM_SEEK_SET);
                        w.Write(wt.Data() - 8, CbIHDR);
                        w.Seek(pos, STREAM_SEEK_SET);
                    }
                    wt += (cbOriginalData + 4);// 跳过数据和CRC
                }
                else if (memcmp(chChunkId, IdIDAT, 4) == 0)
                {
                    if (bFirstFrame)
                    {
                        pChunkIdBegin = wt.Data() - 4;
                        w << ReverseInteger(cbOriginalData);
                        w.Write(pChunkIdBegin, cbOriginalData + 4);
                        w.Write(pChunkIdBegin + cbOriginalData + 4, 4);
                    }
                    else
                    {
                        pChunkIdBegin = wt.Data() - 8;
                        memcpy(pChunkIdBegin, IdfdAT, 4);
                        const auto u = ReverseInteger(uSerialNum);
                        memcpy(pChunkIdBegin + 4, &u, 4);
                        ++uSerialNum;

                        cbChunkData += 4;
                        w << ReverseInteger(cbChunkData);
                        w.Write(pChunkIdBegin, cbChunkData + 4);
                        w << ReverseInteger(CalculateCrc32(pChunkIdBegin, cbChunkData + 4));
                    }
                    wt += (cbOriginalData + 4);// 跳过当前的IDAT数据及CRC
                }
                else if (memcmp(chChunkId, IdIEND, 4) == 0)
                    break;
                else
                    wt += (cbOriginalData + 4);// 跳过数据和CRC
            }
            bFirstFrame = FALSE;
        }
        // 写IEND
        w << 0u << IdIEND << ReverseInteger(CalculateCrc32(IdIEND, 4));
        return S_OK;
    }

    // 将忽略下列属性：重复次数、是否透明、背景颜色、处理方式
    HRESULT SaveAsTiff(
        IStream* pStream,
        float fCompressionQuality = 0.f,
        WICTiffCompressionOption eCompression = WICTiffCompressionDontCare) noexcept
    {
        HRESULT hr;
        ComPtr<IWICBitmapEncoder> pEncoder;
        g_pWicFactory->CreateEncoder(GUID_ContainerFormatTiff, nullptr, &pEncoder);
        if (FAILED(hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache)))
            return hr;
        for (const auto& e : m_vFrame)
        {
            ComPtr<IWICBitmapFrameEncode> pFrameEncode;
            if (fCompressionQuality > 0.f ||
                eCompression != WICTiffCompressionDontCare)
            {
                ComPtr<IPropertyBag2> pPropBag;
                if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, &pPropBag)))
                    return hr;
                PROPBAG2 Prop{ .pstrName = (PWSTR)L"TiffCompressionMethod" };
                VARIANT Var{};
                Var.vt = VT_UI1;
                Var.bVal = eCompression;
                pPropBag->Write(1, &Prop, &Var);
                Prop.pstrName = (PWSTR)L"TiffCompressionQuality";
                Var.vt = VT_R4;
                Var.fltVal = fCompressionQuality;
                pPropBag->Write(1, &Prop, &Var);
                if (FAILED(hr = pFrameEncode->Initialize(pPropBag.Get())))
                    return hr;
            }
            else
            {
                if (FAILED(hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr)))
                    return hr;
                if (FAILED(hr = pFrameEncode->Initialize(nullptr)))
                    return hr;
            }
            if (FAILED(hr = pFrameEncode->WriteSource(e.pSource.Get(), nullptr)))
                return hr;
            if (FAILED(hr = pFrameEncode->Commit()))
                return hr;
        }
        return pEncoder->Commit();
    }

    EckInlineNdCe int GetFrameCount() const noexcept { return (int)m_vFrame.size(); }

    // 0表示无限播放
    EckInlineCe void SetRepeatCount(int c) noexcept { m_cRepeat = c; }
    EckInlineNdCe int GetRepeatCount() const noexcept { return m_cRepeat; }

    // 仅影响GIF，TRUE指示应写入透明信息
    EckInlineCe void SetTransparent(BOOL b) noexcept { m_bTransparent = b; }
    EckInlineNdCe BOOL GetTransparent() const noexcept { return m_bTransparent; }

    // 仅影响GIF，将指定的颜色值视为透明
    EckInlineCe void SetTransparentColor(WICColor cr) noexcept { m_crTransparent = cr; }
    EckInlineNdCe WICColor GetTransparentColor() const noexcept { return m_crTransparent; }

    // 仅影响GIF，指定一个阈值，透明度低于该值的颜色将被视为透明（仅当未指定透明颜色时有效）
    EckInlineCe void SetAlphaThreshold(BYTE by) noexcept { m_byAlphaThreshold = by; }
    EckInlineNdCe BYTE GetAlphaThreshold() const noexcept { return m_byAlphaThreshold; }

    // 仅影响APNG，当帧的处理模式设为Undefined时使用此处指定的值，
    // 若此值也为Undefined，则默认使用None
    EckInlineCe void SetDisposalMethod(Disposal e) noexcept { m_eDisposal = e; }
    EckInlineNdCe Disposal GetDisposalMethod() const noexcept { return m_eDisposal; }

    EckInlineCe void SetBackgroundColor(WICColor cr) noexcept { m_crBkg = cr; }
    EckInlineNdCe WICColor GetBackgroundColor() const noexcept { return m_crBkg; }
};
ECK_NAMESPACE_END