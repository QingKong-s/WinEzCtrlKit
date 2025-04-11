#pragma once
#include "ITimeLine.h"

#define ECK_EASING_NAMESPACE_BEGIN namespace Easing {
#define ECK_EASING_NAMESPACE_END }

ECK_NAMESPACE_BEGIN
ECK_EASING_NAMESPACE_BEGIN
using FAn = float(*)(float fCurrTime, float fStart, float fDistance, float fDuration);

struct FInCircle
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return -fDistance * (sqrt(1.f - fCurrTime * fCurrTime) - 1.f) + fStart;
	}
};

EckInline float InCircle(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInCircle{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutCircle
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * sqrt(1 - fCurrTime * fCurrTime) + fStart;
	}
};

EckInline float OutCircle(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutCircle{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutCircle(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutCircle{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return -fDistance * cosf(fCurrTime / fDuration * PiF / 2.f) + fDistance + fStart;
	}
};

EckInline float InSine(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInSine{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * sinf(fCurrTime / fDuration * PiF / 2.f) + fStart;
	}
};

EckInline float OutSine(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutSine{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInOutSine
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return -fDistance / 2.f * (cosf(PiF * fCurrTime / fDuration) - 1.f) + fStart;
	}
};

EckInline float InOutSine(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutSine{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInQuad
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime + fStart;
	}
};

EckInline float InQuad(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInQuad{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutQuad
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return -fDistance * fCurrTime * (fCurrTime - 2.f) + fStart;
	}
};

EckInline float OutQuad(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutQuad{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutQuad(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutQuad{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInCubic
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

EckInline float InCubic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInCubic{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutCubic
{
	EckInline constexpr float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}
};

EckInline float OutCubic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutCubic{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutCubic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutCubic{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInQuart
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

EckInline float InQuart(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInQuart{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutQuart
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return -fDistance * (fCurrTime * fCurrTime * fCurrTime * fCurrTime - 1.f) + fStart;
	}
};

EckInline float OutQuart(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutQuart{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutQuart(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutQuart{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInQuint
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + fStart;
	}
};

EckInline float InQuint(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInQuint{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutQuint
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}
};

EckInline float OutQuint(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutQuint{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutQuint(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutQuint{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InExpo(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInExpo{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float OutExpo(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutExpo{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutExpo(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutExpo{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InElastic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInElastic{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float OutElastic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutElastic{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutElastic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutElastic{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInBack
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 1.70158f;
		fCurrTime /= fDuration;
		return fDistance * fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime - f) + fStart;
	}
};

EckInline float InBack(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInBack{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FOutBack
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		const float f = 1.70158f;
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * ((f + 1.f) * fCurrTime + f) + 1.f) + fStart;
	}
};

EckInline float OutBack(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutBack{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutBack(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutBack{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float OutBounce(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FOutBounce{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FInBounce
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance - FOutBounce{}(fDuration - fCurrTime, 0.f, fDistance, fDuration) + fStart;
	}
};

EckInline float InBounce(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInBounce{}(fCurrTime, fStart, fDistance, fDuration);
}

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

EckInline float InOutBounce(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FInOutBounce{}(fCurrTime, fStart, fDistance, fDuration);
}

struct FLinear
{
	EckInline float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * fCurrTime / fDuration + fStart;
	}
};

EckInline float Linear(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FLinear{}(fCurrTime, fStart, fDistance, fDuration);
}

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
			const float f1 = fCurrTime / fDuration - 1.f;
			return fDistance * expf(-f1 * f) * sinf(6.f * PiF * f1) + fDistance + fStart;
		}
	}
};

EckInline float Punch(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FPunch{}(fCurrTime, fStart, fDistance, fDuration);
}

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
			const float g = fCurrTime / fDuration - 1.f;
			return -fDistance * expf(-g * f) * sinf(2.f * PiF * g / 0.3f) + fDistance + fStart;
		}
	}
};

EckInline float Spring(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return FSpring{}(fCurrTime, fStart, fDistance, fDuration);
}
ECK_EASING_NAMESPACE_END

enum
{
	ECBF_DEF = 0,
	ECBF_CONTINUE = 1u << 1,
};

class CEasingCurve :public CUnknownSingleThread<CEasingCurve, ITimeLine>
{
	ECK_DECL_CUNK_FRIENDS;
public:
	using FCallBack = void(*)(float fCurrValue, float fOldValue, LPARAM lParam);
private:
	float m_fCurrTime{};
	float m_fDuration{};
	float m_fStart{};
	float m_fDistance{};

	float m_fCurrValue{};

	int m_iCurrInterval{};

	LPARAM m_lParam{};
	FCallBack m_pfnCallBack{};
	Easing::FAn m_pfnAn{ Easing::Linear };

	BOOLEAN m_bActive{};
	BOOLEAN m_bReverse{};
	LONG m_cRef{ 1 };

	EckInline BOOL IntTick(float fMs)
	{
		m_fCurrTime += fMs;
		BOOL bEnd = FALSE;
		if (m_fCurrTime > m_fDuration)
		{
			EckAssert(fMs > 0.f);
			m_fCurrTime = m_fDuration;
			bEnd = TRUE;
		}
		else if (m_fCurrTime < 0.f)
		{
			EckAssert(fMs < 0.f);
			m_fCurrTime = 0.f;
			bEnd = TRUE;
		}

		m_fCurrValue = m_pfnAn(m_fCurrTime, m_fStart, m_fDistance, m_fDuration);
		return bEnd;
	}
public:
	ECK_DISABLE_COPY_MOVE_DEF_CONS(CEasingCurve)
public:
	~CEasingCurve()
	{
		End();
	}
	// **IUnknown**
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject)
	{
		const static QITAB qit[]
		{
			QITABENT(CEasingCurve, ITimeLine),
			{}
		};

		return QISearch(this, qit, iid, ppvObject);
	}
	// **ITimeLine**
	void STDMETHODCALLTYPE Tick(int iMs)
	{
		m_iCurrInterval = iMs;
		EckAssert(m_pfnCallBack);
		const float fOldValue = m_fCurrValue;
		const auto bEnd = IntTick((float)(m_bReverse ? -iMs : iMs));
		m_pfnCallBack(m_fCurrValue, fOldValue, m_lParam);

		if (bEnd)
			End();
	}

	EckInline BOOL STDMETHODCALLTYPE IsValid()
	{
		return m_bActive;
	}

	EckInline int STDMETHODCALLTYPE GetCurrTickInterval()
	{
		return m_iCurrInterval;
	}
	// 
	EckInline void SetParam(LPARAM lParam) { m_lParam = lParam; }

	EckInline LPARAM GetParam() const { return m_lParam; }

	EckInline void SetCallBack(FCallBack pfnCallBack) { m_pfnCallBack = pfnCallBack; }

	EckInline void SetDuration(float fDuration) { m_fDuration = fDuration; }

	EckInline void SetRange(float fStart, float fDistance)
	{
		m_fStart = fStart;
		m_fDistance = fDistance;
	}

	EckInline void SetCurrTime(float fCurrTime)
	{ 
		m_fCurrTime = fCurrTime;
	}

	EckInline void SetReverse(BOOL bReverse)
	{
		m_bReverse = bReverse;
		if (!IsActive())
			if (bReverse)
				m_fCurrValue = m_fStart + m_fDistance;
			else
				m_fCurrValue = m_fStart;
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="uFlags">ECBF_</param>
	void Begin(UINT uFlags = 0u)
	{
		m_bActive = TRUE;
		if (!(uFlags & ECBF_CONTINUE))
		{
			SetCurrTime(0.f);
			m_fCurrValue = m_fStart;
		}
	}

	EckInline void End()
	{
		m_bActive = FALSE;
		m_fCurrValue = m_fStart + m_fDistance;
	}

	EckInline BOOL IsActive() const { return m_bActive; }

	EckInline float GetCurrValue() const { return m_fCurrValue; }

	EckInline float GetCurrTime() const { return m_fCurrTime; }

	EckInline float GetDuration() const { return m_fDuration; }

	EckInline float GetStart() const { return m_fStart; }

	EckInline float GetDistance() const { return m_fDistance; }

	EckInline BOOL GetReverse() const { return m_bReverse; }

	EckInline void SetAnProc(Easing::FAn pfnAn) { m_pfnAn = pfnAn; }
};

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
		if (IsEnd())
			m_fCurrTime = m_fDuration;
		return FAn{}(m_fCurrTime, m_fStart, m_fDistance, m_fDuration);
	}

	EckInline BOOL IsEnd()
	{
		return m_fCurrTime >= m_fDuration;
	}
};
ECK_NAMESPACE_END