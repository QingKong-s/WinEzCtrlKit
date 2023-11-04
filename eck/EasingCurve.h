#pragma once
#include "ECK.h"

#include <math.h>
#include <numbers>

ECK_NAMESPACE_BEGIN
EckInline float OutCircle(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	fCurrTime = fCurrTime / fDuration - 1.f;
	return fDistance * sqrt(1 - fCurrTime * fCurrTime) + fStart;
}

EckInline float OutSine(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	return fDistance * sinf(fCurrTime / fDuration * std::numbers::pi_v<float> / 2.f) + fStart;
}

EckInline float OutCubic(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	fCurrTime = fCurrTime / fDuration - 1.f;
	return fDistance * (fCurrTime * fCurrTime * fCurrTime + 1.f) + fStart;
}

EckInline float OutExpo(float fCurrTime, float fStart, float fDistance, float fDuration)
{
	if (fCurrTime == fDuration)
		return fStart + fDistance;
	return fDistance * (-powf(-10.f * fCurrTime / fDuration, 2.f) + 1.f) + fStart;
}
ECK_NAMESPACE_END