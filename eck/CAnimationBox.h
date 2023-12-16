#pragma once
#include "CWnd.h"
#include "GraphicsHelper.h"
#include "MathHelper.h"

ECK_NAMESPACE_BEGIN
enum class AbBorderStop
{
	None,
	HitBorderLine,
	CrossBorderLine,
};

struct ANBOXMARCHINFO
{
	float fStep;
	int msGap;
	float fMaxDistance;
	int msMaxTime;
	BOOL bAutoRemove;
	BOOL bStopWhenCollision;
	AbBorderStop uStopWhenHitBorder;
};


class CAbSpirit
{
	friend class CAnimationBox;
public:
	enum class Type
	{
		Invalid,
		Image,
		Text,
		MultiImage,
	};
	enum :UINT
	{
		SR_NONE = 0u,
		SR_OK = 1u << 0,
		SR_NEEDBEREMOVED = 1u << 1,
		SR_ARRIVEENDPOINT = 1u << 2,
		SR_NOUPDATEMARCH = 1u << 3,
		SR_TIMEOUT = 1u << 4,
	};
protected:
	Type m_uType = Type::Invalid;
	int m_iZOrder = 0;// Z序
	RECT m_rcPos{};
	D2D1_POINT_2F m_pt{};// 位置

	ANBOXMARCHINFO m_MarchInfo{};// 前进信息
	float m_fDistance = 0.f;
	ULONGLONG m_ullStopTime = 0;
	ULONGLONG m_ullLastTick = 0ull;

	float m_fAngle = 0.f;// 方向

	BOOL m_bMarching = FALSE;

	UINT March(ULONGLONG ullCurrTick)
	{
		if (!m_bMarching)
			return SR_NONE;
		EckAssert(!!m_ullStopTime && !!m_ullLastTick);
		UINT uRet = SR_OK;
		int iTimeDelta = (int)(ullCurrTick - m_ullLastTick);
		if (iTimeDelta >= m_MarchInfo.msGap)
		{
			if (ullCurrTick > m_ullStopTime)
			{
				uRet |= SR_TIMEOUT;
				return uRet;
			}
			float fStep = m_MarchInfo.fMaxDistance - m_fDistance;
			if (fStep < m_MarchInfo.fStep)
			{
				uRet |= SR_ARRIVEENDPOINT;
				if (m_MarchInfo.bAutoRemove)
					uRet |= SR_NEEDBEREMOVED;
				if (fStep < 0.f)
					return uRet;
			}
			else
				fStep = m_MarchInfo.fStep;
			m_pt.x += (cosf(m_fAngle) * fStep);
			m_pt.y += (sinf(m_fAngle) * fStep);
			m_fDistance += fStep;
			m_ullLastTick = ullCurrTick;
		}
		
	}

	void ResetMarch()
	{
		m_ullStopTime = 0ull;
		m_ullLastTick = 0ull;
		m_fDistance = 0.f;
	}
public:
	using TCoordComp = float;
	virtual ~CAbSpirit()
	{

	}

	void AutoMarch(const ANBOXMARCHINFO& Info)
	{
		m_MarchInfo = Info;
		if (m_MarchInfo.fMaxDistance == 0.f)
			m_MarchInfo.fMaxDistance = std::numeric_limits<float>{}.max();
		if (m_MarchInfo.msMaxTime == 0ull)
			m_MarchInfo.msMaxTime = std::numeric_limits<int>{}.max();

		m_ullLastTick = GetTickCount64();
		m_ullStopTime = m_ullLastTick + m_MarchInfo.msMaxTime;
		m_fDistance = 0.f;
		m_bMarching = TRUE;
	}

	void TurnDelta(float fAngleDelta)
	{
		m_fAngle += fAngleDelta;
		while (m_fAngle > 2.f * std::numbers::pi_v<float>)
			m_fAngle -= (2.f * std::numbers::pi_v<float>);
	}

	void Turn(float fAngle)
	{
		m_fAngle = fAngle;
	}

	int GetZOrder() const { return m_iZOrder; }

	EckInline virtual UINT Tick(ULONGLONG ullTick, ID2D1DeviceContext* pDC)
	{
		return March(ullTick);
	}

	void SetPos(D2D1_POINT_2F pt)
	{
		m_pt = pt;
	}
};

