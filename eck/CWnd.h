#pragma once
#include "CObject.h"
#include "WndHelper.h"
#include "CMemWalker.h"
#include "ILayout.h"
#include "CSignal.h"

ECK_NAMESPACE_BEGIN
enum class FrameType
{
	None,	// 无边框
	Sunken,	// 凹入式
	Raised,	// 凸出式
	Flat,	// 浅凹入式
	Box,	// 镜框式
	Single	// 单线边框式
};

enum class ScrollType
{
	None,	// 无
	Horz,	// 水平滚动条
	Vert,	// 垂直滚动条
	Both	// 水平和垂直滚动条
};


constexpr inline int CDV_WND_1 = 0;

// CTRLDATA_WND::uFlags
enum : UINT
{
	SERDF_SBH = 1u << 0,
	SERDF_SBV = 1u << 1,
	SERDF_IMAGELIST = 1u << 2,
	SERDF_X = 1u << 3,
	SERDF_Y = 1u << 4,
	SERDF_CX = 1u << 5,
	SERDF_CY = 1u << 6,
	SERDF_POSSIZE = SERDF_X | SERDF_Y | SERDF_CX | SERDF_CY,
};

#pragma pack(push, ECK_CTRLDATA_ALIGN)
struct CTRLDATA_WND
{
	UINT uFlags;
	int iVer;
	int cchText;
	DWORD dwStyle;
	DWORD dwExStyle;
	// WCHAR szText[];
	// SCROLLINFO siHorz;// SERDF_SBH设置时
	// SCROLLINFO siVert;// SERDF_SBV设置时
	// RCWH rcPos;// SERDF_X、SERDF_Y、SERDF_CX、SERDF_CY中的一个被设置时才附加此结构

	constexpr PCWSTR Text() const
	{
		if (cchText)
			return (PCWSTR)(this + 1);
		else
			return nullptr;
	}

	constexpr size_t Size() const
	{
		return sizeof(CTRLDATA_WND) + (cchText + 1) * sizeof(WCHAR) +
			((uFlags & SERDF_SBH) ? sizeof(SCROLLINFO) : 0) +
			((uFlags & SERDF_SBV) ? sizeof(SCROLLINFO) : 0) +
			((uFlags & SERDF_POSSIZE) ? sizeof(RCWH) : 0);
	}

	constexpr RCWH* PosSize() const
	{
		if (!(uFlags & SERDF_POSSIZE))
			return nullptr;
		return (RCWH*)((PCBYTE)this + Size() - sizeof(RCWH));
	}

	constexpr SCROLLINFO* ScrollInfoHorz() const
	{
		if (!(uFlags & SERDF_SBH))
			return nullptr;
		return (SCROLLINFO*)((PCBYTE)this + sizeof(CTRLDATA_WND) + (cchText + 1) * sizeof(WCHAR));
	}

	constexpr SCROLLINFO* ScrollInfoVert() const
	{
		if (!(uFlags & SERDF_SBV))
			return nullptr;
		return (SCROLLINFO*)((PCBYTE)this + sizeof(CTRLDATA_WND) + (cchText + 1) * sizeof(WCHAR) +
			((uFlags & SERDF_SBH) ? sizeof(SCROLLINFO) : 0));
	}
};
#pragma pack(pop)

// SERIALIZE_OPT::uFlags
enum : UINT
{
	SERF_NO_COMBO_ITEM = 1u << 0,		// 一般供ComboBoxEx使用，指示CComboBox不要序列化项目数据
	SERF_EXCLUDE_IMAGELIST = 1u << 1,	// 不要序列化图像列表数据
};

struct SERIALIZE_OPT
{
	UINT uFlags;
	int cchTextBuf;
	void* pUserData;
};

#ifdef ECK_CTRL_DESIGN_INTERFACE
struct DESIGNDATA_WND
{
	CRefStrW rsName;
	BITBOOL bVisible : 1;
	BITBOOL bEnable : 1;
};
#endif

// 生成以ID创建的方法
#define ECK_CWND_CREATE											\
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,	\
		int x, int y, int cx, int cy, HWND hParent, int nID, ::eck::PCVOID pData = nullptr)	\
	{															\
		return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,\
			hParent, ::eck::i32ToP<HMENU>(nID), pData);			\
	}

// 按类名和实例句柄生成创建方法
#define ECK_CWND_CREATE_CLS_HINST(ClsName, HInst)	\
	ECK_CWND_CREATE						\
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle, int x, int y,				\
		int cx, int cy, HWND hParent, HMENU hMenu, ::eck::PCVOID pData = nullptr) override	\
	{									\
		if (pData)						\
		{								\
			const auto* const pBase = (::eck::CTRLDATA_WND*)pData;							\
			PreDeserialize(pData);		\
			IntCreate(pBase->dwExStyle, ClsName, pBase->Text(), pBase->dwStyle,				\
				x, y, cx, cy, hParent, hMenu, HInst, nullptr);	\
			PostDeserialize(pData);		\
		}								\
		else							\
		{								\
			IntCreate(dwExStyle, ClsName, pszText, dwStyle,		\
				x, y, cx, cy, hParent, hMenu, HInst, nullptr);	\
		}								\
		return m_hWnd;					\
	}

