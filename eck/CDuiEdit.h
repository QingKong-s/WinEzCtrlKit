#pragma once
#include "DuiBase.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CEdit :public CElem
{
private:
	IDWriteTextFormat* m_pFormat = NULL;
	IDWriteTextLayout* m_pLayout = NULL;
	CRefStrW m_rsText{};
	D2D1_RECT_F m_rcfTextAera{};

	BITBOOL m_bUserTextAera : 1 = FALSE;

	void UpdateLayout(PCWSTR psz, int cch)
	{
		SafeRelease(m_pLayout);
		g_pDwFactory->CreateTextLayout(psz, cch, m_pFormat, 
			GetViewWidthF(), GetViewHeightF(), &m_pLayout);

	}
public:
	LRESULT OnEvent(UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_PAINT:
		{
			ELEMPAINTSTRU ps;
			BeginPaint(ps, wParam, lParam);



			EndPaint(ps);
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	float GetTextRectWidth()
	{
		return m_rcfTextAera.right - m_rcfTextAera.left;
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END