/*
* WinEzCtrlKit Library
*
* CDuiEdit.h ： DUI编辑框
*
* Copyright(C) 2024 QingKong
*/
#pragma once
#if ECKCXX20
#include "CDuiScrollBar.h"
#include "ShellHelper.h"

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
class CEdit :public CElem
{
private:
	IDWriteTextFormat* m_pFormat = NULL;
	IDWriteTextLayout* m_pLayout = NULL;
	ID2D1SolidColorBrush* m_pBr = NULL;
	D2D1_RECT_F m_rcfTextAera{};

	BITBOOL m_bUserTextAera : 1 = FALSE;
	UINT32 m_uSelBegin{ Neg1U };
	UINT32 m_cSel{};
	UINT32 m_uBtnSelStart{ Neg1U };

	CScrollBar m_SBH{};
	CScrollBar m_SBV{};

	std::vector<DWRITE_HIT_TEST_METRICS> m_vHtmRange{};

	BITBOOL m_bLBtnDown : 1 = FALSE;

	void UpdateLayout()
	{
		SafeRelease(m_pLayout);
		if (m_pFormat)
			g_pDwFactory->CreateTextLayout(m_rsText.Data(), m_rsText.Size(), m_pFormat,
				m_rcfTextAera.right - m_rcfTextAera.left,
				m_rcfTextAera.bottom - m_rcfTextAera.top, &m_pLayout);
		if (m_pLayout)
		{
			m_pLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			m_pLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
		}
		UpdateSB();
	}

	void UpdateSB()
	{
		DWRITE_TEXT_METRICS tm;
		m_pLayout->GetMetrics(&tm);
		const auto psv = m_SBV.GetScrollView();
		psv->SetMin(0);
		psv->SetMax((int)tm.height);
		m_SBV.GetScrollView()->SetPage((int)(m_rcfTextAera.bottom - m_rcfTextAera.top));
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

			if (m_pLayout)
			{
				const float oy = -(float)m_SBV.GetScrollView()->GetPos();
				const float ox = 0.f;
				if (m_uSelBegin != Neg1U)
				{
					UINT32 cActualLine;
					m_pLayout->GetLineMetrics(NULL, 0, &cActualLine);
					DWRITE_TEXT_METRICS tm;
					m_pLayout->GetMetrics(&tm);

					m_vHtmRange.resize(cActualLine * tm.maxBidiReorderingDepth);
					UINT32 cActualMetrics;
					m_pLayout->HitTestTextRange(m_uSelBegin, m_cSel, m_rcfTextAera.left, m_rcfTextAera.top,
						m_vHtmRange.data(), (UINT32)m_vHtmRange.size(), &cActualMetrics);

					m_pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Blue, 0.6f));
					EckCounter(cActualMetrics, i)
					{
						const auto& e = m_vHtmRange[i];
						m_pDC->FillRectangle(MakeD2DRcF(
							e.left + ox,
							e.top + oy,
							e.width, e.height), m_pBr);
					}

					m_pBr->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
					if (m_uSelBegin == m_uBtnSelStart)
					{
						const auto& e = m_vHtmRange[cActualMetrics - 1];
						m_pDC->DrawLine({ e.left + e.width + ox,e.top + oy },
							{ e.left + e.width + ox,e.top + e.height + oy },
							m_pBr, 2.f);
					}
					else
					{
						const auto& e = m_vHtmRange.front();
						m_pDC->DrawLine({ e.left + ox,e.top + oy },
							{ e.left + ox,e.top + e.height + oy },
							m_pBr, 2.f);
					}
				}

				m_pBr->SetColor(D2D1::ColorF(D2D1::ColorF::White));
				m_pDC->DrawTextLayout({ m_rcfTextAera.left + ox,m_rcfTextAera.top+oy }, m_pLayout, m_pBr,
					D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
			}

