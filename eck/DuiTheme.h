#pragma once
#include "CFile.h"
#include "GraphicsHelper.h"
#include "CUnknown.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
// 部件
enum class Part : BYTE
{
	Button,
	CircleButton,
	RadioButton,
	CheckButton,
	TriStateCheckButton,
	GroupBox,

	Edit,
	EditBottomBar,

	List,
	ListSelRect,
	ListItem,

	Header,
	HeaderItem,

	Progress,

	TrackBar,
	TrackBarThumb,

	ScrollBar,
	ScrollButton,
	ScrollThumb,

	LabelText,
	LabelBk,

	MaxStdControl,

	Invalid = 0xFF,
	UserBegin = 128,
};
// 状态
enum class State : BYTE
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

	UserBegin = 128,
};
// 几何类型
enum class GeoType : UINT
{
	FillBegin,
	FillRect,
	FillRoundRect,
	FillEllipse,
	Ring,
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
	FrameRing,
	FillFrameEnd,

	Line,
	PloyLine,
};
EckInline constexpr BOOL GeoIsFill(GeoType eType) { return eType > GeoType::FillBegin && eType < GeoType::FillEnd; }
EckInline constexpr BOOL GeoIsFrame(GeoType eType) { return eType > GeoType::FrameBegin && eType < GeoType::FrameEnd; }
EckInline constexpr BOOL GeoIsFillFrame(GeoType eType) { return eType > GeoType::FillFrameBegin && eType < GeoType::FillFrameEnd; }

struct RING_PARAM
{
	D2D1_ELLIPSE ellOutter;
	D2D1_ELLIPSE ellInner;
};
// 几何参数
struct GEO_PARAM
{
	union
	{
		D2D1_RECT_F rc;
		D2D1_ROUNDED_RECT rrc;
		D2D1_ELLIPSE ell;
		D2D1_POINT_2F pt[2];
		RING_PARAM ring;
		struct
		{
			UINT cPoints;
			D2D1_POINT_2F ptPloy[4];
		};
	};
};
// 颜色信息
struct THEME_CLR
{
	D2D1_COLOR_F crBk;
	D2D1_COLOR_F crText;
	D2D1_COLOR_F crBorder;
	D2D1_COLOR_F crExtra1;
};
// 基于调色板的颜色信息
struct THEME_CLR_IDX
{
	UINT idxClrBk;
	UINT idxClrText;
	UINT idxClrBorder;
	UINT idxClrExtra1;
};
// 绘制序列的范围
struct THEME_SEQ_RANGE
{
	UINT idxBegin;
	UINT cSeq;
};

constexpr inline UINT IdxColorizationColor = 0xFFFFFFFF;
// 状态描述
struct THEME_STATE
{
	State eState;
	USHORT idxImgOrAtlas;
	BITBOOL b9Grid : 1;			// 含九宫信息
	BITBOOL bGeometry : 1;		// 使用几何绘制
	BITBOOL bImg : 1;			// 使用单个图片而不是图集
	BITBOOL bNoStretch : 1;		// 不要拉伸到绘制区域
	BITBOOL bNoUsePalette : 1;	// 不使用调色板
	BITBOOL bUseSeq : 1;		// 使用绘制序列
	union
	{
		THEME_SEQ_RANGE SeqRange;	// bUseSeq为TRUE时有效
		struct
		{
			D2D1_RECT_F rcInAtlas;	// bImg为FALSE时有效
			D2D1_RECT_F rc9Grid;	// b9Grid为TRUE时有效
		} Img;// bGeometry为FALSE时有效
		struct
		{
			GeoType eType;
			float fWidth;	// 含有描边操作时有效
			GEO_PARAM Param;
		} Geo;// bGeometry为TRUE时有效
	};
	union
	{
		THEME_CLR_IDX ColorIdx;	// bNoUsePalette为FALSE时有效
		THEME_CLR Color;		// bNoUsePalette为TRUE时有效
	};
};
// 绘制序列
struct THEME_SEQ
{
	float fScale;
	THEME_STATE State;
};
// 部件描述
struct THEME_PART
{
	Part ePart;
	int idxState;
	int cState;
};
// 系统颜色
enum class SysColor
{
	Text,
	Bk,
	MainTitle,

