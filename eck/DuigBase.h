#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUIG_NAMESPACE_BEGIN
class CElem : public ILayout
{

};

class CDuiWnd : public CWnd
{
private:
	CElem* m_pFirstChild{};	// 第一个子元素
	CElem* m_pLastChild{};	// 最后一个子元素

	CEzCDC m_DC{};
	GpGraphics* m_pGraphics{};
public:
	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_DESTROY:
		{
			GdipDeleteGraphics(m_pGraphics);
			m_pGraphics = nullptr;
			m_DC.Destroy();
		}
		break;
		case WM_CREATE:
		{
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			m_DC.Create(hWnd, rcClient.right - rcClient.left,
				rcClient.bottom - rcClient.top);
			GdipCreateFromHDC(m_DC.GetDC(), &m_pGraphics);
		}
		break;
		}
		return __super::OnMsg(hWnd, uMsg, wParam, lParam);
	}
};
ECK_DUIG_NAMESPACE_END
ECK_NAMESPACE_END