class CAbSpiritImage :public CAbSpirit
{
protected:
	ID2D1Image* m_pImage = NULL;
public:
	CAbSpiritImage() = default;
	CAbSpiritImage(ID2D1Image* pImage) :m_pImage{ pImage } {}

	UINT Tick(ULONGLONG ullTick, ID2D1DeviceContext* pDC) override
	{
		auto uRet = CAbSpirit::Tick(ullTick, pDC);
		if (uRet)
			pDC->DrawImage(m_pImage, m_pt);
		return uRet;
	}
};

class CAbSpiritText :public CAbSpirit
{
protected:

public:
	UINT Tick(ULONGLONG ullTick, ID2D1DeviceContext* pDC) override
	{
		return CAbSpirit::Tick(ullTick, pDC);
	}
};

class CAbSpiritMultiImage :public CAbSpirit
{
protected:
	std::vector<ID2D1Image*> m_vImage{};
};


class CAnimationBox :public CWnd
{
private:
	CEzD2D m_D2D{};
	int m_cxClient = 0,
		m_cyClient = 0;
	std::vector<CAbSpirit*> m_vSpirit{};
	ID2D1Brush* m_pBrBack = NULL;

	constexpr static UINT IDT_TICK = 101;
public:
	EckInline static ATOM RegisterWndClass() { return EzRegisterWndClass(WCN_ANIMATIONBOX); }

	LRESULT OnMsg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
	{
		switch (uMsg)
		{
		case WM_TIMER:
		{
			if (wParam != IDT_TICK)
				break;
			const auto pDC = m_D2D.GetDC();
			pDC->BeginDraw();
			pDC->FillRectangle(D2D1_RECT_F{ 0,0,(FLOAT)m_cxClient,(FLOAT)m_cyClient }, m_pBrBack);
			auto ullTick = GetTickCount64();
			for (auto e : m_vSpirit)
			{
				e->Tick(ullTick, pDC);
			}
			pDC->EndDraw();
			m_D2D.m_pSwapChain->Present(0, 0);
		}
		break;
		case WM_CREATE:
		{
			m_D2D.Create(EZD2D_PARAM::MakeFlip(hWnd, g_pDxgiFactory, g_pDxgiDevice, g_pD2dDevice, 0, 0));
			ID2D1SolidColorBrush* pBr;
			m_D2D.GetDC()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBr);
			m_pBrBack = pBr;
		}
		return 0;
		case WM_SIZE:
		{
			GET_SIZE_LPARAM(m_cxClient, m_cyClient, lParam);
			m_D2D.ReSize(2, m_cxClient, m_cyClient, 0);
		}
		return 0;
		}

		return CWnd::OnMsg(hWnd, uMsg, wParam, lParam);
	}

	HWND Create(PCWSTR pszText, DWORD dwStyle, DWORD dwExStyle,
		int x, int y, int cx, int cy, HWND hParent, int nID, PCVOID pData = NULL) override
	{
		m_hWnd = IntCreate(dwExStyle, WCN_ANIMATIONBOX, pszText, dwStyle,
			x, y, cx, cy, hParent, i32ToP<HMENU>(nID), g_hInstance, NULL);
		return m_hWnd;
	}

	void AddSpirit(CAbSpirit* pSpirit)
	{
		EckAssert(pSpirit);
		if (!m_vSpirit.size())
			SetTimer(GetHWND(), IDT_TICK, 20, NULL);
		auto it = std::upper_bound(m_vSpirit.begin(), m_vSpirit.end(),
			pSpirit, [](const CAbSpirit* p1, const CAbSpirit* p2)->bool
			{
				return p1->GetZOrder() < p2->GetZOrder();
			});
		if (it == m_vSpirit.end())
			m_vSpirit.push_back(pSpirit);
		else [[likely]]
			m_vSpirit.insert(it, pSpirit);
	}

	void RemoveSpirit(int idxSpirit)
	{
		EckAssert(idxSpirit >= 0 && idxSpirit < (int)m_vSpirit.size());
		m_vSpirit.erase(m_vSpirit.begin() + idxSpirit);
		if (!m_vSpirit.size())
			KillTimer(GetHWND(), IDT_TICK);
	}



	void SetZOrder(int idxSpirit, int iZOrder)
	{

	}

	EckInline auto GetDC() const { return m_D2D.GetDC(); }
};



ECK_NAMESPACE_END