	Max
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
	Padding,		// 通用空白，一般用于外部，如内容与边框之间
	LargePadding,	// 通用空白（大）
	SmallPadding,	// 通用空白（小），一般用于内部，如图片与文本之间
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
	UINT Magic;	// 标识
	int iVer;	// 版本
	UINT uFlags;// 标志
	UINT cAtlas;// 图集数量
	UINT cImage;// 单个图片数量
	UINT cPart;	// 部件数量
	UINT cState;// 状态数量
	UINT cSeq;	// 状态的绘制序列数量
	UINT cPalette;	// 调色板数量
	int onSysClr;	// 系统颜色在调色板中的偏移，若为负值则不使用调色板
	float fMetrics[(size_t)Metrics::Max];
	/*
	* THEME_OFFSET	ofAtlas[cAtlas];// 图集偏移
	* THEME_OFFSET	ofImage[cImage];// 单个图片偏移
	* THEME_PART	Part[cPart];	// 部件，ID递增排列
	* THEME_STATE	State[cState];	// 状态，ID递增排列
	* THEME_SEQ		Seq[cSeq];		// 状态的绘制序列
	* struct
	* {
	*  UINT			cEntries;
	*  D2D1_COLOR_F	Color[cPaletteEntries];
	* } Palette[cPalette];			// 调色板
	* D2D1_COLOR_F	SysClr[SysColor::Max];	// 系统颜色，仅当onSysClr < 0时有效
	*
	* [ ... ]
	* BYTE Data[];					// 图片数据
	* [ ... ]
	*/
};
// DrawBackground选项
enum DtbOptFlag : UINT
{
	DTBO_NONE = 0,
	DTBO_CLIP_RECT = 1u << 0,
	DTBO_NEW_COLOR = 1u << 1,
	DTBO_NEW_STROKE_WIDTH = 1u << 2,
	DTBO_NEW_OPACITY = 1u << 3,
	DTBO_NEW_STROKE_STYLE = 1u << 4,
	DTBO_NEW_INTERPOLATION_MODE = 1u << 5,
	DTBO_NEW_RADX = 1u << 6,
	DTBO_NEW_RADY = 1u << 7,
};
ECK_ENUM_BIT_FLAGS(DtbOptFlag);
// DrawBackground选项
struct DTB_OPT
{
	DtbOptFlag uFlags;
	D2D1_RECT_F rcClip;
	THEME_CLR CustomColor;
	float fStrokeWidth;
	float fOpacity;
	ID2D1StrokeStyle* pStrokeStyle;
	D2D1_INTERPOLATION_MODE eInterpolationMode;
	float fRadX;
	float fRadY;
};
// 默认DrawBackground选项
constexpr inline DTB_OPT DtbOptDefault{};

struct __declspec(uuid("85623275-F66F-4D96-8EFE-6F97E2519AC8"))
	ITheme : public IUnknown
{
	virtual ~ITheme() = default;
	virtual HRESULT DrawBackground(Part ePart, State eState,
		const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt) = 0;
	virtual HRESULT SetColorizationColor(const D2D1_COLOR_F& cr) = 0;
	virtual HRESULT GetColorizationColor(_Out_ D2D1_COLOR_F& cr) = 0;
	virtual HRESULT GetColor(Part ePart, State eState,
		ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) = 0;
	virtual HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) = 0;
	virtual float GetMetrics(Metrics eMetrics) = 0;
};

class CThemePalette : public CRefObj<CThemePalette>
{
protected:
	std::vector<D2D1_COLOR_F> m_vEntries{};
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CThemePalette);
	CThemePalette(const D2D1_COLOR_F* pEntries, UINT cEntries)
		: m_vEntries{ pEntries, pEntries + cEntries } {
	}
	virtual ~CThemePalette() = default;

	EckInline constexpr const auto& GetEntries() const { return m_vEntries; }

	EckInline constexpr const auto& GetColor(UINT idx) const { return m_vEntries[idx]; }
};
// 主题描述
class CTheme : public CRefObj<CTheme>
{
	friend class CThemeRealization;
protected:
	ULONG m_cRef{ 1 };
	BOOL m_bNeedFreeData{};
	std::vector<IWICBitmap*> m_vAtlas{};		// 图集
	std::vector<IWICBitmap*> m_vImage{};		// 单个图片
	std::vector<CThemePalette*> m_vPalette{};	// 调色板
	CThemePalette* m_pPalette{};				// 当前调色板

