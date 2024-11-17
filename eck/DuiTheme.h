/*
* WinEzCtrlKit Library
*
* DuiTheme.h ： DUI主题管理器
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "SystemHelper.h"
#include "GraphicsHelper.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

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

	UserBegin = 0x1000,
};
// 几何类型
enum class GeoType : UINT
{
	FillBegin,
	FillRect,
	FillRoundRect,
	FillEllipse,
	FillEnd,

	FrameBegin,
	FrameRect,
	FrameRoundRect,
	FrameEllipse,
	FrameEnd,

	FillFrameBegin,
	FillFrameRect,
	FillFrameRoundRect,
	FillFrameEllipse,
	FillFrameEnd,

	Line,
};
EckInline constexpr BOOL GeoIsFill(GeoType eType) { return eType > GeoType::FillBegin && eType < GeoType::FillEnd; }
EckInline constexpr BOOL GeoIsFrame(GeoType eType) { return eType > GeoType::FrameBegin && eType < GeoType::FrameEnd; }
EckInline constexpr BOOL GeoIsFillFrame(GeoType eType) { return eType > GeoType::FillFrameBegin && eType < GeoType::FillFrameEnd; }
// 几何参数
struct GEO_PARAM
{
	union
	{
		D2D1_RECT_F rc;
		D2D1_ROUNDED_RECT rrc;
		D2D1_ELLIPSE ell;
		D2D1_POINT_2F pt[2];
	};
};

enum class Metrics
{
	CxVScroll,		// 垂直滚动条宽度
	CxVThumb,		// 垂直滚动条滑块宽度
	CyMinVThumb,	// 最小垂直滚动条滑块高度
	CyHScroll,		// 水平滚动条高度
	CyHThumb,		// 水平滚动条滑块高度
	CxMinHThumb,	// 最小水平滚动条滑块宽度
	Padding,		// 通用边距
	LargePadding,	// 通用边距（大）
	CxFocusBorder,	// 焦点矩形水平边距
	CyFocusBorder,	// 焦点矩形垂直边距
	CxIconSpacing,	// 图标水平间距
	CyIconSpacing,	// 图标垂直间距
	Max
};
// 主题文件标识
constexpr inline UINT ThemeFileMagic = 'eKdT';
// 图片数据偏移
struct THEME_OFFSET
{
	UINT ocb;
	UINT cb;
};
// 文件头
struct THEME_FILE_HEADER
{
	UINT Magic;
	int iVer;
	UINT uFlags;
	UINT cAtlas;
	UINT cImage;
	float fMetrics[(size_t)Metrics::Max];
	/*
	* THEME_OFFSET ofAtlas[cAtlas];
	* THEME_OFFSET ofImage[cImage];
	*/
};

struct THEME_CLR
{
	D2D1_COLOR_F crBk;
	D2D1_COLOR_F crText;
	D2D1_COLOR_F crBorder;
};

enum DtbOptFlag : UINT
{
	DTBO_NONE = 0,
	DTBO_CLIP_RECT = 1u << 0,
	DTBO_NEW_COLOR = 1u << 1,
	DTBO_NEW_STROKE_WIDTH = 1u << 2,
	DTBO_NEW_STROKE_STYLE = 1u << 3,
	DTBO_NEW_OPACITY = 1u << 4,
	DTBO_NEW_INTERPOLATION_MODE = 1u << 5,
};

struct DTB_OPT
{
	DtbOptFlag uFlags;
	D2D1_RECT_F rcClip;
	THEME_CLR CustomColor;
	float fStrokeWidth;
	ID2D1StrokeStyle* pStrokeStyle;
	float fOpacity;
	D2D1_INTERPOLATION_MODE eInterpolationMode;
};
constexpr inline DTB_OPT DtbOptDefault{};

