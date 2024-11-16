#pragma once
#include "ECK.h"

#include <d2d1_2.h>

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 部件
enum class Part : int
{
	Invalid = -1,

	Button,
	RadioButton,
	CheckButton,
	GroupBox,

	Edit,

	List,
	ListItem,

	Header,
	HeaderItem,

	Progress,

	ScrollBar,
	ScrollButton,

	UserBegin = 0x1000,
};
// 状态
enum class State : int
{
	Normal,
	Hot,
	Pressed,
	Disabled,
	Checked,
	Focused,
	Selected,
	CheckedFocused,
	CheckedSelected,
	MaxState
};
// 几何类型
enum class GeoType : UINT
{
	FillRect,
	FillRoundRect,
	FillEllipse,
	FillGeometry,
	FrameRect,
	FrameRoundRect,
	FrameEllipse,
	FrameGeometry,
	FillFrameRect,
	FillFrameRoundRect,
	FillFrameEllipse,
	FillFrameGeometry,
};
// 几何参数
struct GEO_PARAM
{
	union
	{
		D2D1_RECT_F rc;
		D2D1_ROUNDED_RECT rrc;
		D2D1_ELLIPSE ell;
		ID2D1Geometry* pGeometry;
	};
};

constexpr inline UINT ThemeFileMagic = 'eKdT';

// 文件头
struct THEME_FILE_HEADER
{
	UINT Magic;
	int iVer;
	UINT uFlags;
	UINT cAtlas;
	UINT cImage;
	/*
	* UINT ocbAtlas[cAtlas];
	* UINT ocbImage[cImage];
	*/
};

class CTheme : public IUnknown
{
private:
	struct THEME_INFO
	{
		int cPart;
	};

	struct STATE
	{
		State eState;
		USHORT idxImgOrAtlas;
		BITBOOL b9Grid : 1;
		BITBOOL bIsGeo : 1;
		BITBOOL bIsImgOrAtlas : 1;
		union
		{
			struct
			{
				D2D1_RECT_F rcInAtlas;
				D2D1_RECT_F rc9Grid;
			} Img;
			struct
			{
				GeoType eType;
				GEO_PARAM Param;
			} Geo;
		};
	};

	struct PART
	{
		Part ePart;
		int idxState;
		int cState;
	};

	ULONG m_cRef{ 1 };

	THEME_INFO m_Info{};
	std::vector<PART> m_vPart{};
	std::vector<STATE> m_vState{};
public:
	STDMETHODIMP_(ULONG) AddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		ULONG cRef = InterlockedDecrement(&m_cRef);
		if (cRef == 0)
		{
			delete this;
		}
		return cRef;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
	{
		EckDbgBreak();
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	HRESULT Open(_In_z_ PCWSTR pszFileName)
	{

	}

	HRESULT Open(_In_reads_bytes_(cbData) PCVOID pData, SIZE_T cbData)
	{

	}

	HRESULT DrawBackground(ID2D1DeviceContext* pDC, Part ePart, State eState,
		const D2D1_RECT_F& rc, _In_opt_ const D2D1_RECT_F* prcClip)
	{

	}

	HRESULT GetColor(Part ePart, State eState, _Out_ D2D1_COLOR_F& cr) const
	{

	}

	BOOL IsPartValid(Part ePart, State eState) const
	{

	}
};

class CThemeRealization
{
	ID2D1DeviceContext1* m_pDC{};
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END