	PCBYTE m_pData{};
	SIZE_T m_cbData{};
	// 以下字段基于m_pData字段
	const THEME_FILE_HEADER* m_pHdr{};	// 文件头
	THEME_OFFSET* m_pofAtlas{};			// 图集偏移
	THEME_OFFSET* m_pofImage{};			// 单个图片偏移
	const THEME_PART* m_pPart{};		// 部件，ID递增排列
	const THEME_STATE* m_pState{};		// 状态，ID递增排列
	const THEME_SEQ* m_pSeq{};			// 状态的绘制序列
	const D2D1_COLOR_F* m_pSysClr{};	// 系统颜色，为nullptr则应使用调色板

	EckInline constexpr auto GetIterPartEnd() const { return m_pPart + m_pHdr->cPart; }
	EckInline constexpr auto GetIterStateEnd() const { return m_pState + m_pHdr->cState; }

	EckInline constexpr auto FindPart(Part ePart) const
	{
		const auto it = std::lower_bound(m_pPart, GetIterPartEnd(), ePart,
			[](const THEME_PART& e, Part ePart) { return e.ePart < ePart; });
		if (it != GetIterPartEnd() && it->ePart == ePart)
			return it;
		return GetIterPartEnd();
	}

	EckInline constexpr auto FindState(State eState, const THEME_PART& Part) const
	{
		const auto itEnd = m_pState + Part.idxState + Part.cState;
		const auto it = std::lower_bound(m_pState + Part.idxState,
			itEnd, eState,
			[](const THEME_STATE& e, State eState) { return e.eState < eState; });
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
		m_pPart = (THEME_PART*)(m_pofImage + m_pHdr->cImage);
		m_pState = (THEME_STATE*)(m_pPart + m_pHdr->cPart);
		m_pSeq = (THEME_SEQ*)(m_pState + m_pHdr->cState);
		CMemReader w{ m_pSeq + m_pHdr->cSeq };
		UINT c;
		EckCounter(m_pHdr->cPalette, i)
		{
			w >> c;
			const auto pPal = new CThemePalette{ (const D2D1_COLOR_F*)w.Data(),c };
			w += (sizeof(D2D1_COLOR_F) * c);
			m_vPalette.emplace_back(pPal);
		}
		if (m_pHdr->cPalette)
		{
			m_pPalette = m_vPalette.front();
			m_pPalette->AddRef();
		}
		m_pSysClr = (m_pHdr->onSysClr < 0 ? (const D2D1_COLOR_F*)w.Data() : nullptr);
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CTheme);
	virtual ~CTheme()
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
		SafeRelease(m_pPalette);
		for (auto p : m_vAtlas)
			SafeRelease(p);
		for (auto p : m_vImage)
			SafeRelease(p);
		for (auto p : m_vPalette)
			SafeRelease(p);
		m_vAtlas.clear();
		m_vImage.clear();
		m_vPalette.clear();
		m_bNeedFreeData = FALSE;
		m_pHdr = nullptr;
		m_pPart = nullptr;
		m_pState = nullptr;
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

	EckInline constexpr HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) const
	{
		if (m_pSysClr)
		{
			cr = m_pSysClr[(size_t)eSysColor];
			return S_OK;
		}
		else if (m_pPalette)
		{
			cr = m_pPalette->GetColor((size_t)eSysColor + m_pHdr->onSysClr);
			return S_OK;
		}
		else
			return E_UNEXPECTED;
	}

	EckInline constexpr float GetMetrics(Metrics eMetric) const
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

	EckInline void SetPalette(CThemePalette* pPal)
	{
		std::swap(m_pPalette, pPal);
		if (m_pPalette)
			m_pPalette->AddRef();
		if (pPal)
			pPal->Release();
	}

	EckInline constexpr auto GetPalette() const { return m_pPalette; }

	EckInline void AddPalette(CThemePalette* pPal)
	{
		m_vPalette.emplace_back(pPal)->AddRef();
	}

	EckInline constexpr auto& GetPaletteList() const { return m_vPalette; }
};

// 基于指定DC的主题实现
class CThemeRealization : public CUnknown<CThemeRealization, ITheme>
{
protected:
	ID2D1DeviceContext* m_pDC;	// 关联DC
	CTheme* m_pTheme;			// 关联主题

	std::vector<ID2D1Bitmap1*> m_vAtlas;// 图集实现
	std::vector<ID2D1Bitmap1*> m_vImage;// 单个图片实现

	ID2D1SolidColorBrush* m_pBrush{};	// 通用画刷

	D2D1_COLOR_F m_crColorization{};


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