// 按类名生成创建方法
#define ECK_CWND_CREATE_CLS(ClsName) ECK_CWND_CREATE_CLS_HINST(ClsName, nullptr)

#define ECK_CWND_DISABLE_ATTACH			\
	void Attach(HWND hWnd) override		\
	{									\
		EckDbgPrintWithPos(L"** WARNING ** CWnd::Attach is disabled."); \
		abort();						\
	}									\
	HWND Detach() override				\
	{									\
		EckDbgPrintWithPos(L"** WARNING ** CWnd::Detach is disabled."); \
		abort();						\
		return nullptr;					\
	}

#define ECK_CWND_DISABLE_ATTACHNEW		\
	void AttachNew(HWND hWnd) override	\
	{									\
		EckDbgPrintWithPos(L"** WARNING ** CWnd::AttachNew is disabled."); \
		abort();						\
	}									\
	void DetachNew() override			\
	{									\
		EckDbgPrintWithPos(L"** WARNING ** CWnd::DetachNew is disabled."); \
		abort();						\
	}

#define ECK_CWND_SINGLEOWNER(Class)		\
	Class() = default;					\
	ECK_CWND_DISABLE_ATTACH				\
	ECK_CWND_DISABLE_ATTACHNEW

#define ECK_CWND_SINGLEOWNER_NO_DEF_CONS(Class)		\
	ECK_CWND_DISABLE_ATTACH							\
	ECK_CWND_DISABLE_ATTACHNEW

#define ECK_CWND_NOSINGLEOWNER(Class)	\
	Class() = default;					\
	Class(HWND hWnd) { m_hWnd = hWnd; }

class CWnd : public ILayout
{
	friend HHOOK BeginCbtHook(CWnd*, FWndCreating);
public:
	ECK_RTTI(CWnd);

#ifdef ECK_CTRL_DESIGN_INTERFACE
	DESIGNDATA_WND m_DDBase{};
#endif

#ifdef _DEBUG
	CRefStrW DbgTag{};
#endif
protected:
	HWND m_hWnd{};
	WNDPROC m_pfnRealProc{ DefWindowProcW };
	CSignal<Intercept_T, LRESULT, HWND, UINT, WPARAM, LPARAM> m_Sig{};
public:
	using HSlot = decltype(m_Sig)::HSlot;
protected:

	EckInline HWND IntCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		FWndCreating pfnCreatingProc = nullptr)
	{
		BeginCbtHook(this, pfnCreatingProc);
#ifdef _DEBUG
		CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
		if (!m_hWnd)
		{
			EckDbgPrintFormatMessage(GetLastError());
			EckDbgBreak();
		}
		if (IsWindow(m_hWnd))
			return m_hWnd;
		else
			return m_hWnd = nullptr;
#else
		return CreateWindowExW(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam);
#endif // _DEBUG
	}

	template<class T>
	EckInline void FillNmhdr(T& nm, UINT uCode) const
	{
		static_assert(sizeof(T) >= sizeof(NMHDR));
		auto p = (NMHDR*)&nm;
		p->hwndFrom = GetHWND();
		p->code = uCode;
		p->idFrom = GetDlgCtrlID(GetHWND());
	}

	template<class T>
	EckInline LRESULT SendNotify(T& nm, HWND hParent) const
	{
		return SendMessageW(hParent, WM_NOTIFY, ((NMHDR*)&nm)->idFrom, (LPARAM)&nm);
	}

	template<class T>
	EckInline LRESULT SendNotify(T& nm) const
	{
		return SendNotify(nm, GetParent(GetHWND()));
	}

	template<class T>
	EckInline LRESULT FillNmhdrAndSendNotify(T& nm, HWND hParent, UINT uCode) const
	{
		FillNmhdr(nm, uCode);
		return SendNotify(nm, hParent);
	}

	template<class T>
	EckInline LRESULT FillNmhdrAndSendNotify(T& nm, UINT uCode) const
	{
		FillNmhdr(nm, uCode);
		return SendNotify(nm);
	}

	LRESULT DefNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CWndFromHWND(hParent)->OnMsg(hParent, uMsg, wParam, lParam);
	}

	EckInline LRESULT CallMsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		SlotCtx Ctx{};
		const auto r = m_Sig.Emit2(Ctx, hWnd, uMsg, wParam, lParam);
		if (Ctx.IsProcessed())
			return r;
		return OnMsg(hWnd, uMsg, wParam, lParam);
	}
