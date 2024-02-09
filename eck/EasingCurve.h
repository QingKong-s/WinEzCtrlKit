#pragma once
#include "ECK.h"
#include "CMsgMmTimer.h"

#define ECK_EASING_NAMESPACE_BEGIN namespace Easing {
#define ECK_EASING_NAMESPACE_END }

ECK_NAMESPACE_BEGIN

template<class FAn>
struct CEasingAn
{
	float m_fCurrTime = 0.f;
	float m_fDuration = 0.f;
	float m_fStart = 0.f;
	float m_fDistance = 0.f;

	EckInline void Begin(float fStart, float fDistance, float fDuration)
	{
		m_fStart = fStart;
		m_fDistance = fDistance;
		m_fDuration = fDuration;
		m_fCurrTime = 0.f;
	}

	EckInline float Tick(float fElapse)
	{
		m_fCurrTime += fElapse;
		return FAn{}(m_fCurrTime, m_fStart, m_fDistance, m_fDuration);
	}

	EckInline BOOL IsEnd()
	{
		return m_fCurrTime >= m_fDuration;
	}
};

ECK_EASING_NAMESPACE_BEGIN
struct FInCircle
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return -fDistance * (sqrt(1.f - fCurrTime * fCurrTime) - 1.f) + fStart;
	}
};

struct FOutCircle
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * sqrt(1 - fCurrTime * fCurrTime) + fStart;
	}
};

struct FInOutCircle
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return -fDistance / 2.f * (sqrt(1.f - fCurrTime * fCurrTime) - 1.f) + fStart;
		else
		{
			fCurrTime -= 2.f;
			return fDistance / 2.f * (sqrt(1.f - fCurrTime * fCurrTime) + 1.f) + fStart;
		}
	}
};

struct FInSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return -fDistance * cosf(fCurrTime / fDuration * PiF / 2.f) + fDistance + fStart;
	}
};

struct FOutSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * sinf(fCurrTime / fDuration * PiF / 2.f) + fStart;
	}
};

struct FInOutSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return -fDistance / 2.f * (cosf(PiF * fCurrTime / fDuration) - 1.f) + fStart;
	}
};

struct FInQuad
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime + fStart;
	}
};

struct FOutQuad
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return -fDistance * fCurrTime * (fCurrTime - 2.f) + fStart;
	}
};

struct FInOutQuad
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return fDistance / 2.f * fCurrTime * fCurrTime + fStart;
		else
		{
			fCurrTime -= 1.f;
			return -fDistance / 2.f * (fCurrTime * (fCurrTime - 2.f) - 1.f) + fStart;
		}
	}
};

struct FInCubic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

struct FOutCubic
{
	EckInline constexpr float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}
};

struct FInOutCubic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return fDistance / 2.f * fCurrTime * fCurrTime * fCurrTime + fStart;
		else
		{
			fCurrTime -= 2.f;
			return fDistance / 2.f * (fCurrTime * fCurrTime * fCurrTime + 2.f) + fStart;
		}
	}
};

struct FInQuart
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

struct FOutQuart
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return -fDistance * (fCurrTime * fCurrTime * fCurrTime * fCurrTime - 1.f) + fStart;
	}
};

struct FInOutQuart
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return fDistance / 2.f * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
		else
		{
			fCurrTime -= 2.f;
			return -fDistance / 2.f * (fCurrTime * fCurrTime * fCurrTime * fCurrTime - 2.f) + fStart;
		}
	}
};

struct FInQuint
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

struct FOutQuint
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}
};

struct FInOutQuint
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return fDistance / 2.f * fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
		else
		{
			fCurrTime -= 2.f;
			return fDistance / 2.f * (fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + 2.f) + fStart;
		}
	}
};

struct FInExpo
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, 0.f))
			return fStart;
		else ECKLIKELY
			return fDistance * powf(2.f, 10.f * (fCurrTime / fDuration - 1.f)) + fStart;
	}
};

struct FOutExpo
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, fDuration))
			return fStart + fDistance;
		else ECKLIKELY
			return fDistance * (-powf(2.f, -10.f * fCurrTime / fDuration) + 1.f) + fStart;
	}
};

struct FInOutExpo
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, 0.f))
			return fStart;
		else if (FloatEqual(fCurrTime, fDuration))
			return fStart + fDistance;
		else ECKLIKELY
		{
			fCurrTime /= (fDuration / 2.f);
			if (fCurrTime < 1.f)
				return fDistance / 2.f * powf(2.f, 10.f * (fCurrTime - 1.f)) + fStart;
			else
				return fDistance / 2.f * (-powf(2.f, -10.f * (fCurrTime - 1.f)) + 2.f) + fStart;
		}
	}
};

