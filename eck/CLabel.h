/*
* WinEzCtrlKit Library
*
* CLabel.h ： 标签
*
* Copyright(C) 2023 QingKong
*/
#pragma once
#include "CWnd.h"
#include "CSubclassMgr.h"
#include "Utility.h"

#include <windowsx.h>

ECK_NAMESPACE_BEGIN
struct ECKLABELDATA
{
	int iBKPicMode;			// 底图模式，0 - 居左上  1 - 平铺  2 - 居中  3 - 缩放
	int iAlignH;			// 横向对齐
	int iAlignV;			// 纵向对齐
	COLORREF crText;		// 文本颜色
	COLORREF crTextBK;		// 文本背景颜色
	COLORREF crBK;			// 背景颜色
	/*
	* 渐变背景模式，可选值：
	* 0 - 无  1 - 从上到下  2 - 从下到上  3 - 从左到右  4 - 从右到左
	* 5 - 从左上到右下  6 - 从右下到左上  7 - 从左下到右上  8 - 从右上到左下
	*/
	int iGradientMode;
	COLORREF crGradient[3];	// 渐变背景颜色
	int iEllipsisMode;		// 省略号模式，0 - 无  1 - 末尾省略  2 - 路径省略  3 - 省略单词
	int iPrefixMode;		// 前缀模式，0 - 常规  1 - 不解释前缀  2 - 隐藏下划线  3 - 只显示下划线
	int iMousePassingThrough;	// 鼠标穿透，0 - 无  1 - 穿透空白区域  2 - 穿透整个控件
	BITBOOL bAutoWrap : 1;		// 自动折行
	BITBOOL bFullWndPic : 1;	// 底图尽量充满控件
	BITBOOL bTransparent : 1;	// 透明标签
	BITBOOL bUxThemeText : 1;
};

class CLabel :public COwnWnd
{
private:
	WND_RECORDER_DECL(CLabel)

	ECKLABELDATA m_Info{};

	int m_cxClient = 0,
		m_cyClient = 0;			// 客户区大小
	HDC m_hCDC = NULL;			// 后台兼容DC
	HDC m_hcdcHelper = NULL;	// 画位图使用的辅助DC
	HBITMAP m_hBitmap = NULL;	// 后台兼容位图
	HGDIOBJ m_hOld1 = NULL;		// 后台DC旧位图句柄
	HGDIOBJ m_hOld2 = NULL;		// 辅助DC旧位图句柄

	HBITMAP m_hbmBK = NULL;		// 底图
	HBITMAP m_hbmPic = NULL;	// 图片
	int m_cxBKPic = 0,
		m_cyBKPic = 0;			// 底图大小

	int m_cxPic = 0,
		m_cyPic = 0;			// 图片大小
	RECT m_rcPartPic{};			// 缓存的图片矩形
	RECT m_rcPartText{};		// 缓存的文本矩形

	static ATOM m_atomLabel;	// 标签类原子

	/// <summary>
	/// 绘制标签。
	/// 若m_Info.bTransparent为TRUE，则自动设置剪辑DC至控件矩形
	/// </summary>
	/// <param name="hDC">目标DC，调用之前属性必须设置完毕</param>
	void Paint(HDC hDC);

	/// <summary>
	/// 计算部件矩形
	/// </summary>
	void CalcPartsRect();

	/// <summary>
	/// 置外部DC属性。
	/// 函数先使用SaveDC保存DC状态，然后根据控件属性设置DC
	/// </summary>
	/// <param name="hDC">目标DC</param>
	void SetDCAttr(HDC hDC);

	/// <summary>
	/// 控件窗口过程
	/// </summary>
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	static ATOM RegisterWndClass(HINSTANCE hInstance);

