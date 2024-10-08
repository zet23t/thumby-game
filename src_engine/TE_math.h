#ifndef __TE_MATH_H__
#define __TE_MATH_H__

#include <math.h>
#include <inttypes.h>

#define TE_PI 3.14159265358979323846f

typedef struct TE_Vector2_u8
{
    uint8_t x, y;
} TE_Vector2_u8;

typedef struct TE_Vector2_s8
{
    int8_t x, y;
} TE_Vector2_s8;

typedef struct TE_Vector2_s16
{
    int16_t x, y;
} TE_Vector2_s16;

typedef struct TE_Vector2_f
{
    float x, y;
} TE_Vector2_f;

float fLerp(float a, float b, float t);
float fLerpClamped(float a, float b, float t);

float fMoveTowards(float current, float target, float maxDelta);

float fTweenElasticOut(float t);
int absi(int a);
int16_t max_s16(int16_t a, int16_t b);
int16_t min_s16(int16_t a, int16_t b);
float max_f(float a, float b);
float min_f(float a, float b);
float sign_f(float a);

#endif