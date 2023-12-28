#pragma once
#include "ECK.h"

#include <math.h>

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
struct FOutCircle
{
	float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * sqrt(1 - fCurrTime * fCurrTime) + fStart;
	}
};

struct FOutSine
{
	float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		return fDistance * sinf(fCurrTime / fDuration * PiF / 2.f) + fStart;
	}
};

struct FOutCubic
{
	float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		fCurrTime = fCurrTime / fDuration - 1.f;
		return fDistance * (fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
	}
};

struct FOutExpo
{
	float operator()(float fCurrTime, float fStart, float fDistance, float fDuration)
	{
		if (fCurrTime == fDuration)
			return fStart + fDistance;
		return fDistance * (-powf(-10.f * fCurrTime / fDuration, 2.f) + 1.f) + fStart;
	}
};

ECK_EASING_NAMESPACE_END
ECK_NAMESPACE_END