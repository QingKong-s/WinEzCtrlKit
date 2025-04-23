#pragma once
#include "DuiBase.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CLabel : public CElem
{
private:
	IDWriteTextLayout* m_pLayout{};
	ID2D1SolidColorBrush* m_pBrush{};

	D2D1_COLOR_F m_crText{};
	D2D1_COLOR_F m_crBk{};

	ID2D1Bitmap* m_pBmp{};

	BITBOOL m_bUseThemeColor : 1{ TRUE };
	BITBOOL m_bTransparent : 1{ TRUE };
	BITBOOL m_bOnlyBitmap : 1{ FALSE };
	BITBOOL m_bFullElem : 1{ FALSE };
	BkImgMode m_eBkImgMode{ BkImgMode::TopLeft };
	BYTE m_eInterMode{ D2D1_INTERPOLATION_MODE_LINEAR };

	void UpdateTextLayout(PCWSTR pszText, int cchText)
	{
		SafeRelease(m_pLayout);
		if (!pszText || !cchText || m_bOnlyBitmap)
			return;
		float cx;
		if (m_pBmp)
			cx = GetWidthF() - m_pBmp->GetSize().width -
			GetTheme()->GetMetrics(Metrics::SmallPadding);
		else
			cx = GetWidthF();
		g_pDwFactory->CreateTextLayout(pszText, cchText,
			GetTextFormat(), cx, GetHeightF(), &m_pLayout);
	}

	EckInline void UpdateTextLayout()
	{
		UpdateTextLayout(GetText().Data(), GetText().Size());
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

			if (m_bOnlyBitmap)
			{
				DrawBackgroundImage(m_pDC, m_pBmp, GetViewRectF(), -1, -1,
					m_eBkImgMode, m_bFullElem, (D2D1_INTERPOLATION_MODE)m_eInterMode);
			}
			else
			{
				float x{};
				if (m_pBmp)
				{
					const auto sz = m_pBmp->GetSize();
					D2D1_RECT_F rc;
					rc.left = x;
					rc.top = (GetHeightF() - sz.height) / 2.f;
					rc.right = x + sz.width;
					rc.bottom = rc.top + sz.height;
					m_pDC->DrawBitmap(m_pBmp, rc);
					x += (sz.width + GetTheme()->GetMetrics(Metrics::SmallPadding));
				}

				if (m_pLayout)
				{
					if (m_bUseThemeColor)
					{
						D2D1_COLOR_F cr;
						GetTheme()->GetSysColor(SysColor::Text, cr);
						m_pBrush->SetColor(cr);
					}
					else
						m_pBrush->SetColor(m_crText);
					m_pDC->DrawTextLayout({ x }, m_pLayout, m_pBrush,
						DrawTextLayoutFlags);
				}
			}

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_SETTEXT:
			UpdateTextLayout((PCWSTR)lParam, (int)wParam);
			return 0;
		case WM_SIZE:
			UpdateTextLayout();
			return 0;
		case WM_SETFONT:
			UpdateTextLayout();
			if (lParam)
				InvalidateRect();
			break;

		case WM_ERASEBKGND:
		{
			const auto* const pps = (ELEMPAINTSTRU*)lParam;
			if (!m_bTransparent)
			{
				if (m_bUseThemeColor)
				{
					D2D1_COLOR_F cr;
					GetTheme()->GetSysColor(SysColor::Bk, cr);
					m_pBrush->SetColor(cr);
				}
				else
					m_pBrush->SetColor(m_crBk);
				m_pDC->FillRectangle(pps->rcfClip, m_pBrush);
			}
		}
		return 0;

		case WM_CREATE:
			m_pDC->CreateSolidColorBrush({}, &m_pBrush);
			UpdateTextLayout();
			break;

		case WM_DESTROY:
			SafeRelease(m_pLayout);
			SafeRelease(m_pBrush);
			SafeRelease(m_pBmp);
			break;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetBitmap(ID2D1Bitmap* pBmp)
	{
		ECK_DUILOCK;
		std::swap(m_pBmp, pBmp);
		if (m_pBmp)
			m_pBmp->AddRef();
		if (pBmp)
			pBmp->Release();
		UpdateTextLayout();
	}

	EckInlineCe auto GetBitmap() const { return m_pBmp; }

	EckInlineCe void SetUseThemeColor(BOOL b) { m_bUseThemeColor = b; }
	EckInlineCe BOOL GetUseThemeColor() const { return m_bUseThemeColor; }

	EckInlineCe void SetTransparent(BOOL b) { m_bTransparent = b; }
	EckInlineCe BOOL GetTransparent() const { return m_bTransparent; }

	EckInlineCe void SetOnlyBitmap(BOOL b) { m_bOnlyBitmap = b; }
	EckInlineCe BOOL GetOnlyBitmap() const { return m_bOnlyBitmap; }

	EckInlineCe void SetFullElem(BOOL b) { m_bFullElem = b; }
	EckInlineCe BOOL GetFullElem() const { return m_bFullElem; }

	EckInlineCe void SetBkImgMode(BkImgMode e) { m_eBkImgMode = e; }
	EckInlineCe BkImgMode GetBkImgMode() const { return m_eBkImgMode; }

	EckInlineCe void SetInterMode(D2D1_INTERPOLATION_MODE e) { m_eInterMode = e; }
	EckInlineCe D2D1_INTERPOLATION_MODE GetInterMode() const { return (D2D1_INTERPOLATION_MODE)m_eInterMode; }

	EckInlineCe void SetTextColor(D2D1_COLOR_F cr) { m_crText = cr; }
	EckInlineCe D2D1_COLOR_F GetTextColor() const { return m_crText; }

	EckInlineCe void SetBkColor(D2D1_COLOR_F cr) { m_crBk = cr; }
	EckInlineCe D2D1_COLOR_F GetBkColor() const { return m_crBk; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END