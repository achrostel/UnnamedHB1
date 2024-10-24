#ifndef PMATH_H
#define PMATH_H

#include <stdint.h>
#include <psxgte.h>

int16_t sin_t(int16_t a);

int16_t cos_t(int16_t a);

// this one looks cursed to me, saves some cycles over running seperatly
uint32_t sincos_t(int16_t a);

int32_t patan(long x, long y);

void RotMat(SVECTOR* r, MATRIX* m);

#endif // PMATH_H