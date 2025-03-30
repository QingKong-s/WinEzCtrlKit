#pragma once
#include "DuiBase.h"

#if !ECKCXX20
#error "EckDui requires C++20"
#endif// !ECKCXX20

ECK_NAMESPACE_BEGIN
ECK_DUI_NAMESPACE_BEGIN
struct CBTN_CUSTOM_DRAW : CUSTOM_DRAW
{
	State eState;
	D2D1_RECT_F rcImg;
	ID2D1Bitmap* pImg;
};

class CCircleButton :public CElem
{
private:
	ID2D1Bitmap* m_pImg{};

	D2D1_SIZE_F m_sizeImg{};
	D2D1_INTERPOLATION_MODE m_iInterpolation = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;

	BITBOOL m_bHot : 1 = FALSE;
	BITBOOL m_bLBtnDown : 1 = FALSE;

	BITBOOL m_bAutoImgSize : 1{ TRUE };
	BITBOOL m_bCustomDraw : 1{ FALSE };

	BOOL PtInBtn(POINT ptInClient)
	{
		const int iRad = std::min(GetWidth(), GetHeight()) / 2;
		const POINT ptCenter
		{
			GetRectInClient().left + iRad,
			GetRectInClient().top + iRad
		};
		return PtInCircle(ptInClient, ptCenter, iRad);
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

			State eState;
			if (m_bLBtnDown)
				eState = State::Selected;
			else if (m_bHot)
				eState = State::Hot;
			else
				eState = State::Normal;

			BOOL bSkipDefault{};
			CBTN_CUSTOM_DRAW cd;
			if (m_pImg)
			{
				cd.rcImg.left = (GetWidthF() - m_sizeImg.width) / 2.f;
				cd.rcImg.top = (GetHeightF() - m_sizeImg.height) / 2.f;
				cd.rcImg.right = cd.rcImg.left + m_sizeImg.width;
				cd.rcImg.bottom = cd.rcImg.top + m_sizeImg.height;
			}

			if (m_bCustomDraw)
			{
				cd.uCode = EE_CUSTOMDRAW;
				cd.dwStage = CDDS_PREPAINT;
				cd.eState = eState;
				cd.pImg = m_pImg;

				bSkipDefault = (GenElemNotify(&cd) & CDRF_SKIPDEFAULT);
			}

			if (!bSkipDefault)
			{
				GetTheme()->DrawBackground(Part::CircleButton, eState, GetViewRectF());
				if (m_pImg)
					m_pDC->DrawBitmap(m_pImg, cd.rcImg, 1.f, m_iInterpolation);
			}

			EndPaint(ps);
		}
		return 0;

		case WM_NCHITTEST:
			return (PtInBtn(ECK_GET_PT_LPARAM(lParam)) ? HTCLIENT : HTTRANSPARENT);

		case WM_MOUSEMOVE:
		{
			if (!m_bHot)
			{
				m_bHot = TRUE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_MOUSELEAVE:
		{
			if (m_bHot)
			{
				m_bHot = FALSE;
				InvalidateRect();
			}
		}
		return 0;

		case WM_LBUTTONDBLCLK:// 连击修正
		case WM_LBUTTONDOWN:
		{
			SetFocus();
			m_bLBtnDown = TRUE;
			SetCapture();
			InvalidateRect();
		}
		return 0;

		case WM_LBUTTONUP:
		{
			if (m_bLBtnDown)
			{
				m_bLBtnDown = FALSE;
				ReleaseCapture();
				InvalidateRect();
				if (PtInBtn(ECK_GET_PT_LPARAM(lParam)))
				{
					DUINMHDR nm{ EE_COMMAND };
					GenElemNotify(&nm);
				}
			}
		}
		return 0;

		case WM_SIZE:
		{
			if (m_bAutoImgSize)
			{
				m_sizeImg.width = std::min(GetWidthF(), GetHeightF());
				m_sizeImg.width /= 1.414f;
				m_sizeImg.height = m_sizeImg.width;
			}
		}
		return 0;

		case WM_DESTROY:
		{
			SafeRelease(m_pImg);
			m_bHot = FALSE;
			m_bLBtnDown = FALSE;
		}
		return 0;
		}
		return CElem::OnEvent(uMsg, wParam, lParam);
	}

	void SetImage(ID2D1Bitmap* pImg)
	{
		std::swap(m_pImg, pImg);
		if (m_pImg)
			m_pImg->AddRef();
		if (pImg)
			pImg->Release();
	}
	EckInlineNdCe ID2D1Bitmap* GetImage() const { return m_pImg; }

	EckInlineCe void SetImageSize(D2D1_SIZE_F s) { m_sizeImg = s; }
	EckInlineNdCe D2D1_SIZE_F GetImageSize() const { return m_sizeImg; }

	EckInlineCe void SetInterpolationMode(D2D1_INTERPOLATION_MODE e) { m_iInterpolation = e; }
	EckInlineNdCe D2D1_INTERPOLATION_MODE GetInterpolationMode() const { return m_iInterpolation; }

	EckInlineCe void SetAutoImageSize(BOOL b) { m_bAutoImgSize = b; }
	EckInlineNdCe BOOL GetAutoImageSize() const { return m_bAutoImgSize; }

	EckInlineCe void SetCustomDraw(BOOL b) { m_bCustomDraw = b; }
	EckInlineNdCe BOOL GetCustomDraw() const { return m_bCustomDraw; }
};
ECK_DUI_NAMESPACE_END
ECK_NAMESPACE_END