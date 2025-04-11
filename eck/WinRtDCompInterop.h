#pragma once
#include "ComPtr.h"

#include <dcomp.h>
#include <Windows.UI.Composition.h>
#include <winrt/Windows.UI.Composition.Desktop.h>

// ================================================================
// Written by ADeltaX
// https://blog.adeltax.com/interopcompositor-and-coredispatcher
// https://gist.github.com/ADeltaX/c0e565a50d2cedb62dab5c13674cb8b5
// ================================================================

DECLARE_INTERFACE_IID_(IInteropCompositorPartnerCallback, IUnknown,
	"9bb59fc9-3326-4c32-bf06-d6b415ac2bc5")
{
	STDMETHOD(NotifyDirty)(THIS_) PURE;
	STDMETHOD(NotifyDeferralState)(THIS_
		bool deferRequested) PURE;
};

DECLARE_INTERFACE_IID_(IInteropCompositorFactoryPartner, IInspectable,
	"22118adf-23f1-4801-bcfa-66cbf48cc51b")
{
	STDMETHOD(CreateInteropCompositor)(THIS_
		IN IUnknown * renderingDevice,
		IN IInteropCompositorPartnerCallback * callback,
		IN REFIID iid,
		OUT VOID * *instance
		) PURE;
	STDMETHOD(CheckEnabled)(THIS_
		OUT bool* enableInteropCompositor,
		OUT bool* enableExposeVisual
		) PURE;
};

DECLARE_INTERFACE_IID_(IInteropCompositorPartner, IUnknown,
	"e7894c70-af56-4f52-b382-4b3cd263dc6f")
{
	STDMETHOD(MarkDirty)(THIS_) PURE;
	STDMETHOD(ClearCallback)(THIS_) PURE;
	STDMETHOD(CreateManipulationTransform)(THIS_
		IN IDCompositionTransform * transform,
		IN REFIID iid,
		OUT VOID * *result) PURE;
	STDMETHOD(RealClose)(THIS_) PURE;
};

// 14393-15063
DECLARE_INTERFACE_IID_(HwndTarget, IUnknown, "6677DA68-C80C-407A-A4D2-3AA118AD7C46")
{
	STDMETHOD(GetRoot)(THIS_ OUT ABI::Windows::UI::Composition::IVisual * *value) PURE;
	STDMETHOD(SetRoot)(THIS_ IN ABI::Windows::UI::Composition::IVisual * value) PURE;
};

DECLARE_INTERFACE_IID_(InteropCompositionTarget, IUnknown, "EACDD04C-117E-4E17-88F4-D1B12B0E3D89")
{
	STDMETHOD(SetRoot)(THIS_ IN IDCompositionVisual * visual) PURE;
};

ECK_NAMESPACE_BEGIN
/// <summary>
/// 创建DirectComposition与Windows.UI.Composition互操作工厂
/// </summary>
/// <param name="pD2DDevice">D2D设备</param>
/// <param name="pFactory">返回工厂</param>
/// <param name="Compositor">返回合成器</param>
/// <param name="pDcDesktopDevice">返回IDCompositionDesktopDevice</param>
/// <returns>HRESULT</returns>
inline HRESULT DciCreateInteropCompositorFactory(
	_In_ ID2D1Device* pD2DDevice,
	_Out_ IInteropCompositorFactoryPartner*& pFactory,
	_Out_ winrt::Windows::UI::Composition::Compositor& Compositor,
	_Out_ IDCompositionDesktopDevice*& pDcDesktopDevice
) noexcept
{
	/*
	ActivationFactory
	|
	IInteropCompositorFactoryPartner
	|
	+--QI--ICompositor
	|
	+--QI--IDCompositionDesktopDevice
	*/
	try
	{
		pFactory = winrt::get_activation_factory<
			winrt::Windows::UI::Composition::Compositor,
			IInteropCompositorFactoryPartner>().detach();
	}
	catch (winrt::hresult_error const& e)
	{
		return e.to_abi();
	}
	HRESULT hr;
	ComPtr<IInteropCompositorPartner> pPartner;
	hr = pFactory->CreateInteropCompositor(pD2DDevice, nullptr,
		IID_PPV_ARGS(&pPartner));
	if (FAILED(hr))
		return hr;
	hr = pPartner->QueryInterface(
		__uuidof(ABI::Windows::UI::Composition::ICompositor),
		winrt::put_abi(Compositor));
	if (FAILED(hr))
		return hr;
	hr = pPartner->QueryInterface(&pDcDesktopDevice);
	return hr;
}

/// <summary>
/// 为互操作合成器创建窗口目标。
/// winrt侧互操作合成器创建的视觉对象无法QI为Win32 DComp视觉对象，
/// 但反过来，互操作DComp设备创建的Win32 DComp视觉对象可以QI为Visual。
/// 可使用合成器和DComp分别创建所需对象。
/// 16299后最佳做法：只用合成器管理可视化树。
/// </summary>
/// <param name="hWnd">窗口句柄</param>
/// <param name="bTopmost">可视化树是否显示在分层子窗口之上</param>
/// <param name="Compositor">互操作合成器</param>
/// <param name="pDcDesktopDevice">互操作DComp设备</param>
/// <param name="pRootVisual">可选的根视觉对象，若不为空，则窗口目标的根视觉对象设置为该对象</param>
/// <param name="pDcTarget_">返回IDCompositionTarget</param>
/// <param name="Target">【16299后使用】此参数与下一参数任选其一，返回必要的目标对象，可被QI为InteropCompositionTarget</param>
/// <param name="pHwndTarget">【14393-15063使用】参看上一参数</param>
/// <returns>HRESULT</returns>
inline HRESULT DciCreateWindowTarget(
	_In_ HWND hWnd,
	_In_ BOOL bTopmost,
	_In_ winrt::Windows::UI::Composition::Compositor Compositor,
	_In_ IDCompositionDesktopDevice* pDcDesktopDevice,
	_In_ winrt::Windows::UI::Composition::Visual pRootVisual,
	_Out_ IDCompositionTarget*& pDcTarget_,
	_Out_ winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget& Target,
	_Out_ HwndTarget*& pHwndTarget
) noexcept
{
	/*
	IDCompositionDesktopDevice
	|
	IDCompositionTarget
	|
	|   +--QI--HwndTarget---------------QI--+
	+--or                                   +--InteropCompositionTarget
		+--QI--InteropCompositionTarget-QI--+
	*/
	HRESULT hr;
	winrt::com_ptr<IDCompositionTarget> pDcTarget;
	hr = pDcDesktopDevice->CreateTargetForHwnd(hWnd, bTopmost, pDcTarget.put());
	if (FAILED(hr))
		return hr;
	Target = pDcTarget.try_as<winrt::Windows::UI::Composition::
		Desktop::DesktopWindowTarget>();
	if (Target)
	{
		if (pRootVisual)
		{
			try {
				Target.Root(pRootVisual);
			}
			catch (const winrt::hresult_error& e) {
				hr = e.to_abi();
			}
		}
		pHwndTarget = nullptr;
	}
	else
	{
		pHwndTarget = pDcTarget.try_as<HwndTarget>().detach();
		if (!pHwndTarget)
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		if (pRootVisual)
			hr = pHwndTarget->SetRoot((ABI::Windows::UI::Composition::IVisual*)
				winrt::get_abi(pRootVisual));
		Target = nullptr;
	}
	pDcTarget_ = pDcTarget.detach();
	return hr;
}
ECK_NAMESPACE_END