#include "ECK.h"
#include "CLabel.h"
#include "CColorPicker.h"
#include "CBk.h"
#include "CDialog.h"
#include "CSplitBar.h"
#include "CDrawPanel.h"
#include "CListBoxNew.h"
#include "CAnimationBox.h"
#include "Utility.h"

#include <Shlwapi.h>
#include <d3d11.h>

ECK_NAMESPACE_BEGIN
CRefStrW g_rsCurrDir{};
ULONG_PTR g_uGpToken = 0u;

HINSTANCE g_hInstance = NULL;
IWICImagingFactory* g_pWicFactory = NULL;
ID2D1Factory1* g_pD2dFactory = NULL;
IDWriteFactory* g_pDwFactory = NULL;
ID2D1Device* g_pD2dDevice = NULL;
IDXGIDevice1* g_pDxgiDevice = NULL;
IDXGIFactory2* g_pDxgiFactory = NULL;


DWORD g_dwTlsSlot = 0;

static BOOL DefMsgFilter(const MSG&)
{
	return FALSE;
}
FMsgFilter g_pfnMsgFilter = DefMsgFilter;

#ifdef _DEBUG
static void CALLBACK GdiplusDebug(GpDebugEventLevel dwLevel, CHAR* pszMsg)
{
	DbgPrint(StrX2W(pszMsg).Data());
	if (dwLevel == DebugEventLevelFatal)
		DebugBreak();
}
#endif

InitStatus Init(HINSTANCE hInstance, const INITPARAM* pInitParam, DWORD* pdwErrCode)
{
	EckAssert(!g_hInstance && !g_dwTlsSlot);
	DWORD dwTemp;
	if (!pdwErrCode)
		pdwErrCode = &dwTemp;
	INITPARAM ip{};
	if (!pInitParam)
		pInitParam = &ip;
	*pdwErrCode = 0;

	g_dwTlsSlot = TlsAlloc();
	g_hInstance = hInstance;

	if (!IsBitSet(pInitParam->uFlags, ECKINIT_NOINITTHREAD))
		ThreadInit();

	GdiplusStartupInput gpsi{};
	gpsi.GdiplusVersion = 1;
#ifdef _DEBUG
	gpsi.DebugEventCallback = GdiplusDebug;
#endif
	GpStatus uGpRet = GdiplusStartup(&g_uGpToken, &gpsi, NULL);
	if (uGpRet != GpStatus::GpOk)
	{
		*pdwErrCode = uGpRet;
		return InitStatus::GdiplusInitError;
	}

	if (!CLabel::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CBk::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CDialog::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CSplitBar::RegisterWndClass(hInstance))
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CDrawPanel::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CDrawPanelD2D::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CListBoxNew::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	if (!CAnimationBox::RegisterWndClass())
	{
		*pdwErrCode = GetLastError();
		EckDbgPrintFormatMessage(*pdwErrCode);
		return InitStatus::RegWndClassError;
	}

	g_rsCurrDir.ReSize(32768);
	GetModuleFileNameW(NULL, g_rsCurrDir.Data(), g_rsCurrDir.Size());
	PathRemoveFileSpecW(g_rsCurrDir.Data());
	g_rsCurrDir.ReCalcLen();

	HRESULT hr;
#ifndef NDEBUG
	D2D1_FACTORY_OPTIONS D2DFactoryOptions;
	D2DFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	hr = D2D1CreateFactory(pInitParam->uD2dFactoryType, __uuidof(ID2D1Factory1), &D2DFactoryOptions, (void**)&g_pD2dFactory);
#else
	hr = D2D1CreateFactory(pInitParam->uD2dFactoryType, IID_PPV_ARGS(&g_pD2dFactory));
#endif // !NDEBUG
	if (FAILED(hr))
	{
		*pdwErrCode = hr;
		EckDbgPrintFormatMessage(hr);
		return InitStatus::D2dFactoryError;
	}
	//////////////创建DWrite工厂
	hr = DWriteCreateFactory(pInitParam->uDWriteFactoryType, __uuidof(IDWriteFactory), (IUnknown**)&g_pDwFactory);
	if (FAILED(hr))
	{
		*pdwErrCode = hr;
		EckDbgPrintFormatMessage(hr);
		return InitStatus::DWriteFactoryError;
	}
	//////////////创建DXGI工厂
	ID3D11Device* pD3DDevice;
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
		| D3D11_CREATE_DEVICE_DEBUG
#endif // !NDEBUG
		, pInitParam->pD3dFeatureLevel, pInitParam->cD3dFeatureLevel,
		D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);
	if (FAILED(hr))
	{
		*pdwErrCode = hr;
		EckDbgPrintFormatMessage(hr);
		return InitStatus::D3dDeviceError;
	}
	pD3DDevice->QueryInterface(IID_PPV_ARGS(&g_pDxgiDevice));
	pD3DDevice->Release();

	IDXGIAdapter* pDXGIAdapter;
	g_pDxgiDevice->GetAdapter(&pDXGIAdapter);
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&g_pDxgiFactory));
	pDXGIAdapter->Release();
	//////////////创建DXGI设备
	hr = g_pD2dFactory->CreateDevice(g_pDxgiDevice, &g_pD2dDevice);
	if (FAILED(hr))
	{
		*pdwErrCode = hr;
		EckDbgPrintFormatMessage(hr);
		return InitStatus::DxgiDeviceError;
	}
	//////////////创建WIC工厂
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWicFactory));
	if (FAILED(hr))
	{
		*pdwErrCode = hr;
		EckDbgPrintFormatMessage(hr);
		return InitStatus::WicFactoryError;
	}

	return InitStatus::Ok;
}

