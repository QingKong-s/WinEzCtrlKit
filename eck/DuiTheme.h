/*
* WinEzCtrlKit Library
*
* DuiTheme.h ： DUI主题管理器
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#include "CFile.h"
#include "GraphicsHelper.h"
#include "CUnknown.h"

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
	TriStateButton,
	GroupBox,

	Edit,

	List,
	ListItem,

	Header,
	HeaderItem,

	Progress,

	TrackBar,
	TrackBarThumb,

	ScrollBar,
	ScrollButton,
	ScrollThumb,

	UserBegin = 0x1000,
};
// 状态
enum class State : int
{
	None,		// 若部件不关心状态，则此值作为占位符
	// 以下是基本状态
	Normal,		// 默认状态
	Hot,		// 点燃
	Selected,	// 选中/按下
	Disabled,	// 禁用
	// 以下状态可与基本状态组合
	Checked,	// 检查（检查框选中等）
	Mixed,		// 半检查（三态复选等）
	Focused,	// 焦点
	// 以下为组合状态
	HotSelected,	// 点燃且选中

	CheckedHot,		// 检查且点燃
	CheckedSelected,// 检查且选中
	CheckedDisabled,// 检查且禁用

	MixedHot,		// 半检查且点燃
	MixedSelected,	// 半检查且选中
	MixedDisabled,	// 半检查且禁用

	NoFocusSelected,// 无焦点且选中

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
// 状态描述
struct STATE_INFO
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
// 部件描述
struct PART_INFO
{
	Part ePart;
	int idxState;
	int cState;
};
// 主题度量
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

	Max				// 度量索引的最大值
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
	UINT cPart;
	UINT cState;
	float fMetrics[(size_t)Metrics::Max];
	/*
	* THEME_OFFSET ofAtlas[cAtlas];
	* THEME_OFFSET ofImage[cImage];
	* PART_INFO Part[cPart];	// 部件ID递增排列
	* STATE_INFO State[cState];	// 状态ID递增排列
	*/
};
// 颜色信息
struct THEME_CLR
{
	D2D1_COLOR_F crBk;
	D2D1_COLOR_F crText;
	D2D1_COLOR_F crBorder;
};
// DrawBackground选项
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
// DrawBackground选项
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
// 默认DrawBackground选项
constexpr inline DTB_OPT DtbOptDefault{};

// 主题描述
class CTheme : public CRefObjMultiThread<CTheme>
{
	friend class CThemeRealization;
protected:
	ULONG m_cRef{ 1 };
	BOOL m_bNeedFreeData{};
	std::vector<IWICBitmap*> m_vAtlas{};// 图集
	std::vector<IWICBitmap*> m_vImage{};// 单个图片

	PCBYTE m_pData{};
	SIZE_T m_cbData{};
	// 以下字段基于m_pData字段
	const THEME_FILE_HEADER* m_pHdr{};	// 文件头
	THEME_OFFSET* m_pofAtlas{};		// 图集偏移
	THEME_OFFSET* m_pofImage{};		// 单个图片偏移
	const PART_INFO* m_pPart{};		// 部件ID递增排列
	const STATE_INFO* m_pState{};	// 状态ID递增排列

	EckInline constexpr auto GetIterPartEnd() const { return m_pPart + m_pHdr->cPart; }
	EckInline constexpr auto GetIterStateEnd() const { return m_pState + m_pHdr->cState; }

	EckInline constexpr auto FindPart(Part ePart) const
	{
		const auto it = std::lower_bound(m_pPart, GetIterPartEnd(), ePart,
			[](const PART_INFO& e, Part ePart) { return e.ePart < ePart; });
		if (it != GetIterPartEnd() && it->ePart == ePart)
			return it;
		return GetIterPartEnd();
	}

	EckInline constexpr auto FindPart(Part ePart)
	{
		const auto it = std::lower_bound(m_pPart, GetIterPartEnd(), ePart,
			[](const PART_INFO& e, Part ePart) { return e.ePart < ePart; });
		if (it != GetIterPartEnd() && it->ePart == ePart)
			return it;
		return GetIterPartEnd();
	}

	EckInline constexpr auto FindState(State eState, const PART_INFO& Part) const
	{
		const auto itEnd = m_pState + Part.idxState + Part.cState;
		const auto it = std::lower_bound(m_pState + Part.idxState,
			itEnd, eState,
			[](const STATE_INFO& e, State eState) { return e.eState < eState; });
		if (it != itEnd && it->eState == eState)
			return it;
		return GetIterStateEnd();
	}

	EckInline constexpr auto FindState(State eState, const PART_INFO& Part)
	{
		const auto itEnd = m_pState + Part.idxState + Part.cState;
		const auto it = std::lower_bound(m_pState + Part.idxState,
			itEnd, eState,
			[](const STATE_INFO& e, State eState) { return e.eState < eState; });
		if (it != itEnd && it->eState == eState)
			return it;
		return GetIterStateEnd();
	}

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

	void PostOpenData()
	{
		m_vAtlas.resize(m_pHdr->cAtlas);
		m_vImage.resize(m_pHdr->cImage);
		m_pofAtlas = (THEME_OFFSET*)(m_pHdr + 1);
		m_pofImage = (THEME_OFFSET*)(m_pofAtlas + m_pHdr->cAtlas);
		m_pPart = (PART_INFO*)(m_pofImage + m_pHdr->cImage);
		m_pState = (STATE_INFO*)(m_pPart + m_pHdr->cPart);
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CTheme);
	~CTheme()
	{
		Close();
	}