class CTheme : public IUnknown
{
	friend class CThemeRealization;
protected:
	struct STATE
	{
		State eState;
		USHORT idxImgOrAtlas;
		BITBOOL b9Grid : 1;
		BITBOOL bGeometry : 1;
		BITBOOL bImg : 1;
		BITBOOL bNoStretch : 1;
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
				float fWidth;
				GEO_PARAM Param;
			} Geo;
		};
		THEME_CLR Color;
	};

	struct PART
	{
		Part ePart;
		int idxState;
		int cState;
	};

	ULONG m_cRef{ 1 };
	BOOL m_bNeedFreeData{};
	const THEME_FILE_HEADER* m_pHdr{};
	std::vector<PART> m_vPart{};	// 部件ID递增排列
	std::vector<STATE> m_vState{};	// 状态ID递增排列
	std::vector<IWICBitmap*> m_vAtlas{};// 图集
	std::vector<IWICBitmap*> m_vImage{};// 单个图片
	THEME_OFFSET* m_pofAtlas{};
	THEME_OFFSET* m_pofImage{};
	PCBYTE m_pData{};
	SIZE_T m_cbData{};

	EckInline constexpr auto FindPart(Part ePart) const
	{
		const auto it = std::lower_bound(m_vPart.begin(), m_vPart.end(), ePart,
			[](const PART& e, Part ePart) { return e.ePart < ePart; });
		if (it != m_vPart.end() && it->ePart == ePart)
			return it;
		return m_vPart.end();
	}

	EckInline constexpr auto FindPart(Part ePart)
	{
		const auto it = std::lower_bound(m_vPart.begin(), m_vPart.end(), ePart,
			[](const PART& e, Part ePart) { return e.ePart < ePart; });
		if (it != m_vPart.end() && it->ePart == ePart)
			return it;
		return m_vPart.end();
	}

	EckInline constexpr auto FindState(State eState, const PART& Part) const
	{
		const auto itEnd = m_vState.begin() + Part.idxState + Part.cState;
		const auto it = std::lower_bound(m_vState.begin() + Part.idxState,
			itEnd, eState,
			[](const STATE& e, State eState) { return e.eState < eState; });
		if (it != itEnd && it->eState == eState)
			return it;
		return m_vState.end();
	}

	EckInline constexpr auto FindState(State eState, const PART& Part)
	{
		const auto itEnd = m_vState.begin() + Part.idxState + Part.cState;
		const auto it = std::lower_bound(m_vState.begin() + Part.idxState,
			itEnd, eState,
			[](const STATE& e, State eState) { return e.eState < eState; });
		if (it != itEnd && it->eState == eState)
			return it;
		return m_vState.end();
	}

	EckInline constexpr auto GetIterPartEnd() const { return m_vPart.end(); }
	EckInline constexpr auto GetIterStateEnd() const { return m_vState.end(); }

	IWICBitmap* LoadAtlas(UINT idxAtlas, _Out_ HRESULT& hr)
	{
		if (idxAtlas < 0 || idxAtlas >= GetInfo().cAtlas)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
			return nullptr;
		}
		if (m_vAtlas[idxAtlas])
		{
			hr = S_FALSE;
			return m_vAtlas[idxAtlas];
		}
		const auto pStream = new CStreamView{ m_pData + m_pofAtlas[idxAtlas].cb,
			m_pofAtlas[idxAtlas].ocb };
		hr = CreateWicBitmap(m_vAtlas[idxAtlas], pStream);
		pStream->Release();
		return m_vAtlas[idxAtlas];
	}

	IWICBitmap* LoadImg(UINT idxImage, _Out_ HRESULT& hr)
	{
		if (idxImage < 0 || idxImage >= GetInfo().cImage)
		{
			hr = HRESULT_FROM_WIN32(ERROR_INVALID_INDEX);
			return nullptr;
		}
		if (m_vImage[idxImage])
		{
			hr = S_FALSE;
			return m_vImage[idxImage];
		}
		const auto pStream = new CStreamView{ m_pData + m_pofImage[idxImage].cb,
			m_pofImage[idxImage].ocb };
		hr = CreateWicBitmap(m_vImage[idxImage], pStream);
		pStream->Release();
		return m_vImage[idxImage];
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CTheme);

	STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
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
		//NTSTATUS nts;
		//const auto hFile = NaOpenFile(pszFileName,
		//	FILE_GENERIC_READ, FILE_SHARE_READ, 0, &nts);
		//if (!hFile)
		//	return HRESULT_FROM_NT(nts);
		//const auto pData = VAlloc()
	}

	HRESULT Open(_In_reads_bytes_(cbData) PCVOID pData, SIZE_T cbData, BOOL bCopy = TRUE)
	{
		const auto* const pHdr = (const THEME_FILE_HEADER*)pData;
		if (pHdr->Magic != ThemeFileMagic)
			return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
		if (bCopy)
		{
			m_pData = (PCBYTE)pData;
			m_cbData = cbData;
		}
		else
		{
			const auto p = VAlloc(cbData);
			if (!p)
				return E_OUTOFMEMORY;
			memcpy(p, pData, cbData);
			m_pData = (PCBYTE)p;
			m_cbData = cbData;
		}
		m_pHdr = pHdr;
		m_bNeedFreeData = bCopy;
		return S_OK;
	}

	HRESULT Close()
	{
		if (!m_pData)
			return S_FALSE;
		if (m_bNeedFreeData)
			VFree((void*)m_pData);
		m_pData = nullptr;
		m_cbData = 0;
		return S_OK;
	}

	constexpr HRESULT GetColor(Part ePart, State eState,
		ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) const
	{
		const auto itPart = FindPart(ePart);
		if (itPart == m_vPart.end())
			return HrNotFound;
		const auto itState = FindState(eState, *itPart);
		if (itState == m_vState.end())
			return HrNotFound;
		switch (eClrPart)
		{
		case ClrPart::Bk:
			cr = itState->Color.crBk;
			break;
		case ClrPart::Text:
			cr = itState->Color.crText;
			break;
		case ClrPart::Border:
			cr = itState->Color.crBorder;
			break;
		default:
			return E_INVALIDARG;
		}
		return S_OK;
	}

	HRESULT GetPartSize(Part ePart, State eState ,
		_Out_ D2D1_SIZE_F& sz, _In_opt_ const D2D1_RECT_F* prc = nullptr) const
	{
		const auto itPart = FindPart(ePart);
		if (itPart == m_vPart.end())
			return HrNotFound;
		const auto itState = FindState(eState, *itPart);
		if (itState == m_vState.end())
			return HrNotFound;
		if (itState->bGeometry)
		{
			switch (itState->Geo.eType)
			{
			case GeoType::FillRect:
			case GeoType::FrameRect:
			case GeoType::FillFrameRect:
				if(prc&&!itState->bNoStretch)
				{
					sz.width = prc->right - prc->left;
					sz.height = prc->bottom - prc->top;
				}
				else
				{
					sz.width = itState->Geo.Param.rc.right - itState->Geo.Param.rc.left;
					sz.height = itState->Geo.Param.rc.bottom - itState->Geo.Param.rc.top;
				}
				break;
			case GeoType::FillRoundRect:
			case GeoType::FrameRoundRect:
			case GeoType::FillFrameRoundRect:
				if(prc&&!itState->bNoStretch)
				{
					sz.width = prc->right - prc->left;
					sz.height = prc->bottom - prc->top;
				}
				else
				{
					sz.width = itState->Geo.Param.rrc.rect.right - itState->Geo.Param.rrc.rect.left;
					sz.height = itState->Geo.Param.rrc.rect.bottom - itState->Geo.Param.rrc.rect.top;
				}
				break;
			case GeoType::FillEllipse:
			case GeoType::FrameEllipse:
			case GeoType::FillFrameEllipse:
				if(prc&&!itState->bNoStretch)
				{
					sz.width = prc->right - prc->left;
					sz.height = prc->bottom - prc->top;
				}
				else
				{
					sz.width = itState->Geo.Param.ell.radiusX * 2;
					sz.height = itState->Geo.Param.ell.radiusY * 2;
				}
				break;
			case GeoType::Line:
				if(prc&&!itState->bNoStretch)
				{
					sz.width = prc->right - prc->left;
					sz.height = prc->bottom - prc->top;
				}
				else
				{
					sz.width = itState->Geo.Param.pt[1].x - itState->Geo.Param.pt[0].x;
					sz.height = itState->Geo.Param.pt[1].y - itState->Geo.Param.pt[0].y;
				}
				break;
			default:
				return E_INVALIDARG;
			}
		}
		else
		{

		}
	}

	constexpr BOOL IsPartValid(Part ePart, State eState) const
	{
		const auto it = FindPart(ePart);
		if (it != m_vPart.end())
		{
			const auto itState = FindState(eState, *it);
			return itState != m_vState.end();
		}
		return FALSE;
	}

	EckInline constexpr const THEME_FILE_HEADER& GetInfo() const { return *m_pHdr; }
};