public:
	ECKPROP_R(GetHWND)					HWND		HWnd;			// 窗口句柄
	ECKPROP(GetFont, SetFont)			HFONT		HFont;			// 字体句柄
	ECKPROP(GetStyle, SetStyle)			DWORD		Style;			// 窗口样式
	ECKPROP(GetExStyle, SetExStyle)		DWORD		ExStyle;		// 扩展窗口样式
	ECKPROP(GetText, SetText)			CRefStrW	Text;			// 标题
	ECKPROP(GetFrameType, SetFrameType) enum class FrameType FrameType;		// 边框类型
	ECKPROP(GetScrollBar, SetScrollBar) ScrollType	ScrollBarType;	// 滚动条类型
	ECKPROP(IsVisible, SetVisibility)	BOOL		Visible;		// 可视
	ECKPROP(IsEnabled, Enable)			BOOL		Enabled;		// 可用（未禁止）
	ECKPROP_W(SetRedraw)				BOOL		Redrawable;		// 可重画
	ECKPROP(GetLeft, SetLeft)			int			Left;			// 左边，相对父窗口
	ECKPROP(GetTop, SetTop)				int			Top;			// 顶边，相对父窗口
	ECKPROP(GetWidth, SetWidth)			int			Width;			// 宽度
	ECKPROP(GetHeight, SetHeight)		int			Height;			// 高度
	ECKPROP(GetPosition, SetPosition)	POINT		Position;		// 位置
	ECKPROP(GetSize, SetSize)			SIZE		Size;			// 尺寸
	ECKPROP_R(GetClientWidth)			int			ClientWidth;	// 客户区宽度
	ECKPROP_R(GetClientHeight)			int			ClientHeight;	// 客户区高度


	static LRESULT CALLBACK EckWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		const auto pCtx = GetThreadCtx();
		const auto p = pCtx->WmAt(hWnd);
		EckAssert(p);

		CWnd* pChild;
		BOOL bProcessed = FALSE;
		switch (uMsg)
		{
		case WM_NOTIFY:
			if (pChild = pCtx->WmAt(((NMHDR*)lParam)->hwndFrom))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_HSCROLL:
		case WM_VSCROLL:
		case WM_COMMAND:
		case WM_CHARTOITEM:
		case WM_VKEYTOITEM:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			if (pChild = pCtx->WmAt((HWND)lParam))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_DRAWITEM:
			if (pChild = pCtx->WmAt(((DRAWITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_MEASUREITEM:
			if (pChild = pCtx->WmAt(GetDlgItem(hWnd, ((MEASUREITEMSTRUCT*)lParam)->CtlID)))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_DELETEITEM:
			if (pChild = pCtx->WmAt(((DELETEITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_COMPAREITEM:
			if (pChild = pCtx->WmAt(((COMPAREITEMSTRUCT*)lParam)->hwndItem))
			{
				const auto lResult = pChild->OnNotifyMsg(hWnd, uMsg, wParam, lParam, bProcessed);
				if (bProcessed)
					return lResult;
			}
			break;
		case WM_STYLECHANGED:
		{
			if (wParam == GWL_STYLE)
			{
				const auto* const pss = (STYLESTRUCT*)lParam;
				if (!IsBitSet(pss->styleNew, WS_CHILD) && IsBitSet(pss->styleOld, WS_CHILD))
				{
					const auto pCtx = GetThreadCtx();
					EckAssert(!pCtx->TwmAt(hWnd));
					pCtx->TwmAdd(hWnd, p);
				}

				if (IsBitSet(pss->styleNew, WS_CHILD) && !IsBitSet(pss->styleOld, WS_CHILD))
				{
					const auto pCtx = GetThreadCtx();
					EckAssert(pCtx->TwmAt(hWnd));
					pCtx->TwmRemove(hWnd);
				}
			}
		}
		break;
		case WM_CREATE:
		{
			if (pCtx->bAutoNcDark && pCtx->TwmAt(hWnd) == p)
				EnableWindowNcDarkMode(hWnd, ShouldAppsUseDarkMode());
		}
		break;
		case WM_NCDESTROY:// 窗口生命周期中的最后一个消息，在这里解绑HWND和CWnd，从窗口映射中清除无效内容
		{
			const auto lResult = p->CallMsgProc(hWnd, uMsg, wParam, lParam);
			(void)p->CWnd::Detach();// 控件类可能不允许拆离，必须使用基类拆离
			return lResult;
		}
		}
		return p->CallMsgProc(hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 是否可更改句柄所有权
	/// </summary>
	/// <returns>若类与HWND没有强联系则返回FALSE，否则返回TRUE，此时不能执行依附拆离等操作</returns>
	[[nodiscard]] EckInline static constexpr BOOL IsSingleOwner() { return FALSE; }

	CWnd() = default;

	/// <summary>
	/// 构造自句柄。
	/// 不插入窗口映射，仅用于临时使用
	/// </summary>
	/// <param name="hWnd"></param>
	CWnd(HWND hWnd) :m_hWnd{ hWnd } {}

	ECK_DISABLE_COPY_MOVE(CWnd);

	virtual ~CWnd()
	{
#ifdef _DEBUG
		// 对于已添加进映射的窗口，CWnd的生命周期必须在窗口生命周期之内
		if (m_hWnd)
			EckAssert(((GetThreadCtx()->WmAt(m_hWnd) == this) ? (!m_hWnd) : TRUE));
#endif // _DEBUG
	}

	// 依附句柄。函数将新句柄插入窗口映射，调用此函数必须满足两个前提：本类不能持有句柄；新句柄未在窗口映射中
	virtual void Attach(HWND hWnd)
	{
		EckAssert(!m_hWnd);// 当前类必须未持有句柄
		const auto pCtx = GetThreadCtx();
		EckAssert(!pCtx->WmAt(hWnd));// 新句柄必须未被CWnd持有
		m_hWnd = hWnd;
		pCtx->WmAdd(hWnd, this);
		if (!IsBitSet(GetStyle(), WS_CHILD))
			pCtx->TwmAdd(hWnd, this);
	}

	// 拆离句柄。 函数将从窗口映射中移除句柄
	[[nodiscard]] virtual HWND Detach()
	{
		HWND hWnd = nullptr;
		std::swap(hWnd, m_hWnd);
		const auto pCtx = GetThreadCtx();
		EckAssert(pCtx->WmAt(hWnd) == this);// 检查匹配性
		pCtx->WmRemove(hWnd);
		pCtx->TwmRemove(hWnd);
		return hWnd;
	}

	// 依附句柄，并同步状态
	EckInline virtual void AttachNew(HWND hWnd)
	{
		CWnd::Attach(hWnd);
		m_pfnRealProc = SetWindowProc(hWnd, EckWndProc);
	}

	// 拆离句柄，并重置状态
	EckInline virtual void DetachNew()
	{
		SetWindowProc(Detach(), m_pfnRealProc);
	}

	/// <summary>
	/// 创建窗口
	/// </summary>
	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = nullptr)
	{
		return Create(pszText, dwStyle, dwExStyle, x, y, cx, cy,
			hParent, i32ToP<HMENU>(nID), pData);
	}

	EckInline HWND NativeCreate(DWORD dwExStyle, PCWSTR pszClass, PCWSTR pszText, DWORD dwStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, HINSTANCE hInst, void* pParam,
		FWndCreating pfnCreatingProc = nullptr)
	{
		return IntCreate(dwExStyle, pszClass, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, hInst, pParam, pfnCreatingProc);
	}

	/// <summary>
	/// 创建窗口。
	/// 不能调用基类的此方法
	/// </summary>
	virtual HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr)
	{
		EckDbgPrintWithPos(L"** ERROR ** CWnd::Create未实现");
		EckDbgBreak();
		abort();
	}

	/// <summary>
	/// 序列化数据。
	/// 子类若要存储额外数据，一般情况下应首先调用基类的此方法，然后再序列化自己的数据
	/// </summary>
	/// <param name="rb">字节集</param>
	/// <param name="pOpt">可选的序列化选项</param>
	virtual void SerializeData(CRefBin& rb, const SERIALIZE_OPT* pOpt = nullptr)
	{
		CRefStrW rsText = GetText();
		const auto dwStyle = GetStyle();

		const SIZE_T cbSize = sizeof(CTRLDATA_WND) + rsText.ByteSize() +
			(IsBitSet(dwStyle, WS_HSCROLL) ? sizeof(SCROLLINFO) : 0) +
			(IsBitSet(dwStyle, WS_VSCROLL) ? sizeof(SCROLLINFO) : 0);

		CMemWriter w(rb.PushBack(cbSize), cbSize);
		CTRLDATA_WND* p;
		w.SkipPointer(p);
		p->uFlags = 0u;
		p->iVer = CDV_WND_1;
		p->cchText = rsText.Size();
		p->dwStyle = dwStyle;
		p->dwExStyle = GetExStyle();
		w << rsText;
		SCROLLINFO* psi;
		if (IsBitSet(dwStyle, WS_HSCROLL))
		{
			p->uFlags |= SERDF_SBH;
			w.SkipPointer(psi);
			GetSbInfo(SB_HORZ, psi);
		}
		if (IsBitSet(dwStyle, WS_VSCROLL))
		{
			p->uFlags |= SERDF_SBV;
			w.SkipPointer(psi);
			GetSbInfo(SB_VERT, psi);
		}
	}

	virtual void PreDeserialize(PCVOID pData) {}

	virtual void PostDeserialize(PCVOID pData)
	{
		const auto* const p = (const CTRLDATA_WND*)pData;
		const auto* psi = (const SCROLLINFO*)SkipBaseData(pData);
		if (p->uFlags & SERDF_SBH)
		{
			SetSbInfo(SB_HORZ, psi);
			++psi;
		}
		if (p->uFlags & SERDF_SBV)
			SetSbInfo(SB_VERT, psi);
	}

	/// <summary>
	/// 消息处理函数
	/// </summary>
	EckInline virtual LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return CallWindowProcW(m_pfnRealProc, hWnd, uMsg, wParam, lParam);
	}

	/// <summary>
	/// 消息过滤器
	/// </summary>
	/// <param name="Msg">MSG结构</param>
	/// <returns>若要禁止派发该消息则返回TRUE，否则返回FALSE</returns>
	EckInline virtual BOOL PreTranslateMessage(const MSG& Msg)
	{
		return FALSE;
	}

	/// <summary>
	/// 父窗口通知类消息映射。
	/// 父窗口接收到的通知消息将路由到本方法，一般情况下无需手动调用本方法。
	/// 路由的消息有以下四种：所有者项目系列（WM_XxxITEM）、
	/// 标准通知系列（WM_COMMAND、WM_NOTIFY）、着色系列（WM_CTLCOLORXxx）、
	/// 滚动条系列（WM_VSCROLL、WM_HSCROLL）
	/// </summary>
	/// <param name="hParent">父窗口句柄</param>
	/// <param name="uMsg">消息</param>
	/// <param name="wParam">wParam</param>
	/// <param name="lParam">lParam</param>
	/// <param name="bProcessed">若设为TRUE，则不再将当前消息交由父窗口处理，调用函数前保证其为FALSE</param>
	/// <returns>消息返回值</returns>
	EckInline virtual LRESULT OnNotifyMsg(HWND hParent, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
	{
		return 0;
	}

	void LoGetAppropriateSize(int& cx, int& cy) override
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		cx = rc.right - rc.left;
		cy = rc.bottom - rc.top;
	}

	void LoSetPos(int x, int y) override
	{
		SetWindowPos(m_hWnd, nullptr, x, y, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	void LoSetSize(int cx, int cy) override
	{
		SetWindowPos(m_hWnd, nullptr, 0, 0, cx, cy,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	void LoSetPosSize(int x, int y, int cx, int cy)  override
	{
		SetWindowPos(m_hWnd, nullptr, x, y, cx, cy,
			SWP_NOZORDER | SWP_NOACTIVATE);
	}

	std::pair<int, int> LoGetPos() override
	{
		const auto pt{ GetPosition() };
		return { pt.x,pt.y };
	}

	std::pair<int, int> LoGetSize() override
	{
		const auto size{ GetSize() };
		return { size.cx,size.cy };
	}

	void LoShow(BOOL bShow) override
	{
		ShowWindow(m_hWnd, bShow ? SW_SHOW : SW_HIDE);
	}

	HWND LoGetHWND() override
	{
		return GetHWND();
	}

	virtual BOOL IsNeedTheme() const { return FALSE; }

	// 跳到当前类序列化数据的尾部
	[[nodiscard]] EckInline constexpr static PCVOID SkipBaseData(PCVOID p)
	{
		return PtrStepCb(p, ((const CTRLDATA_WND*)p)->Size());
	}

	/// <summary>
	/// 重新创建窗口。
	/// 函数先序列化数据，然后销毁当前窗口，最后创建新窗口，期间将自动调整窗口映射
	/// </summary>
	/// <param name="dwNewStyle">可选的窗口样式，将覆盖序列化数据中的样式</param>
	/// <param name="dwNewExStyle">可选的扩展窗口样式，将覆盖序列化数据中的样式</param>
	/// <param name="rcPos">可选的位置（左顶宽高），将覆盖旧位置</param>
	/// <returns>窗口句柄</returns>
	HWND ReCreate(EckOptNul(DWORD, dwNewStyle), EckOptNul(DWORD, dwNewExStyle),
		EckOptNul(RECT, rcPos), const SERIALIZE_OPT* pSerOpt = nullptr)
	{
		CRefBin rb{};
		SerializeData(rb, pSerOpt);
		const HWND hParent = GetParent(m_hWnd);
		const int iID = GetDlgCtrlID(m_hWnd);
		const HFONT hFont = HFont;

		RCWH NewPos;
		if (!rcPos.has_value())
		{
			RECT rc;
			GetWindowRect(m_hWnd, &rc);
			ScreenToClient(hParent, &rc);
			NewPos = { rc.left,rc.top,rc.right - rc.left,rc.bottom - rc.top };
		}
		else
		{
			NewPos = { rcPos.value().left,rcPos.value().top,
				rcPos.value().right - rcPos.value().left,
				rcPos.value().bottom - rcPos.value().top };
		}

		const auto pData = (CTRLDATA_WND*)rb.Data();
		if (dwNewStyle.has_value())
			pData->dwStyle = dwNewStyle.value();
		if (dwNewExStyle.has_value())
			pData->dwExStyle = dwNewExStyle.value();
		if (pData->uFlags & SERDF_X)
			NewPos.x = pData->PosSize()->x;
		if (pData->uFlags & SERDF_Y)
			NewPos.y = pData->PosSize()->y;
		if (pData->uFlags & SERDF_CX)
			NewPos.cx = pData->PosSize()->cx;
		if (pData->uFlags & SERDF_CY)
			NewPos.cy = pData->PosSize()->cy;

		Destroy();
		Create(nullptr, 0, 0, NewPos.x, NewPos.y,
			NewPos.cx, NewPos.cy, hParent, iID, rb.Data());
		if (pData->uFlags & SERDF_SBH)
			SetSbInfo(SB_HORZ, pData->ScrollInfoHorz());
		if (pData->uFlags & SERDF_SBV)
			SetSbInfo(SB_VERT, pData->ScrollInfoVert());
		HFont = hFont;
		return m_hWnd;
	}

	[[nodiscard]] EckInline constexpr HWND GetHWND() const { return m_hWnd; }

	// 强制重新核算非客户区
	EckInline void FrameChanged() const
	{
		SetWindowPos(m_hWnd, nullptr, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	EckInline void SetRedraw(BOOL bRedraw) const
	{
		SendMsg(WM_SETREDRAW, bRedraw, 0);
	}

	EckInline BOOL Redraw(BOOL bErase = FALSE) const
	{
		return InvalidateRect(m_hWnd, nullptr, bErase);
	}

	EckInline BOOL Redraw(const RECT& rc, BOOL bErase = FALSE) const
	{
		return InvalidateRect(m_hWnd, &rc, bErase);
	}

	void SetFrameType(enum class FrameType eType) const
	{
		DWORD dwStyle = GetStyle() & ~WS_BORDER;
		DWORD dwExStyle = GetExStyle()
			& ~(WS_EX_WINDOWEDGE | WS_EX_STATICEDGE | WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE);

		switch (eType)
		{
		case FrameType::Sunken: dwExStyle |= WS_EX_CLIENTEDGE; break;
		case FrameType::Raised: dwExStyle |= (WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME); break;
		case FrameType::Flat:	dwExStyle |= WS_EX_STATICEDGE; break;
		case FrameType::Box:	dwExStyle |= (WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE); break;
		case FrameType::Single: dwStyle |= WS_BORDER; break;
		}

		SetStyle(dwStyle);
		SetExStyle(dwExStyle);
	}

	// For compatibility.
	EckInline void SetFrameType(int iType) const { SetFrameType((enum class FrameType)iType); }

	[[nodiscard]] enum class FrameType GetFrameType() const
	{
		const DWORD dwStyle = GetStyle();
		const DWORD dwExStyle = GetExStyle();
		if (IsBitSet(dwExStyle, WS_EX_DLGMODALFRAME))
		{
			if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
				return FrameType::Box;
			if (IsBitSet(dwExStyle, WS_EX_WINDOWEDGE))
				return FrameType::Raised;
		}

		if (IsBitSet(dwExStyle, WS_EX_CLIENTEDGE))
			return FrameType::Sunken;
		if (IsBitSet(dwExStyle, WS_EX_STATICEDGE))
			return FrameType::Flat;
		if (IsBitSet(dwStyle, WS_BORDER))
			return FrameType::Single;

		return FrameType::None;
	}

	void SetScrollBar(ScrollType eType) const
	{
		switch (eType)
		{
		case ScrollType::None:
			ShowScrollBar(m_hWnd, SB_VERT, FALSE);
			ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
			break;
		case ScrollType::Horz:
			ShowScrollBar(m_hWnd, SB_VERT, FALSE);
			ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
			break;
		case ScrollType::Vert:
			ShowScrollBar(m_hWnd, SB_VERT, TRUE);
			ShowScrollBar(m_hWnd, SB_HORZ, FALSE);
			break;
		case ScrollType::Both:
			ShowScrollBar(m_hWnd, SB_VERT, TRUE);
			ShowScrollBar(m_hWnd, SB_HORZ, TRUE);
			break;
		}
	}

	// For compatibility.
	EckInline void SetScrollBar(int iType) const { SetScrollBar((ScrollType)iType); }

	[[nodiscard]] ScrollType GetScrollBar() const
	{
		const BOOL bVSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_VSCROLL);
		const BOOL bHSB = IsBitSet(GetWindowLongPtrW(m_hWnd, GWL_STYLE), WS_HSCROLL);
		if (bVSB)
			return bHSB ? ScrollType::Both : ScrollType::Vert;
		if (bHSB)
			return ScrollType::Horz;
		return ScrollType::None;
	}

	EckInline LRESULT SendMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	EckInline LRESULT PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) const
	{
		return PostMessageW(m_hWnd, uMsg, wParam, lParam);
	}

	[[nodiscard]] EckInline DWORD GetStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_STYLE);
	}

	[[nodiscard]] EckInline DWORD GetExStyle() const
	{
		return (DWORD)GetWindowLongPtrW(m_hWnd, GWL_EXSTYLE);
	}

	/// <summary>
	/// 修改样式
	/// </summary>
	/// <param name="dwNew">新样式</param>
	/// <param name="dwMask">掩码</param>
	/// <param name="idx">GWL_常量</param>
	/// <returns>旧样式</returns>
	EckInline DWORD ModifyStyle(DWORD dwNew, DWORD dwMask, int idx = GWL_STYLE) const
	{
		return ModifyWindowStyle(m_hWnd, dwNew, dwMask, idx);
	}

	EckInline DWORD SetStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_STYLE, dwStyle);
	}

	EckInline DWORD SetExStyle(DWORD dwStyle) const
	{
		return (DWORD)SetWindowLongPtrW(m_hWnd, GWL_EXSTYLE, dwStyle);
	}

	[[nodiscard]] EckInline CRefStrW GetText() const
	{
		CRefStrW rs;
		int cch = GetWindowTextLengthW(m_hWnd);
		if (cch)
		{
			rs.ReSize(cch);
			GetWindowTextW(m_hWnd, rs.Data(), cch + 1);
		}
		return rs;
	}

	/// <summary>
	/// 取标题
	/// </summary>
	/// <param name="pszBuf">缓冲区</param>
	/// <param name="cchBuf">缓冲区大小</param>
	/// <returns>复制的字符数</returns>
	EckInline int GetText(PWSTR pszBuf, int cchBuf) const
	{
		return GetWindowTextW(m_hWnd, pszBuf, cchBuf);
	}

	EckInline BOOL SetText(PCWSTR pszText) const
	{
		return SetWindowTextW(m_hWnd, pszText);
	}

	EckInline HRESULT SetExplorerTheme() const
	{
		return SetWindowTheme(m_hWnd, L"Explorer", nullptr);
	}

	EckInline HRESULT SetItemsViewTheme() const
	{
		return SetWindowTheme(m_hWnd, L"ItemsView", nullptr);
	}

	EckInline HRESULT SetTheme(PCWSTR pszAppName, PCWSTR pszSubList = nullptr) const
	{
		return SetWindowTheme(m_hWnd, pszAppName, pszSubList);
	}

	EckInline BOOL Move(int x, int y, int cx, int cy, BOOL bNoActive = FALSE) const
	{
		return SetWindowPos(m_hWnd, nullptr, x, y, cx, cy, SWP_NOZORDER | (bNoActive ? SWP_NOACTIVATE : 0));
	}

	EckInline BOOL Destroy()
	{
		EckAssert(IsWindow(m_hWnd));
		return DestroyWindow(m_hWnd);
	}

	EckInline void SetFont(HFONT hFont, BOOL bRedraw = FALSE) const
	{
		SendMsg(WM_SETFONT, (WPARAM)hFont, bRedraw);
	}

	[[nodiscard]] EckInline HFONT GetFont() const
	{
		return (HFONT)SendMsg(WM_GETFONT, 0, 0);
	}

	EckInline BOOL Show(int nCmdShow) const
	{
		return ShowWindow(m_hWnd, nCmdShow);
	}

	EckInline void SetVisibility(BOOL bVisible) const
	{
		Show(bVisible ? SW_SHOW : SW_HIDE);
	}

	EckInline BOOL Enable(BOOL bEnable) const
	{
		return EnableWindow(m_hWnd, bEnable);
	}

	[[nodiscard]] EckInline BOOL IsEnabled() const
	{
		return IsWindowEnabled(m_hWnd);
	}

	[[nodiscard]] EckInline BOOL IsVisible() const
	{
		return IsWindowVisible(m_hWnd);
	}

	[[nodiscard]] EckInline LONG_PTR GetLong(int i) const
	{
		return GetWindowLongPtrW(m_hWnd, i);
	}

	[[nodiscard]] EckInline LONG_PTR SetLong(int i, LONG_PTR l) const
	{
		return SetWindowLongPtrW(m_hWnd, i, l);
	}

	[[nodiscard]] EckInline CRefStrW GetClsName() const
	{
		CRefStrW rs(256);
		rs.ReSize(GetClassNameW(GetHWND(), rs.Data(), 256 + 1));
		return rs;
	}

	void SetLeft(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		SetWindowPos(m_hWnd, nullptr, i, rc.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetLeft() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return rc.left;
	}

	void SetTop(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		SetWindowPos(m_hWnd, nullptr, rc.left, i, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetTop() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return rc.top;
	}

	void SetWidth(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		SetWindowPos(m_hWnd, nullptr, 0, 0, i, rc.bottom - rc.top,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetWidth() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return rc.right - rc.left;
	}

	void SetHeight(int i) const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		SetWindowPos(m_hWnd, nullptr, 0, 0, rc.right - rc.left, i,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] int GetHeight() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return rc.bottom - rc.top;
	}

	EckInline void SetPosition(POINT pt) const
	{
		SetWindowPos(m_hWnd, nullptr, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
	}

	[[nodiscard]] EckInline POINT GetPosition() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		ScreenToClient(GetParent(m_hWnd), (POINT*)&rc);
		return { rc.left, rc.top };
	}

	EckInline void SetSize(SIZE sz) const
	{
		SetWindowPos(m_hWnd, nullptr, 0, 0, sz.cx, sz.cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	[[nodiscard]] EckInline SIZE GetSize() const
	{
		RECT rc;
		GetWindowRect(m_hWnd, &rc);
		return { rc.right - rc.left, rc.bottom - rc.top };
	}

	EckInline BOOL EnableArrows(int iOp, int iBarType)
	{
		EnableScrollBar(m_hWnd, iBarType, iOp);
	}

	EckInline int GetSbPos(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nPos;
	}

	EckInline int GetSbTrackPos(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nTrackPos;
	}

	EckInline BOOL GetSbRange(int iType, int* piMin = nullptr, int* piMax = nullptr) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		BOOL b = GetScrollInfo(m_hWnd, iType, &si);
		if (piMin)
			*piMin = si.nMin;
		if (piMax)
			*piMax = si.nMax;
		return b;
	}

	EckInline int GetSbPage(int iType) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		GetScrollInfo(m_hWnd, iType, &si);
		return si.nPage;
	}

	EckInline BOOL GetSbInfo(int iType, SCROLLINFO* psi) const
	{
		psi->cbSize = sizeof(SCROLLINFO);
		return GetScrollInfo(m_hWnd, iType, psi);
	}

	EckInline void SetSbPos(int iType, int iPos, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = iPos;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbRange(int iType, int iMin, int iMax, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
		si.nMin = iMin;
		si.nMax = iMax;
	}

	EckInline void SetSbMin(int iType, int iMin, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, iType, &si);
		si.nMin = iMin;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbMax(int iType, int iMax, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_RANGE;
		GetScrollInfo(m_hWnd, iType, &si);
		si.nMax = iMax;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbPage(int iType, int iPage, BOOL bRedraw = TRUE) const
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE;
		si.nPage = iPage;
		SetScrollInfo(m_hWnd, iType, &si, bRedraw);
	}

	EckInline void SetSbInfo(int iType, const SCROLLINFO* psi, BOOL bRedraw = TRUE) const
	{
		SetScrollInfo(m_hWnd, iType, psi, bRedraw);
	}

	EckInline [[nodiscard]] int GetClientWidth() const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return rc.right;
	}

	EckInline [[nodiscard]] int GetClientHeight() const
	{
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		return rc.bottom;
	}

	EckInline [[nodiscard]] BOOL IsValid() const
	{
#ifdef _DEBUG
		if (!IsWindow(m_hWnd))
			EckAssert(!m_hWnd);
#endif // _DEBUG
		return !!GetHWND();
	}

	EckInline WNDPROC SetWndProc(WNDPROC pfnWndProc)
	{
		std::swap(m_pfnRealProc, pfnWndProc);
		return pfnWndProc;
	}

	EckInline void SetTopMost(BOOL bTopMost) const
	{
		SetWindowPos(m_hWnd, bTopMost ? HWND_TOPMOST : HWND_NOTOPMOST,
			0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	EckInline constexpr [[nodiscard]] auto& GetSignal() { return m_Sig; }
};
ECK_RTTI_IMPL_INLINE(CWnd);

EckInline void AttachDlgItems(HWND hDlg, size_t cItem,
	_In_reads_(cItem) CWnd* const* pWnd, _In_reads_(cItem) const int* iId)
{
	EckCounter(cItem, i)
		pWnd[i]->AttachNew(GetDlgItem(hDlg, iId[i]));
}
ECK_NAMESPACE_END