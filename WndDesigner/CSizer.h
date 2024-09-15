#pragma once
#include "CApp.h"

#include "eck\CBk.h"

class CSizer;

enum class SizerHTCode
{
	None,
	LeftTop,
	RightTop,
	LeftBottom,
	RightBottom,
	Top,
	Bottom,
	Left,
	Right
};

class CSizerBlock :public eck::CBk
{
private:
	friend class CSizer;

	HBRUSH m_hbrBlock = nullptr;
	RECT m_rcClient{};
	CSizer* m_pSizer = nullptr;
	SizerHTCode m_uType = SizerHTCode::None;
	HCURSOR m_hCursor = nullptr;

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
public:
	ECK_CWND_CREATE;
	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, HMENU hMenu, PCVOID pData = nullptr) override
	{
		return IntCreate(dwExStyle, eck::WCN_BK, pszText, dwStyle,
			x, y, cx, cy, hParent, hMenu, eck::g_hInstance, nullptr);
	}

	EckInline void SetWindowProc(WNDPROC pfnWndProc)
	{
		EckDbgPrint(L"CSizerBlock::SetWindowProc 不支持");
		return;
	}

	void BindSizer(CSizer* pSizer, SizerHTCode uType);
};

class CSizer
{
private:
	friend class CSizerBlock;
	HWND m_hBK = nullptr;
	HWND m_hBottomWorkWnd = nullptr;
	HWND m_hWorkWnd = nullptr;
	HWND m_hWorkWndParent = nullptr;
	int m_sizeBlock = 0;
	CSizerBlock m_Block[8]{};

	HDC m_hDC = nullptr;
	HPEN m_hPen = nullptr;
	HGDIOBJ m_hOld = nullptr;
	RECT m_rcOrg{};
	RECT m_rcLast{};
	BOOL m_bLBtnDown = FALSE;
public:
	~CSizer()
	{
		for (auto& x : m_Block)
			x.Destroy();
		DeleteObject(m_hPen);
	}

	void Create(HWND hBK, HWND hBottomWorkWnd);

	EckInline void SetBlockSize(int i)
	{
		m_sizeBlock = i;
	}

	HWND SetTargetWindow(HWND hWnd);

	EckInline HWND GetTargetWindow()
	{
		return m_hWorkWnd;
	}

	SizerHTCode HitTest(POINT pt);

	void MoveToTargetWindow();

	EckInline void Show(BOOL bShow)
	{
		int iShow = (bShow ? SW_SHOWNOACTIVATE : SW_HIDE);
		for (auto& x : m_Block)
			x.Show(iShow);
	}
private:
	RECT SizerMakeRect(POINT ptCursor, SizerHTCode uType);

	void OnBlockLButtonDown(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockLButtonUp(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockMouseMove(CSizerBlock* pBlock, LPARAM lParam);
};