class CThemeRealization : public IUnknown
{
protected:
	ID2D1DeviceContext* m_pDC;	// 关联DC
	CTheme* m_pTheme;			// 关联主题

	std::vector<ID2D1Bitmap1*> m_vAtlas;// 图集实现
	std::vector<ID2D1Bitmap1*> m_vImage;// 单个图片实现

	ID2D1SolidColorBrush* m_pBrush{};	// 通用画刷

	ULONG m_cRef{ 1 };

public:
	STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
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

	ECK_DISABLE_COPY_MOVE(CThemeRealization);
	CThemeRealization(ID2D1DeviceContext* pDC, CTheme* pTheme)
		: m_pDC{ pDC }, m_pTheme{ pTheme },
		m_vAtlas{ pTheme->GetInfo().cAtlas },
		m_vImage{ pTheme->GetInfo().cImage }
	{
		m_pTheme->AddRef();
		m_pDC->AddRef();
		m_pDC->CreateSolidColorBrush({}, &m_pBrush);
	}

	~CThemeRealization()
	{
		m_pDC->Release();
		m_pTheme->Release();
		m_pBrush->Release();
		for (const auto p : m_vAtlas)
			if (p)
				p->Release();
		for (const auto p : m_vImage)
			if (p)
				p->Release();
	}

