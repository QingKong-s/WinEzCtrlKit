#pragma once
#include "ComPtr.h"
#include "CRefBin.h"
#include "Utility.h"

ECK_NAMESPACE_BEGIN
struct SNP_OUTPUT
{
    UINT idxOutput;
    ComPtr<IDXGIOutput1> pOutput;
    RCWH rc;
};

struct SNP_ADAPTER
{
    UINT idxAdapter;
    ComPtr<IDXGIAdapter> pAdapter;
    std::vector<SNP_OUTPUT> vOutput;
};

struct SNP_CURSOR
{
    CRefBin rbCursor;
    DXGI_OUTDUPL_POINTER_SHAPE_INFO ShapeInfo;
    DXGI_OUTDUPL_POINTER_POSITION Position;
};

using FSnapshotPreFetch = HRESULT(*)(size_t cOutput, const RCWH& rcBound);
using FSnapshotFetch = HRESULT(*)(size_t cOutput, const RCWH& rcBound, const SNP_OUTPUT& Output,
    const D3D11_MAPPED_SUBRESOURCE& MappedRes, const SNP_CURSOR& Cursor);

/// <summary>
/// 快照
/// </summary>
/// <typeparam name="F1"></typeparam>
/// <typeparam name="F2"></typeparam>
/// <param name="fnPreFetch">捕获前预处理回调</param>
/// <param name="fnFetch">获取纹理数据回调</param>
/// <param name="rc">矩形，为空则捕获所有输出</param>
/// <param name="bCursor">是否捕获鼠标指针</param>
/// <param name="msTimeout">帧超时</param>
/// <returns>HRESULT</returns>
template<class F1, class F2>
inline HRESULT IntSnapshot(F1 fnPreFetch, F2 fnFetch,
    const RCWH& rc, BOOL bCursor, UINT msTimeout = 500) noexcept
{
    HRESULT hr;

    std::vector<SNP_ADAPTER> vAdapter{};
    UINT idxAdapter = 0u;
    UINT idxOutput = 0u;

    IDXGIAdapter* pAdapter;
    IDXGIOutput* pOutput;

    RCWH rcBound{};
    size_t cOutput{};// Optimize for single output

    const BOOL bEmpty = IsRectEmpty(rc);
    while (SUCCEEDED(g_pDxgiFactory->EnumAdapters(idxAdapter, &pAdapter)))
    {
        auto& e = vAdapter.emplace_back(idxAdapter, pAdapter);
        while (SUCCEEDED(pAdapter->EnumOutputs(idxOutput, &pOutput)))
        {
            DXGI_OUTPUT_DESC Desc;
            if (SUCCEEDED(pOutput->GetDesc(&Desc)))
            {
                if (bEmpty)
                {
                    ++cOutput;
                    UnionRect(rcBound, rcBound, ToRCWH(Desc.DesktopCoordinates));
                    IDXGIOutput1* pOutput1;
                    pOutput->QueryInterface(&pOutput1);
                    e.vOutput.emplace_back(idxOutput, pOutput1, ToRCWH(Desc.DesktopCoordinates));
                }
                else if (RCWH rcTemp; IntersectRect(rcTemp, rc, ToRCWH(Desc.DesktopCoordinates)))
                {
                    ++cOutput;
                    UnionRect(rcBound, rcBound, rcTemp);
                    IDXGIOutput1* pOutput1;
                    pOutput->QueryInterface(&pOutput1);
                    e.vOutput.emplace_back(idxOutput, pOutput1, rcTemp);
                }
            }
            pOutput->Release();
            ++idxOutput;
        }
        if (e.vOutput.empty())
            vAdapter.pop_back();
        ++idxAdapter;
    }
    if (vAdapter.empty())
        return E_FAIL;

    if (FAILED(hr = fnPreFetch(cOutput, rcBound)))
        return hr;

    for (const auto& e : vAdapter)
    {
        for (const auto& f : e.vOutput)
        {
            ComPtr<ID3D11Device> pDevice;
            ComPtr<ID3D11DeviceContext> pDC;

            hr = D3D11CreateDevice(e.pAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN,
                nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifdef _DEBUG
                | D3D11_CREATE_DEVICE_DEBUG
#endif // _DEBUG
                , nullptr, 0,
                D3D11_SDK_VERSION, &pDevice, nullptr, &pDC);
            if (FAILED(hr))
                return hr;

            ComPtr<IDXGIOutputDuplication> pDup;
            if (FAILED(hr = f.pOutput->DuplicateOutput(pDevice.Get(), &pDup)))
                return hr;

            ComPtr<IDXGIResource> pResource;
            DXGI_OUTDUPL_FRAME_INFO FrameInfo;
            pDup->ReleaseFrame();
            SNP_CURSOR Cursor{};
            EckCounterNV(100)
            {
                if (FAILED(hr = pDup->AcquireNextFrame(msTimeout, &FrameInfo, &pResource)))
                    return hr;
                if (bCursor && FrameInfo.PointerShapeBufferSize
                    && FrameInfo.PointerPosition.Visible)
                {
                    Cursor.rbCursor.ReSize(FrameInfo.PointerShapeBufferSize);
                    UINT Dummy;
                    pDup->GetFramePointerShape(FrameInfo.PointerShapeBufferSize,
                        Cursor.rbCursor.Data(), &Dummy, &Cursor.ShapeInfo);
                    Cursor.Position = FrameInfo.PointerPosition;
                }
                if (!FrameInfo.TotalMetadataBufferSize)
                {
                    pResource->Release();
                    pDup->ReleaseFrame();
                }
                else
                    goto ResourceOk;
            }
            return ERROR_TIMEOUT;
        ResourceOk:
            ComPtr<ID3D11Texture2D> pTexture;
            pResource.As(pTexture);

            D3D11_TEXTURE2D_DESC Desc;
            pTexture->GetDesc(&Desc);
            Desc.BindFlags = 0;
            Desc.MiscFlags = 0;
            Desc.Usage = D3D11_USAGE_STAGING;// 回读
            Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            ComPtr<ID3D11Texture2D> pTex;
            if (FAILED(hr = pDevice->CreateTexture2D(&Desc, nullptr, &pTex)))
                return hr;
            pDC->CopyResource(pTex.Get(), pTexture.Get());
            pDC->Flush();

            D3D11_MAPPED_SUBRESOURCE MappedRes;
            if (FAILED(hr = pDC->Map(pTex.Get(), 0, D3D11_MAP_READ, 0, &MappedRes)))
                return hr;

            hr = fnFetch(cOutput, rcBound, f, MappedRes, Cursor);

            pDC->Unmap(pTex.Get(), 0);
            pDup->ReleaseFrame();
        }
    }

    return hr;
}