	HRESULT Open(_In_z_ PCWSTR pszFileName)
	{
		Close();
		CFile File{ pszFileName,FCD_ONLYEXISTING,FILE_GENERIC_READ };
		if (!File.IsValid())
			return HRESULT_FROM_WIN32(GetLastError());
		const auto cbData = File.GetSize32();
		const auto pData = VAlloc(cbData);
		if (!pData)
			return E_OUTOFMEMORY;
		if (!File.Read(pData, cbData))
		{
			VFree(pData);
			return HRESULT_FROM_WIN32(GetLastError());
		}
		const auto* const pHdr = (const THEME_FILE_HEADER*)pData;
		if (pHdr->Magic != ThemeFileMagic)
		{
			VFree(pData);
			return HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
		}
		m_pData = (PCBYTE)pData;
		m_cbData = cbData;
		m_pHdr = pHdr;
		m_bNeedFreeData = TRUE;
		PostOpenData();
		return S_OK;
	}

	HRESULT Open(_In_reads_bytes_(cbData) PCVOID pData, SIZE_T cbData, BOOL bCopy = TRUE)
	{
		Close();
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
		PostOpenData();
		return S_OK;
	}

	HRESULT Close()
	{
		if (!m_pData)
			return S_FALSE;
		if (m_bNeedFreeData)
			VFree((void*)m_pData);
		m_bNeedFreeData = FALSE;
		m_pHdr = nullptr;
		m_pPart = nullptr;
		m_pState = nullptr;
		m_vAtlas.clear();
		m_vImage.clear();
		m_pofAtlas = nullptr;
		m_pofImage = nullptr;
		m_pData = nullptr;
		m_cbData = 0;
		return S_OK;
	}

	constexpr HRESULT GetColor(Part ePart, State eState,
		ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) const
	{
		const auto itPart = FindPart(ePart);
		if (itPart == GetIterPartEnd())
			return HrNotFound;
		const auto itState = FindState(eState, *itPart);
		if (itState == GetIterStateEnd())
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

	EckInline constexpr float GetMetric(Metrics eMetric) const
	{
		return m_pHdr->fMetrics[(size_t)eMetric];
	}

	constexpr BOOL IsPartValid(Part ePart, State eState) const
	{
		const auto it = FindPart(ePart);
		if (it != GetIterPartEnd())
		{
			const auto itState = FindState(eState, *it);
			return itState != GetIterStateEnd();
		}
		return FALSE;
	}

	EckInline constexpr const THEME_FILE_HEADER& GetInfo() const { return *m_pHdr; }
};

// 基于指定DC的主题实现
class CThemeRealization : public CRefObjMultiThread<CThemeRealization>
{
protected:
	ID2D1DeviceContext* m_pDC;	// 关联DC
	CTheme* m_pTheme;			// 关联主题

	std::vector<ID2D1Bitmap1*> m_vAtlas;// 图集实现
	std::vector<ID2D1Bitmap1*> m_vImage;// 单个图片实现

	ID2D1SolidColorBrush* m_pBrush{};	// 通用画刷

	ULONG m_cRef{ 1 };

	ID2D1Bitmap1* RealizeAtlas(UINT idxAtlas, _Out_ HRESULT& hr)
	{
		if (m_vAtlas[idxAtlas])
		{
			hr = S_FALSE;
			return m_vAtlas[idxAtlas];
		}
		else
		{
			const auto pWicBmp = m_pTheme->LoadAtlas(idxAtlas, hr);
			if (FAILED(hr))
				return nullptr;
			if (FAILED(hr = m_pDC->CreateBitmapFromWicBitmap(pWicBmp,
				nullptr, &m_vAtlas[idxAtlas])))
				return nullptr;
			return m_vAtlas[idxAtlas];
		}
	}

	ID2D1Bitmap1* RealizeImg(UINT idxImage, _Out_ HRESULT& hr)
	{
		if (m_vImage[idxImage])
		{
			hr = S_FALSE;
			return m_vImage[idxImage];
		}
		else
		{
			const auto pWicBmp = m_pTheme->LoadImg(idxImage, hr);
			if (FAILED(hr))
				return nullptr;
			if (FAILED(hr = m_pDC->CreateBitmapFromWicBitmap(pWicBmp,
				nullptr, &m_vImage[idxImage])))
				return nullptr;
			return m_vImage[idxImage];
		}
	}
public:
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

	/// <summary>
	/// 画背景。
	/// 提供默认的基于主题数据的背景绘制实现
	/// </summary>
	/// <param name="ePart">部件ID</param>
	/// <param name="eState">状态ID</param>
	/// <param name="rc">输出矩形</param>
	/// <param name="pOpt">选项</param>
	/// <returns>HRESULT</returns>
	virtual HRESULT DrawBackground(Part ePart, State eState,
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
			HRESULT hr;
			if (itState->bImg)
			{
				pBitmap = RealizeImg(itState->idxImgOrAtlas, hr);
				if (FAILED(hr))
					return hr;
			}
			else
			{
				pBitmap = RealizeAtlas(itState->idxImgOrAtlas, hr);
				if (FAILED(hr))
					return hr;
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