struct FInElastic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, 0.f))
			return fStart;
		else if (FloatEqual(fCurrTime, fDuration))
			return fStart + fDistance;
		else ECKLIKELY
		{
			fCurrTime /= fDuration;
			const float f = fCurrTime - 1.f;
			return -fDistance * powf(2.f, 10.f * f) * sinf((f - 0.075f) * 2.f * PiF / 0.3f) + fStart;
		}
	}
};

struct FOutElastic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, 0.f))
			return fStart;
		else if (FloatEqual(fCurrTime, fDuration))
			return fStart + fDistance;
		else ECKLIKELY
		{
			fCurrTime /= fDuration;
			return fDistance * powf(2.f, -10.f * fCurrTime) * sinf((fCurrTime - 0.075f) * 2.f * PiF / 0.3f) + fDistance + fStart;
		}
	}
};

struct FInOutElastic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (FloatEqual(fCurrTime, 0.f))
			return fStart;
		else if (FloatEqual(fCurrTime, fDuration))
			return fStart + fDistance;
		else ECKLIKELY
		{
			fCurrTime /= (fDuration / 2.f);
			const float f = fCurrTime - 1.f;
			if (fCurrTime < 1.f)
				return -0.5f * fDistance * powf(2.f, 10.f * f) * sinf((f - 0.1125f) * 2.f * PiF / 0.45f) + fStart;
			else
				return fDistance * powf(2.f, -10.f * f) * sinf((f - 0.1125f) * 2.f * PiF / 0.45f) * 0.5f + fDistance + fStart;
		}
	}
};

struct FInBack
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 1.70158f;
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime - f) + fStart;
	}
};

struct FOutBack
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 1.70158f;
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime + f) + 1.f) + fStart;
	}
};

struct FInOutBack
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 1.70158f * 1.525f;
		fCurrTime /= (fDuration / 2.f);
		if (fCurrTime < 1.f)
			return fDistance / 2.f * (fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime - f)) + fStart;
		else
		{
			fCurrTime -= 2.f;
			return fDistance / 2.f * (fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime + f) + 2.f) + fStart;
		}
	}
};

struct FOutBounce
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		if (fCurrTime < 1.f / 2.75f)
			return fDistance * (7.5625f * fCurrTime * fCurrTime) + fStart;
		else if (fCurrTime < 2.f / 2.75f)
		{
			fCurrTime -= 1.5f / 2.75f;
			return fDistance * (7.5625f * fCurrTime * fCurrTime + 0.75f) + fStart;
		}
		else if (fCurrTime < 2.5f / 2.75f)
		{
			fCurrTime -= 2.25f / 2.75f;
			return fDistance * (7.5625f * fCurrTime * fCurrTime + 0.9375f) + fStart;
		}
		else
		{
			fCurrTime -= 2.625f / 2.75f;
			return fDistance * (7.5625f * fCurrTime * fCurrTime + 0.984375f) + fStart;
		}
	}
};

struct FInBounce
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance - FOutBounce{}(fDuration - fCurrTime, 0.f, fDistance, fDuration) + fStart;
	}
};

struct FInOutBounce
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (fCurrTime < fDuration / 2.f)
			return FInBounce{}(fCurrTime * 2.f, 0.f, fDistance, fDuration) * 0.5f + fStart;
		else
			return FOutBounce{}(fCurrTime * 2.f - fDuration, 0.f, fDistance, fDuration) * 0.5f + fDistance * 0.5f + fStart;
	}
};

struct FLinear
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * fCurrTime / fDuration + fStart;
	}
};

struct FPunch
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 9.f;
		if (fCurrTime == 0.f)
			return fStart;
		else if (fCurrTime == fDuration)
			return fStart + fDistance;
		else
		{
			const float fCurrTime = fCurrTime / fDuration - 1.f;
			return fDistance * expf(-fCurrTime * f) * sinf(6.f * PiF * fCurrTime) + fDistance + fStart;
		}
	}
};

struct FSpring
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 0.3f;
		if (fCurrTime == 0.f)
			return fStart;
		else if (fCurrTime == fDuration)
			return fStart + fDistance;
		else
		{
			const float fCurrTime = fCurrTime / fDuration - 1.f;
			return -fDistance * expf(-fCurrTime * f) * sinf(2.f * PiF * fCurrTime / 0.3f) + fDistance + fStart;
		}
	}
};
ECK_EASING_NAMESPACE_END