void UnInit()
{
	SafeRelease(g_pWicFactory);
	SafeRelease(g_pD2dFactory);
	SafeRelease(g_pDwFactory);
	SafeRelease(g_pD2dDevice);
	SafeRelease(g_pDxgiDevice);
	SafeRelease(g_pDxgiFactory);
}

DWORD GetThreadCtxTlsSlot()
{
	return g_dwTlsSlot;
}

void ThreadInit()
{
	EckAssert(!TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = new ECKTHREADCTX{};
	TlsSetValue(GetThreadCtxTlsSlot(), p);
}

void ThreadFree()
{
	EckAssert(TlsGetValue(GetThreadCtxTlsSlot()));
	const auto p = (ECKTHREADCTX*)TlsGetValue(GetThreadCtxTlsSlot());
	delete p;
	TlsSetValue(GetThreadCtxTlsSlot(), NULL);
}

HHOOK BeginCbtHook(CWnd* pCurrWnd, FWndCreating pfnCreatingProc)
{
	EckAssert(pCurrWnd);
	const auto pCtx = GetThreadCtx();
	++pCtx->cHookRef;
	pCtx->pCurrWnd = pCurrWnd;
	pCtx->pfnWndCreatingProc = pfnCreatingProc;
	if (!pCtx->hhkTempCBT)
	{
		EckAssert(pCtx->cHookRef == 1);
		HHOOK hHook = SetWindowsHookExW(WH_CBT, [](int iCode, WPARAM wParam, LPARAM lParam)->LRESULT
			{
				const auto pCtx = GetThreadCtx();
				if (iCode == HCBT_CREATEWND)
				{
					pCtx->pCurrWnd->m_pfnRealProc =
						(WNDPROC)SetWindowLongPtrW((HWND)wParam, GWLP_WNDPROC,
							(LONG_PTR)CWnd::WndProcMsgReflection);
					pCtx->WmAdd((HWND)wParam, pCtx->pCurrWnd);
					if (pCtx->pfnWndCreatingProc)
						pCtx->pfnWndCreatingProc((HWND)wParam, (CBT_CREATEWNDW*)lParam, pCtx);
					EndCbtHook();
				}
				return CallNextHookEx(pCtx->hhkTempCBT, iCode, wParam, lParam);
			}, NULL, GetCurrentThreadId());
		pCtx->hhkTempCBT = hHook;
	}
	return pCtx->hhkTempCBT;
}

void EndCbtHook()
{
	const auto pCtx = GetThreadCtx();
	EckAssert(pCtx->cHookRef > 0);
	EckAssert(pCtx->hhkTempCBT);
	--pCtx->cHookRef;
	if (pCtx->cHookRef == 0)
	{
		UnhookWindowsHookEx(pCtx->hhkTempCBT);
		pCtx->hhkTempCBT = NULL;
	}
}

BOOL PreTranslateMessage(const MSG& Msg)
{
	if (g_pfnMsgFilter(Msg))
		return TRUE;
	HWND hWnd = Msg.hwnd;
	CWnd* pWnd;
	const ECKTHREADCTX* const pCtx = GetThreadCtx();
	while (hWnd)
	{
		pWnd = pCtx->WmAt(hWnd);
		if (pWnd && pWnd->PreTranslateMessage(Msg))
			return TRUE;
		hWnd = GetParent(hWnd);
	}
	return FALSE;
}

void SetMsgFilter(FMsgFilter pfnFilter)
{
	g_pfnMsgFilter = pfnFilter;
}

void DbgPrintWndMap()
{
	auto pCtx = GetThreadCtx();
	std::wstring s = std::format(L"当前线程（TID = {}）窗口映射表内容：\n", GetCurrentThreadId());
	CRefStrW rs(64);
	for (const auto& e : pCtx->hmWnd)
	{
		auto sText = e.second->GetText();
		if (!sText.Data())
			sText = L" ";
		rs.ReSize(sText.Size() + 64);
		swprintf(rs.Data(), L"\tCWnd指针 = 0x%0p，HWND = 0x%0p，标题 = %s\n",
			(PCVOID)e.second,
			(PCVOID)e.first,
			sText.Data());
		s += rs.Data();
	}
	s += std::format(L"共有{}个窗口\n", pCtx->hmWnd.size());
	OutputDebugStringW(s.c_str());
}

const CRefStrW& GetRunningPath()
{
	return g_rsCurrDir;
}
ECK_NAMESPACE_END