			ECK_DUI_DBG_DRAW_FRAME;
			EndPaint(ps);
		}
		return 0;

		case WM_SETCURSOR:
			SetCursor(LoadCursorW(NULL, IDC_IBEAM));
			return TRUE;

		case WM_MOUSEMOVE:
		{
			if (m_bLBtnDown)
			{
				ECK_DUILOCK;
				const float oy = (float)m_SBV.GetScrollView()->GetPos();
				const float ox = 0.f;
				EckAssert(m_uBtnSelStart != (UINT32)-1);
				D2D1_POINT_2F pt ECK_GET_PT_LPARAM_F(lParam);
				ClientToElem(pt);
				pt.x -= m_rcfTextAera.left;
				pt.y -= m_rcfTextAera.top;
				pt.x += ox;
				pt.y += oy;

				BOOL bInside;
				BOOL bTrailing;
				DWRITE_HIT_TEST_METRICS htm;
				m_pLayout->HitTestPoint(pt.x, pt.y, &bTrailing, &bInside, &htm);
				auto uHit = htm.textPosition;
				if (bTrailing)
					if ((uHit + 1 < (UINT)m_rsText.Size()) &&
						IS_LOW_SURROGATE(m_rsText[uHit + 1]))
						uHit += 2;
					else
						++uHit;
				const auto uOld0{ m_uSelBegin }, uOld1{ m_cSel };
				if (uHit < m_uBtnSelStart)
				{
					m_uSelBegin = uHit;
					m_cSel = m_uBtnSelStart - uHit;
				}
				else
				{
					m_uSelBegin = m_uBtnSelStart;
					m_cSel = uHit - m_uBtnSelStart;
				}
				if (m_uSelBegin != uOld0 || m_cSel != uOld1)
					InvalidateRect();
			}
		}
		return 0;

		case WM_KEYDOWN:
		{
			switch (wParam)
			{
			case VK_BACK:
			{
				ECK_DUILOCK;
				if (m_uSelBegin != Neg1U)
				{
					if (m_cSel)
					{
						m_rsText.Erase(m_uSelBegin, m_cSel);
						m_cSel = 0u;
						UpdateLayout();
						InvalidateRect();
					}
					else if (m_uSelBegin != 0)
					{
						if (IS_LOW_SURROGATE(m_rsText[m_uSelBegin - 1]))
							m_rsText.Erase(m_uSelBegin -= 2, 2);
						else
							m_rsText.Erase(--m_uSelBegin);
						UpdateLayout();
						InvalidateRect();
					}
				}
			}
			break;

			case VK_DELETE:
			{
				ECK_DUILOCK;
				if (m_uSelBegin != Neg1U)
				{
					if (m_cSel)
					{
						m_rsText.Erase(m_uSelBegin, m_cSel);
						m_cSel = 0u;
						UpdateLayout();
						InvalidateRect();
					}
					else if (m_uSelBegin < (UINT)m_rsText.Size())
					{
						if (IS_HIGH_SURROGATE(m_rsText[m_uSelBegin]))
							m_rsText.Erase(m_uSelBegin, 2);
						else
							m_rsText.Erase(m_uSelBegin);
						UpdateLayout();
						InvalidateRect();
					}
				}
			}
			break;

			case 'A':
			{
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				{
					ECK_DUILOCK;
					m_uSelBegin = 0u;
					m_cSel = m_rsText.Size();
					InvalidateRect();
				}
			}
			break;

			case 'V':
			{
				ECK_DUILOCK;
				if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
					m_uSelBegin != Neg1U)
				{
					const auto rs = GetClipboardString(GetWnd()->HWnd);
					if (!rs.IsEmpty())
						InsertText(m_uSelBegin, rs.Data(), rs.Size());
				}
			}
			break;

			case 'C':
			{
				ECK_DUILOCK;
				if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&
					m_uSelBegin != Neg1U && m_cSel)
					SetClipboardString(m_rsText.Data() + m_uSelBegin, m_cSel, GetWnd()->HWnd);
			}
			break;
			}
		}
		return 0;

		case WM_CHAR:
		{
			const WCHAR ch = (WCHAR)wParam;
			if (ch >= 32 && ch <= 126)
			{
				if (m_uSelBegin != Neg1U)
				{
					ECK_DUILOCK;
					if (m_cSel)
					{
						const WCHAR sz[]{ ch,L'\0' };
						m_rsText.Replace(m_uSelBegin, m_cSel, sz, 1);
					}
					else
						m_rsText.InsertChar(m_uSelBegin++, ch);
					UpdateLayout();
					InvalidateRect();
				}
			}
		}
		return 0;

		case WM_IME_COMPOSITION:
		{
			ECK_DUILOCK;
			if (!IsBitSet(lParam, GCS_RESULTSTR) ||
				m_uSelBegin == Neg1U)
				break;
			const HIMC hImc = ImmGetContext(GetWnd()->HWnd);
			const int cch = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, NULL, 0) / sizeof(WCHAR);
			if (cch)
			{
				CRefStrW rs(cch);
				ImmGetCompositionStringW(hImc, GCS_RESULTSTR, rs.Data(), (DWORD)rs.ByteSize());
				InsertText(m_uSelBegin, rs.Data(), rs.Size());
			}
			ImmReleaseContext(GetWnd()->HWnd, hImc);
		}
		break;

		case WM_IME_STARTCOMPOSITION:
		{
			COMPOSITIONFORM cf;
			cf.dwStyle = CFS_POINT;
			cf.ptCurrentPos = { m_rcInClient.left,m_rcInClient.bottom };
			const HIMC hImc = ImmGetContext(GetWnd()->HWnd); 
			ImmSetCompositionWindow(hImc, &cf);
			ImmReleaseContext(GetWnd()->HWnd, hImc);
			return 0;
		}
		break;

		case WM_LBUTTONDOWN:
		{
			ECK_DUILOCK;
			const float oy = (float)m_SBV.GetScrollView()->GetPos();
			const float ox = 0.f;
			SetFocus();
			SetCapture();
			m_bLBtnDown = TRUE;
			D2D1_POINT_2F pt ECK_GET_PT_LPARAM_F(lParam);
			ClientToElem(pt);
			pt.x -= m_rcfTextAera.left;
			pt.y -= m_rcfTextAera.top;
			pt.x += ox;
			pt.y += oy;

			BOOL bInside;
			BOOL bTrailing;
			DWRITE_HIT_TEST_METRICS htm;
			m_pLayout->HitTestPoint(pt.x, pt.y, &bTrailing, &bInside, &htm);
			m_uBtnSelStart = htm.textPosition;
			if (bTrailing)
				if ((m_uBtnSelStart + 1 < (UINT)m_rsText.Size()) &&
					IS_LOW_SURROGATE(m_rsText[m_uBtnSelStart + 1]))
					m_uBtnSelStart += 2;
				else
					++m_uBtnSelStart;

			m_uSelBegin = m_uBtnSelStart;
			m_cSel = 0u;
			InvalidateRect();
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				ReleaseCapture();
				m_bLBtnDown = FALSE;
			}
		}
		return 0;

		case WM_MOUSEWHEEL:
		{
			m_SBV.GetScrollView()->OnMouseWheel2(-GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
			GetWnd()->WakeRenderThread();
		}
		return 0;

		case WM_SIZE:
		{
			ECK_DUILOCK;
			m_rcfTextAera = { 15,5,GetWidthF() - 10,5 + GetHeightF() - 10 };
			const int cx = (int)GetWnd()->GetDs().CommSBCxy;
			const int x = GetWidth() - cx;
			m_SBV.SetRect({ x,0,x + cx,GetHeight() });
		}
		return 0;

		case WM_SETTEXT:
		{
			ECK_DUILOCK;
			UpdateLayout();
			InvalidateRect();
		}
		return 0;

		case WM_CREATE:
		{
			m_SBV.Create(NULL, DES_VISIBLE| DES_COMPOSITED, 0, 0, 0, 0, 0, this, GetWnd());
			m_SBV.GetScrollView()->SetCallBack([](int iPos, int iPrevPos, LPARAM lParam)
				{
					//if (iPos != iPrevPos)
					{
						const auto p = (CEdit*)lParam;
						p->InvalidateRect();
					}
				}, (LPARAM)this);
			m_pDC->CreateSolidColorBrush({}, &m_pBr);
		}
			break;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetTextFormat(IDWriteTextFormat* ptf)
	{
		ECK_DUILOCK;
		std::swap(ptf, m_pFormat);
		m_pFormat->AddRef();
		if (ptf)
			ptf->Release();
		UpdateLayout();
	}

	EckInline BOOL Copy()
	{
		ECK_DUILOCK;
		if (m_uSelBegin == Neg1U || m_cSel == 0)
			return FALSE;
		return SetClipboardString(m_rsText.Data() + m_uSelBegin, m_cSel, GetWnd()->HWnd);
	}

	EckInline BOOL Cut()
	{
		ECK_DUILOCK;
		if (m_uSelBegin == Neg1U || m_cSel == 0)
			return FALSE;
		const BOOL b = SetClipboardString(m_rsText.Data() + m_uSelBegin, m_cSel, GetWnd()->HWnd);
		m_rsText.Erase(m_uSelBegin, m_cSel);
		UpdateLayout();
		InvalidateRect();
		return b;
	}

	EckInline void InsertText(int pos, PCWSTR psz, int cch = -1)
	{
		ECK_DUILOCK;
		if (pos < 0 || pos > m_rsText.Size())
			return;
		if (cch < 0)
			cch = (int)wcslen(psz);
		m_rsText.Insert(pos, psz, cch);
		if (m_uSelBegin >= (UINT)pos)
			m_uSelBegin += cch;
		if (m_uBtnSelStart >= (UINT)pos)
			m_uBtnSelStart += cch;
		UpdateLayout();
		InvalidateRect();
	}
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END
#else
#error "EckDui requires C++20"
#endif// ECKCXX20