	HRESULT DrawBackground(Part ePart, State eState,
		const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt = nullptr)
	{
		if (!pOpt)
			pOpt = &DtbOptDefault;
		if (pOpt->uFlags & DTBO_CLIP_RECT)
			m_pDC->PushAxisAlignedClip(pOpt->rcClip, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		const auto itPart = m_pTheme->FindPart(ePart);
		if (itPart == m_pTheme->GetIterPartEnd())
			return HrNotFound;
		const auto itState = m_pTheme->FindState(eState, *itPart);
		if (itState == m_pTheme->GetIterStateEnd())
			return HrNotFound;
		if (itState->bGeometry)
		{
			GEO_PARAM Geo;
			const auto& Param = itState->Geo.Param;
			float fWidth;
			ID2D1StrokeStyle* pStrokeStyle;

			if (pOpt->uFlags & DTBO_NEW_OPACITY)
				m_pBrush->SetOpacity(pOpt->fOpacity);

			if (GeoIsFill(itState->Geo.eType) ||
				GeoIsFillFrame(itState->Geo.eType))
			{
				m_pBrush->SetColor(pOpt->uFlags & DTBO_NEW_COLOR ?
					pOpt->CustomColor.crBk : itState->Color.crBk);
			}
			else
			{
				fWidth = pOpt->uFlags & DTBO_NEW_STROKE_WIDTH ?
					pOpt->fStrokeWidth : itState->Geo.fWidth;
				pStrokeStyle = pOpt->uFlags & DTBO_NEW_STROKE_STYLE ?
					pOpt->pStrokeStyle : nullptr;
				m_pBrush->SetColor(pOpt->uFlags & DTBO_NEW_COLOR ?
					pOpt->CustomColor.crBorder : itState->Color.crBorder);
			}

			switch (itState->Geo.eType)
			{
			case GeoType::FillRect:
			case GeoType::FrameRect:
			case GeoType::FillFrameRect:
				Geo.rc = Param.rc;
				if (itState->bNoStretch)
					CenterRect(Geo.rc, rc);
				break;
			case GeoType::FillRoundRect:
			case GeoType::FrameRoundRect:
			case GeoType::FillFrameRoundRect:
				Geo.rrc.radiusX = Param.rrc.radiusX;
				Geo.rrc.radiusY = Param.rrc.radiusY;
				if (itState->bNoStretch)
				{
					Geo.rrc.rect = Param.rrc.rect;
					CenterRect(Geo.rrc.rect, rc);
				}
				else
					Geo.rrc.rect = rc;
				break;
			case GeoType::FillEllipse:
			case GeoType::FrameEllipse:
			case GeoType::FillFrameEllipse:
				Geo.ell.point.x = (rc.left + rc.right) / 2.f;
				Geo.ell.point.y = (rc.top + rc.bottom) / 2.f;
				if (itState->bNoStretch)
				{
					Geo.ell.radiusX = Param.ell.radiusX;
					Geo.ell.radiusY = Param.ell.radiusY;
				}
				else
				{
					Geo.ell.radiusX = (rc.right - rc.left) / 2.f;
					Geo.ell.radiusY = (rc.bottom - rc.top) / 2.f;
				}
				break;
			case GeoType::Line:
				if (itState->bNoStretch)
				{
					Geo.rc.left = Param.pt[0].x;
					Geo.rc.top = Param.pt[0].y;
					Geo.rc.right = Param.pt[1].x;
					Geo.rc.bottom = Param.pt[1].y;
					CenterRect(Geo.rc, rc);
					Geo.pt[0].x = Geo.rc.left;
					Geo.pt[0].y = Geo.rc.top;
					Geo.pt[1].x = Geo.rc.right;
					Geo.pt[1].y = Geo.rc.bottom;
				}
				else
				{
					Geo.pt[0].x = rc.left;
					Geo.pt[0].y = rc.top;
					Geo.pt[1].x = rc.right;
					Geo.pt[1].y = rc.bottom;
				}
				break;
			default:
				return E_INVALIDARG;
			}

			switch (itState->Geo.eType)
			{
			case GeoType::FillRect:			m_pDC->FillRectangle(Geo.rc, m_pBrush);			break;
			case GeoType::FillRoundRect:	m_pDC->FillRoundedRectangle(Geo.rrc, m_pBrush); break;
			case GeoType::FillEllipse:		m_pDC->FillEllipse(Geo.ell, m_pBrush);			break;

			case GeoType::FrameRect:
				m_pDC->DrawRectangle(Geo.rc, m_pBrush, fWidth, pStrokeStyle);			break;
			case GeoType::FrameRoundRect:
				m_pDC->DrawRoundedRectangle(Geo.rrc, m_pBrush, fWidth, pStrokeStyle);	break;
			case GeoType::FrameEllipse:
				m_pDC->DrawEllipse(Geo.ell, m_pBrush, fWidth, pStrokeStyle);			break;

			case GeoType::FillFrameRect:
				m_pDC->FillRectangle(Geo.rc, m_pBrush);
				m_pBrush->SetColor(pOpt->uFlags & DTBO_NEW_COLOR ?
					pOpt->CustomColor.crBorder : itState->Color.crBorder);
				m_pDC->DrawRectangle(Geo.rc, m_pBrush, fWidth, pStrokeStyle);
				break;
			case GeoType::FillFrameRoundRect:
				m_pDC->FillRoundedRectangle(Geo.rrc, m_pBrush);
				m_pBrush->SetColor(pOpt->uFlags & DTBO_NEW_COLOR ?
					pOpt->CustomColor.crBorder : itState->Color.crBorder);
				m_pDC->DrawRoundedRectangle(Geo.rrc, m_pBrush, fWidth, pStrokeStyle);
				break;
			case GeoType::FillFrameEllipse:
				m_pDC->FillEllipse(Geo.ell, m_pBrush);
				m_pBrush->SetColor(pOpt->uFlags & DTBO_NEW_COLOR ?
					pOpt->CustomColor.crBorder : itState->Color.crBorder);
				m_pDC->DrawEllipse(Geo.ell, m_pBrush, fWidth, pStrokeStyle);
				break;

			case GeoType::Line:
				m_pDC->DrawLine(Geo.pt[0], Geo.pt[1], m_pBrush, fWidth, pStrokeStyle);
				break;
			default: ECK_UNREACHABLE;
			}
		}
		else
		{
			ID2D1Bitmap1* pBitmap;
			if (itState->bImg)
			{
				if (m_vImage[itState->idxImgOrAtlas])
					pBitmap = m_vImage[itState->idxImgOrAtlas];
				else
				{
					HRESULT hr;
					const auto pWicBmp = m_pTheme->LoadImg(itState->idxImgOrAtlas, hr);
					if (FAILED(hr))
						return hr;
					if (FAILED(hr = m_pDC->CreateBitmapFromWicBitmap(pWicBmp, nullptr, &pBitmap)))
						return hr;
					m_vImage[itState->idxImgOrAtlas] = pBitmap;
				}
			}
			else
			{
				if (m_vAtlas[itState->idxImgOrAtlas])
					pBitmap = m_vAtlas[itState->idxImgOrAtlas];
				else
				{
					HRESULT hr;
					const auto pWicBmp = m_pTheme->LoadAtlas(itState->idxImgOrAtlas, hr);
					if (FAILED(hr))
						return hr;
					if (FAILED(hr = m_pDC->CreateBitmapFromWicBitmap(pWicBmp, nullptr, &pBitmap)))
						return hr;
					m_vAtlas[itState->idxImgOrAtlas] = pBitmap;
				}
			}

			const auto fOpacity = pOpt->uFlags & DTBO_NEW_OPACITY ?
				pOpt->fOpacity : 1.f;
			const auto eInterMode = pOpt->uFlags & DTBO_NEW_INTERPOLATION_MODE ?
				pOpt->eInterpolationMode : D2D1_INTERPOLATION_MODE_LINEAR;
			if (itState->bNoStretch)
			{
				D2D1_RECT_F rcDst = itState->Img.rcInAtlas;
				CenterRect(rcDst, rc);
				if (itState->b9Grid)
					DrawImageFromGrid(m_pDC, pBitmap, rcDst, itState->Img.rcInAtlas,
						itState->Img.rc9Grid, eInterMode, fOpacity);
				else
					m_pDC->DrawBitmap(pBitmap, &rcDst, fOpacity, eInterMode,
						itState->bImg ? nullptr : &itState->Img.rcInAtlas);
			}
			else
			{
				if (itState->b9Grid)
					DrawImageFromGrid(m_pDC, pBitmap, rc, itState->Img.rcInAtlas,
						itState->Img.rc9Grid, eInterMode, fOpacity);
				else
					m_pDC->DrawBitmap(pBitmap, &rc, fOpacity, eInterMode,
						itState->bImg ? nullptr : &itState->Img.rcInAtlas);
			}
		}
		if (pOpt->uFlags & DTBO_CLIP_RECT)
			m_pDC->PopAxisAlignedClip();
		return S_OK;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END