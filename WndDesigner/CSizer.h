#pragma once
#include "CApp.h"
#include "..\eck\CBk.h"

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

	HBRUSH m_hbrBlock = NULL;
	RECT m_rcClient{};
	CSizer* m_pSizer = NULL;
	SizerHTCode m_uType = SizerHTCode::None;
	HCURSOR m_hCursor = NULL;

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	
	ECK_CWND_CREATE;

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
	HWND m_hBK = NULL;
	HWND m_hBottomWorkWnd = NULL;
	HWND m_hWorkWnd = NULL;
	HWND m_hWorkWndParent = NULL;
	int m_sizeBlock = 0;
	CSizerBlock m_Block[8]{};

	HDC m_hDC = NULL;
	HPEN m_hPen = NULL;
	HGDIOBJ m_hOld = NULL;
	RECT m_rcOrg{};
	RECT m_rcLast{};
	BOOL m_bLBtnDown = FALSE;
public:
	~CSizer()
	{
		for (auto& x : m_Block)
			DestroyWindow(x);
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
			ShowWindow(x, iShow);
	}
private:
	RECT SizerMakeRect(POINT ptCursor, SizerHTCode uType);

	void OnBlockLButtonDown(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockLButtonUp(CSizerBlock* pBlock, LPARAM lParam);

	void OnBlockMouseMove(CSizerBlock* pBlock, LPARAM lParam);
};