	EckInline constexpr const auto& GetBkColor(const THEME_STATE& State, const DTB_OPT* pOpt)
	{
		return (pOpt->uFlags & DTBO_NEW_COLOR) ? pOpt->CustomColor.crBk :
			State.bNoUsePalette ? State.Color.crBk :
			State.ColorIdx.idxClrBk == IdxColorizationColor ? m_crColorization :
			m_pTheme->m_pPalette->GetColor(State.ColorIdx.idxClrBk);
	}

	EckInline constexpr const auto& GetBorderColor(const THEME_STATE& State, const DTB_OPT* pOpt)
	{
		return (pOpt->uFlags & DTBO_NEW_COLOR) ? pOpt->CustomColor.crBorder :
			State.bNoUsePalette ? State.Color.crBorder :
			State.ColorIdx.idxClrBorder == IdxColorizationColor ? m_crColorization :
			m_pTheme->m_pPalette->GetColor(State.ColorIdx.idxClrBorder);
	}

	EckInline constexpr const auto& GetExtra1Color(const THEME_STATE& State, const DTB_OPT* pOpt)
	{
		return (pOpt->uFlags & DTBO_NEW_COLOR) ? pOpt->CustomColor.crExtra1 :
			State.bNoUsePalette ? State.Color.crExtra1 :
			State.ColorIdx.idxClrExtra1 == IdxColorizationColor ? m_crColorization :
			m_pTheme->m_pPalette->GetColor(State.ColorIdx.idxClrExtra1);
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

	virtual ~CThemeRealization()
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
	HRESULT DrawBackground(Part ePart, State eState,
		const D2D1_RECT_F& rc, _In_opt_ const DTB_OPT* pOpt = nullptr) override
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
			float fWidth{};
			ID2D1StrokeStyle* pStrokeStyle{};

			if (pOpt->uFlags & DTBO_NEW_OPACITY)
				m_pBrush->SetOpacity(pOpt->fOpacity);

			if (GeoIsFill(itState->Geo.eType))
				m_pBrush->SetColor(GetBkColor(*itState, pOpt));
			else
			{
				if (GeoIsFillFrame(itState->Geo.eType))
					m_pBrush->SetColor(GetBkColor(*itState, pOpt));
				else
					m_pBrush->SetColor(GetBorderColor(*itState, pOpt));

				fWidth = pOpt->uFlags & DTBO_NEW_STROKE_WIDTH ?
					pOpt->fStrokeWidth : itState->Geo.fWidth;
				pStrokeStyle = pOpt->uFlags & DTBO_NEW_STROKE_STYLE ?
					pOpt->pStrokeStyle : nullptr;
			}

			switch (itState->Geo.eType)
			{
			case GeoType::FillRect:
			case GeoType::FrameRect:
			case GeoType::FillFrameRect:
				if (itState->bNoStretch)
				{
					Geo.rc = Param.rc;
					CenterRect(Geo.rc, rc);
				}
				else
					Geo.rc = rc;
				break;
			case GeoType::FillRoundRect:
			case GeoType::FrameRoundRect:
			case GeoType::FillFrameRoundRect:
				if (pOpt->uFlags & DTBO_NEW_RADX)
					Geo.rrc.radiusX = pOpt->fRadX;
				else
					Geo.rrc.radiusX = Param.rrc.radiusX;
				if (pOpt->uFlags & DTBO_NEW_RADY)
					Geo.rrc.radiusY = pOpt->fRadY;
				else
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
			case GeoType::Ring:
			case GeoType::FrameRing:
				Geo.ring.ellOutter.point.x = (rc.left + rc.right) / 2.f;
				Geo.ring.ellOutter.point.y = (rc.top + rc.bottom) / 2.f;
				Geo.ring.ellInner.point.x = (rc.left + rc.right) / 2.f;
				Geo.ring.ellInner.point.y = (rc.top + rc.bottom) / 2.f;
				if (itState->bNoStretch)
				{
					Geo.ring.ellOutter.radiusX = Param.ring.ellOutter.radiusX;
					Geo.ring.ellOutter.radiusY = Param.ring.ellOutter.radiusY;
					Geo.ring.ellInner.radiusX = Param.ring.ellInner.radiusX;
					Geo.ring.ellInner.radiusY = Param.ring.ellInner.radiusY;
				}
				else
				{
					Geo.ring.ellOutter.radiusX = (rc.right - rc.left) / 2.f;
					Geo.ring.ellOutter.radiusY = (rc.bottom - rc.top) / 2.f;
					Geo.ring.ellInner.radiusX = Geo.ring.ellOutter.radiusX *
						Param.ring.ellInner.radiusX / Param.ring.ellOutter.radiusX;
					Geo.ring.ellInner.radiusY = Geo.ring.ellOutter.radiusY * 
						Param.ring.ellInner.radiusY / Param.ring.ellOutter.radiusY;
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
				InflateRect(Geo.rc, -fWidth / 2.f, -fWidth / 2.f);
				m_pDC->DrawRectangle(Geo.rc, m_pBrush, fWidth, pStrokeStyle);			break;
			case GeoType::FrameRoundRect:
				InflateRect(Geo.rrc.rect, -fWidth / 2.f, -fWidth / 2.f);
				m_pDC->DrawRoundedRectangle(Geo.rrc, m_pBrush, fWidth, pStrokeStyle);	break;
			case GeoType::FrameEllipse:
				Geo.ell.radiusX -= fWidth / 2.f;
				Geo.ell.radiusY -= fWidth / 2.f;
				m_pDC->DrawEllipse(Geo.ell, m_pBrush, fWidth, pStrokeStyle);			break;

			case GeoType::FillFrameRect:
				m_pDC->FillRectangle(Geo.rc, m_pBrush);
				m_pBrush->SetColor(GetBorderColor(*itState, pOpt));
				InflateRect(Geo.rc, -fWidth / 2.f, -fWidth / 2.f);
				m_pDC->DrawRectangle(Geo.rc, m_pBrush, fWidth, pStrokeStyle);
				break;
			case GeoType::FillFrameRoundRect:
				m_pDC->FillRoundedRectangle(Geo.rrc, m_pBrush);
				m_pBrush->SetColor(GetBorderColor(*itState, pOpt));
				InflateRect(Geo.rrc.rect, -fWidth / 2.f, -fWidth / 2.f);
				m_pDC->DrawRoundedRectangle(Geo.rrc, m_pBrush, fWidth, pStrokeStyle);
				break;
			case GeoType::FillFrameEllipse:
				m_pDC->FillEllipse(Geo.ell, m_pBrush);
				m_pBrush->SetColor(GetBorderColor(*itState, pOpt));
				Geo.ell.radiusX -= fWidth / 2.f;
				Geo.ell.radiusY -= fWidth / 2.f;
				m_pDC->DrawEllipse(Geo.ell, m_pBrush, fWidth, pStrokeStyle);
				break;

			case GeoType::Line:
				m_pDC->DrawLine(Geo.pt[0], Geo.pt[1], m_pBrush, fWidth, pStrokeStyle);
				break;

			case GeoType::Ring:
				m_pDC->FillEllipse(Geo.ring.ellOutter, m_pBrush);
				m_pBrush->SetColor(GetExtra1Color(*itState, pOpt));
				m_pDC->FillEllipse(Geo.ring.ellInner, m_pBrush);
				break;
			case GeoType::FrameRing:
				m_pDC->FillEllipse(Geo.ring.ellOutter, m_pBrush);
				m_pBrush->SetColor(GetExtra1Color(*itState, pOpt));
				m_pDC->FillEllipse(Geo.ring.ellInner, m_pBrush);
				Geo.ring.ellOutter.radiusX -= fWidth / 2.f;
				Geo.ring.ellOutter.radiusY -= fWidth / 2.f;
				m_pBrush->SetColor(GetBorderColor(*itState, pOpt));
				m_pDC->DrawEllipse(Geo.ring.ellOutter, m_pBrush, fWidth, pStrokeStyle);
				break;
			default: ECK_UNREACHABLE;
			}

			if (pOpt->uFlags & DTBO_NEW_OPACITY)
				m_pBrush->SetOpacity(1.f);
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

	HRESULT SetColorizationColor(const D2D1_COLOR_F& cr) override
	{
		m_crColorization = cr;
		return S_OK;
	}

	HRESULT GetColorizationColor(_Out_ D2D1_COLOR_F& cr) override
	{
		cr = m_crColorization;
		return S_OK;
	}

	HRESULT GetColor(Part ePart, State eState,
		ClrPart eClrPart, _Out_ D2D1_COLOR_F& cr) override
	{
		return m_pTheme->GetColor(ePart, eState, eClrPart, cr);
	}

	HRESULT GetSysColor(SysColor eSysColor, _Out_ D2D1_COLOR_F& cr) override
	{
		return m_pTheme->GetSysColor(eSysColor, cr);
	}

	float GetMetrics(Metrics eMetrics) override
	{
		return m_pTheme->GetMetrics(eMetrics);
	}

	EckInlineNdCe auto GetTheme() const { return m_pTheme; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END