enum
{
	ECBF_DEF = 0,
	ECBF_SUPERPOSE_CURRPOS = 1u << 0,
};

template<class FAn>
class CEasingCurve
{
public:
	using FCallBack = void(*)(float fCurrValue, float fOldValue, LPARAM lParam);
private:
	CEasingAn<FAn> m_Calc{};
	CMsgMmTimer m_Timer{};

	float m_fElapse = 0.f;
	float m_fCurrValue = 0.f;

	LPARAM m_lParam = 0;
	FCallBack m_pfnCallBack = NULL;

	BITBOOL m_bActive : 1 = FALSE;

	static UINT m_uMsgTimer;

	static LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
	{
		auto p = (CEasingCurve*)lParam;
		if (uMsg == p->m_uMsgTimer && wParam == p->m_Timer.GetID() && lParam == (LPARAM)p)
		{
			EckAssert(p->m_pfnCallBack);
			const float fOldValue = p->m_fCurrValue;
			p->m_fCurrValue = p->m_Calc.Tick(p->m_fElapse);

			if (p->m_Calc.IsEnd())
			{
				p->m_fCurrValue = p->m_Calc.m_fStart + p->m_Calc.m_fDistance;
				p->m_pfnCallBack(p->m_fCurrValue, fOldValue, p->m_lParam);
				p->End();
			}
			else
			{
				p->m_pfnCallBack(p->m_fCurrValue, fOldValue, p->m_lParam);
			}
			return 0;
		}

		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
public:
	CEasingCurve()
	{
		m_Timer.SetMsg(m_uMsgTimer);
		m_Timer.SetParam((LPARAM)this);
	}

	~CEasingCurve()
	{
		End();
		const auto hWnd = m_Timer.GetHWND();
		if (IsWindow(hWnd))
		{
			UINT uRef = (UINT)GetPropW(hWnd, WPROP_EASING);
			if (uRef == 1)
			{
				RemoveWindowSubclass(hWnd, SubclassProc, SCID_EASING);
				RemovePropW(hWnd, WPROP_EASING);
			}
			else
			{
				--uRef;
				SetPropW(hWnd, WPROP_EASING, i32ToP<HANDLE>(uRef));
			}
		}
	}

	CEasingCurve& operator=(const CEasingCurve&) = delete;
	CEasingCurve(const CEasingCurve&) = delete;
	
	CEasingCurve(CEasingCurve&& x)
	{
		memcpy(this, &x, sizeof(CEasingCurve));
		std::construct_at(&x);
		CEasingCurve();
	}

	CEasingCurve& operator=(CEasingCurve&& x)
	{
		memcpy(this, &x, sizeof(CEasingCurve));
		std::construct_at(&x);
		CEasingCurve();
		return *this;
	}

	EckInline void SetParam(LPARAM lParam) { m_lParam = lParam; }

	EckInline LPARAM GetParam() const { return m_lParam; }

	EckInline void SetWnd(HWND hWnd)
	{
		EckAssert(!m_Timer.GetID());
		m_Timer.SetHWND(hWnd);
		UINT uRef = (UINT)GetPropW(hWnd, WPROP_EASING);
		if (!uRef)
			SetWindowSubclass(hWnd, SubclassProc, SCID_EASING, 0);
		++uRef;
		SetPropW(hWnd, WPROP_EASING, i32ToP<HANDLE>(uRef));
	}

	EckInline void SetCallBack(FCallBack pfnCallBack) { m_pfnCallBack = pfnCallBack; }

	void Begin(float fStart, float fDistance, float fDuration, float fElapse, UINT uFlags = 0u)
	{
		m_bActive = TRUE;
		if (uFlags & ECBF_SUPERPOSE_CURRPOS)
			m_Calc.m_fDistance = m_Calc.m_fDistance - m_fCurrValue + fDistance;
		else
		{
			m_Calc.Begin(fStart, fDistance, fDuration);
			m_fElapse = fElapse;
		}

		if (!m_Timer.GetID())
			m_Timer.SetTimer((UINT)m_fElapse);
	}

	EckInline void End()
	{
		m_Timer.KillTimer();
		m_bActive = FALSE;
	}

	EckInline BOOL IsActive() const { return m_bActive; }

	EckInline float GetCurrValue() const { return m_fCurrValue; }
};
template<class T>
inline UINT CEasingCurve<T>::m_uMsgTimer = RegisterWindowMessageW(MSGREG_EASING);
ECK_NAMESPACE_END