	EckInline HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL)
	{
		dwStyle |= WS_CHILD;
		m_hWnd = CreateWindowExW(dwExStyle, WCN_LABEL, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, this);
		m_Recorder.AddRecord(m_hWnd, this);
		SetClr(2, CLR_DEFAULT);
		return m_hWnd;
	}

	CLabel()
	{
		m_Info.iAlignV = 1;
		m_Info.crBK = CLR_DEFAULT;
		m_Info.crTextBK = CLR_DEFAULT;
		m_Info.crGradient[0] = 0x808080;
		m_Info.crGradient[1] = 0xFFFFFF;
		m_Info.crGradient[2] = 0x808080;
	}

	~CLabel()
	{
		DeleteObject(m_hbmBK);
	}

	void Redraw();

	/// <summary>
	/// 置背景图片
	/// </summary>
	/// <param name="hBitmap">位图句柄</param>
	HBITMAP SetBKPic(HBITMAP hBitmap);

	/// <summary>
	/// 取背景图片
	/// </summary>
	/// <returns>图片句柄</returns>
	EckInline HBITMAP GetBKPic()
	{
		return m_hbmBK;
	}

	/// <summary>
	/// 置图片
	/// </summary>
	/// <param name="hBitmap">位图句柄</param>
	HBITMAP SetPic(HBITMAP hBitmap);

	EckInline HBITMAP GetPic()
	{
		return m_hbmPic;
	}

	/// <summary>
	/// 置底图方式
	/// </summary>
	EckInline void SetBKPicMode(int iBKPicMode)
	{
		m_Info.iBKPicMode = iBKPicMode;
		Redraw();
	}

	EckInline int GetBKPicMode()
	{
		return m_Info.iBKPicMode;
	}

	/// <summary>
	/// 置对齐
	/// </summary>
	/// <param name="bHAlign">是否为横向对齐</param>
	/// <param name="iAlign">对齐方式</param>
	EckInline void SetAlign(BOOL bHAlign, int iAlign)
	{
		if (bHAlign)
			m_Info.iAlignH = iAlign;
		else
			m_Info.iAlignV = iAlign;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetAlign(BOOL bHAlign) const
	{
		if (bHAlign)
			return m_Info.iAlignH;
		else
			return m_Info.iAlignV;
	}

	/// <summary>
	/// 置自动换行
	/// </summary>
	/// <param name="bAutoWrap"></param>
	/// <returns></returns>
	EckInline void SetAutoWrap(BOOL bAutoWrap)
	{
		m_Info.bAutoWrap = bAutoWrap;
		CalcPartsRect();
		Redraw();
	}

	EckInline BOOL GetAutoWrap() const
	{
		return m_Info.bAutoWrap;
	}

	/// <summary>
	/// 置颜色
	/// </summary>
	/// <param name="idx">0 = 文本颜色  1 = 背景  2 = 文本背景</param>
	/// <param name="cr">颜色</param>
	void SetClr(int idx, COLORREF cr);

	EckInline COLORREF GetClr(int idx) const
	{
		switch (idx)
		{
		case 0:return m_Info.crText;
		case 1:return m_Info.crBK;
		case 2:return m_Info.crTextBK;
		}
		assert(FALSE);
		return 0;
	}

	/// <summary>
	/// 置渐变方式
	/// </summary>
	EckInline void SetGradientMode(int iGradientMode)
	{
		m_Info.iGradientMode = iGradientMode;
		Redraw();
	}

	EckInline int GetGradientMode() const
	{
		return m_Info.iGradientMode;
	}

	/// <summary>
	/// 置渐变颜色
	/// </summary>
	EckInline void SetGradientClr(int idx, COLORREF cr)
	{
		m_Info.crGradient[idx] = cr;
		Redraw();
	}

	EckInline COLORREF GetGradientClr(int idx) const
	{
		return m_Info.crGradient[idx];
	}

	/// <summary>
	/// 置省略号模式
	/// </summary>
	/// <param name="iEllipsisMode"></param>
	/// <returns></returns>
	EckInline void SetEllipsisMode(int iEllipsisMode)
	{
		m_Info.iEllipsisMode = iEllipsisMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetEllipsisMode() const
	{
		return m_Info.iEllipsisMode;
	}

	/// <summary>
	/// 置前缀解释模式
	/// </summary>
	/// <param name="iPrefixMode"></param>
	/// <returns></returns>
	EckInline void SetPrefixMode(int iPrefixMode)
	{
		m_Info.iPrefixMode = iPrefixMode;
		CalcPartsRect();
		Redraw();
	}

	EckInline int GetPrefixMode() const
	{
		return m_Info.iPrefixMode;
	}

	/// <summary>
	/// 置底图充满窗口
	/// </summary>
	EckInline void SetFullWndPic(BOOL bFullWndPic)
	{
		m_Info.bFullWndPic = bFullWndPic;
		Redraw();
	}

	EckInline BOOL GetFullWndPic() const
	{
		return m_Info.bFullWndPic;
	}

	/// <summary>
	/// 置透明标签
	/// </summary>
	EckInline void SetTransparent(BOOL bTransparent)
	{
		m_Info.bTransparent = bTransparent;
		ModifyStyle(bTransparent ? WS_EX_TRANSPARENT : 0, WS_EX_TRANSPARENT, GWL_EXSTYLE);
		Redraw();
	}

	EckInline BOOL GetTransparent() const
	{
		return m_Info.bTransparent;
	}

	/// <summary>
	/// 置鼠标穿透
	/// </summary>
	/// <param name="iMousePassingThrough"></param>
	/// <returns></returns>
	EckInline void SetMousePassingThrough(int iMousePassingThrough)
	{
		m_Info.iMousePassingThrough = iMousePassingThrough;
		Redraw();
	}

	EckInline int GetMousePassingThrough() const
	{
		return m_Info.iMousePassingThrough;
	}
};
ECK_NAMESPACE_END