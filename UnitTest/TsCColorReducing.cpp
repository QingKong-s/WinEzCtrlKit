#include "pch.h"
#include "../eck/CColorReducing.h"
#include "../eck/ComPtr.h"

#pragma comment(lib, "ntdll.lib")

using eck::ComPtr;

TS_NS_BEGIN
TEST_CLASS(TsCColorReducing)
{
public:
    TEST_METHOD(Ts)
    {
        return;
        HRESULT hr;
        (void)CoInitialize(nullptr);
        ComPtr<IWICImagingFactory> pFactory;
        pFactory.CreateInstance(CLSID_WICImagingFactory);

        ComPtr<IWICBitmapDecoder> pDecoder;
        hr = pFactory->CreateDecoderFromFilename(
            LR"(E:\Desktop\Temp\1.jpg)",
            nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnLoad, &pDecoder);
        if (FAILED(hr))
            return;
        ComPtr<IWICBitmapFrameDecode> pFrame;
        pDecoder->GetFrame(0, &pFrame);
        UINT w, h;
        pFrame->GetSize(&w, &h);

        ComPtr<IWICFormatConverter> pCvtTo32;
        pFactory->CreateFormatConverter(&pCvtTo32);
        pCvtTo32->Initialize(
            pFrame.Get(),
            GUID_WICPixelFormat32bppRGB,
            WICBitmapDitherTypeNone,
            nullptr, 0.0, WICBitmapPaletteTypeCustom);

        std::vector<ARGB> vPixels(w * h);
        pCvtTo32->CopyPixels(nullptr, w * 4, w * h * 4, (BYTE*)vPixels.data());

        eck::CColorReducing Cr{ 256 };
        for (const auto cr : vPixels)
            Cr.AddColor(GetRValue(cr), GetGValue(cr), GetBValue(cr));

        std::vector<ARGB> vPal(256);
        Cr.GetPalette((UINT*)vPal.data(), 256);
        for (auto& cr : vPal)
            cr |= 0xFF00'0000;

        ComPtr<IWICPalette> pPalette;
        pFactory->CreatePalette(&pPalette);
        pPalette->InitializeCustom((WICColor*)vPal.data(), 256);
        //pPalette->InitializeFromBitmap(pFrame.Get(), 256, FALSE);


        ComPtr<IWICBitmapEncoder> pEncoder;
        hr = pFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);
        if (FAILED(hr))
            return;

        ComPtr<IWICStream> pStream;
        pFactory->CreateStream(&pStream);
        hr = pStream->InitializeFromFilename(
            LR"(E:\Desktop\TsCColorReducing.png)", GENERIC_WRITE);

        hr = pEncoder->Initialize(pStream.Get(), WICBitmapEncoderNoCache);
        ComPtr<IWICBitmapFrameEncode> pFrameEncode;
        hr = pEncoder->CreateNewFrame(&pFrameEncode, nullptr);

        hr = pFrameEncode->Initialize(nullptr);
        hr = pFrameEncode->SetSize(w, h);
        auto Fmt = GUID_WICPixelFormat8bppIndexed;
        hr = pFrameEncode->SetPixelFormat(&Fmt);
        hr = pFrameEncode->SetPalette(pPalette.Get());

        ComPtr<IWICFormatConverter> pCvtTo8;
        hr = pFactory->CreateFormatConverter(&pCvtTo8);
        hr = pCvtTo8->Initialize(
            pFrame.Get(),
            GUID_WICPixelFormat8bppIndexed,
            WICBitmapDitherTypeNone,
            pPalette.Get(), 0.0, WICBitmapPaletteTypeCustom);

        hr = pFrameEncode->WriteSource(pCvtTo8.Get(), nullptr);
        hr = pFrameEncode->Commit();
        hr = pEncoder->Commit();
    }
};
TS_NS_END