/// <summary>
/// 快照
/// </summary>
/// <param name="pBmp">返回WIC位图</param>
/// <param name="rc">矩形，为空则捕获所有输出</param>
/// <param name="bCursor">是否捕获鼠标指针</param>
/// <param name="msTimeout">帧超时</param>
/// <returns>HRESULT</returns>
inline HRESULT Snapshot(IWICBitmap*& pBmp, const RCWH& rc,
    BOOL bCursor = FALSE, UINT msTimeout = 500) noexcept
{
    pBmp = nullptr;
    ComPtr<IWICBitmap> pBitmap;
    ComPtr<IWICBitmapLock> pLock;
    BYTE* pBits{};
    UINT cbStride{};

    const auto hr = IntSnapshot([&](size_t cOutput, const RCWH& rcBound) -> HRESULT
        {
            if (cOutput != 1 || bCursor)
            {
                auto hr = g_pWicFactory->CreateBitmap(rcBound.cx, rcBound.cy,
                    GUID_WICPixelFormat32bppBGRA, WICBitmapCacheOnDemand, &pBitmap);
                if (FAILED(hr))
                    return hr;
                if (FAILED(hr = pBitmap->Lock(nullptr, WICBitmapLockWrite, &pLock)))
                    return hr;
                // TODO: 处理多线程套间
                pLock->GetDataPointer(&cbStride, &pBits);
                pLock->GetStride(&cbStride);
            }
            return S_OK;
        },
        [&](size_t cOutput, const RCWH& rcBound, const SNP_OUTPUT& Output,
            const D3D11_MAPPED_SUBRESOURCE& MappedRes, const SNP_CURSOR& Cursor) -> HRESULT
        {
            if (cOutput != 1 || bCursor)
            {
                RCWH rcDst = Output.rc;
                OffsetRect(rcDst, -rcBound.x, -rcBound.y);

                BYTE* pDst = pBits + rcDst.y * cbStride + rcDst.x * 4;
                for (int y = Output.rc.y; y < Output.rc.y + Output.rc.cy; ++y)
                {
                    memcpy(pDst,
                        (BYTE*)MappedRes.pData + Output.rc.x * 4 + y * MappedRes.RowPitch,
                        Output.rc.cx * 4);
                    pDst = pDst + cbStride;
                }

                if (!Cursor.rbCursor.IsEmpty())
                {
                    POINT ptCursor{ Cursor.Position.Position };
                    ptCursor.x += Output.rc.x;
                    ptCursor.y += Output.rc.y;
                    ptCursor.x -= rcBound.x;
                    ptCursor.y -= rcBound.y;
#pragma warning(suppress: 26813)// 使用位与检查标志
                    const int cyReal = ((Cursor.ShapeInfo.Type == DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME) ?
                        Cursor.ShapeInfo.Height / 2 : Cursor.ShapeInfo.Height);
                    const int yStart = (ptCursor.y < 0 ? -ptCursor.y : 0);
                    const int yEnd = (ptCursor.y + cyReal > rcBound.cy ?
                        rcBound.cy - ptCursor.y : cyReal);
                    const int xStart = (ptCursor.x < 0 ? -ptCursor.x : 0);
                    const int xEnd = (ptCursor.x + (int)Cursor.ShapeInfo.Width > rcBound.cx ?
                        rcBound.cx - ptCursor.x : Cursor.ShapeInfo.Width);
                    switch (Cursor.ShapeInfo.Type)
                    {
                    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_COLOR:
                    {
                        for (int i = yStart; i < yEnd; ++i)
                        {
                            for (int j = xStart; j < xEnd; ++j)
                            {
                                const auto uSrc = *(UINT*)(Cursor.rbCursor.Data() +
                                    i * Cursor.ShapeInfo.Pitch + j * 4);
                                const auto puDst = (UINT*)(pBits +
                                    (ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);
                                *puDst = ArgbAlphaBlend(uSrc, *puDst);
                            }
                        }
                    }
                    break;
                    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MONOCHROME:
                    {
                        BYTE byMask = 0b1000'0000;
                        for (int i = yStart; i < yEnd; ++i)
                        {
                            for (int j = xStart; j < xEnd; ++j)
                            {
                                const auto bySrcAnd = (*(Cursor.rbCursor.Data() +
                                    i * Cursor.ShapeInfo.Pitch + j / 8)) & byMask;
                                const auto bySrcXor = (*(Cursor.rbCursor.Data() +
                                    (i + Cursor.ShapeInfo.Height / 2) * Cursor.ShapeInfo.Pitch +
                                    j / 8)) & byMask;
                                const auto puDst = (UINT*)(pBits +
                                    (ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);

                                *puDst = (*puDst & (bySrcAnd ? 0xFFFFFFFF : 0xFF000000)) ^
                                    (bySrcXor ? 0x00FFFFFF : 0x00000000);

                                if (byMask == 1)
                                    byMask = 0b1000'0000;
                                else
                                    byMask >>= 1;
                            }
                        }
                    }
                    break;
                    case DXGI_OUTDUPL_POINTER_SHAPE_TYPE_MASKED_COLOR:
                    {
                        for (int i = yStart; i < yEnd; ++i)
                        {
                            for (int j = xStart; j < xEnd; ++j)
                            {
                                const auto uSrc = *(UINT*)(Cursor.rbCursor.Data() +
                                    i * Cursor.ShapeInfo.Pitch + j * 4);
                                const auto puDst = (UINT*)(pBits +
                                    (ptCursor.y + i) * cbStride + (ptCursor.x + j) * 4);
                                if (uSrc & 0xFF000000)
                                    *puDst = (*puDst ^ uSrc) | 0xFF000000;
                                else
                                    *puDst = uSrc | 0xFF000000;
                            }
                        }
                    }
                    break;
                    }
                }
            }
            else
                return g_pWicFactory->CreateBitmapFromMemory(
                    Output.rc.cx, Output.rc.cy,
                    GUID_WICPixelFormat32bppBGRA,
                    MappedRes.RowPitch,
                    MappedRes.RowPitch * Output.rc.cy,
                    (BYTE*)MappedRes.pData + Output.rc.x * 4 + Output.rc.y * MappedRes.RowPitch,
                    &pBmp);
            return S_OK;
        }, rc, bCursor, msTimeout);

    if (FAILED(hr))
        return hr;
    if (pBitmap.Get())
        pBmp = pBitmap.Detach();
    return S_OK;
}
ECK_NAMESPACE_END