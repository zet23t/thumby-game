#ifndef __TE_MATH_H__
#define __TE_MATH_H__

#include <math.h>
#include <inttypes.h>

#define TE_PI 3.14159265358979323846f

float fLerp(float a, float b, float t);
float fLerpClamped(float a, float b, float t);

float fMoveTowards(float current, float target, float maxDelta);

float fTweenElasticOut(float t);

#endif