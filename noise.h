#ifndef NOISE_H_
#define NOISE_H_
#include <raylib.h>
#include <stdlib.h>
#include <math.h>

typedef float (*fade_fn)(float);

static inline float lerp(float t, float a, float b) { return a + t * (b - a); }
static inline float fd_perlin(float t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}
static inline float fd_linear(float t) { return t; }
static inline int hash(int x, int y, int seed) {
  srand(x + y * 57 + seed);
  return rand() % 100;
}

float perlin_noise(float, float, int, fade_fn);

#endif // NOISE_H_
