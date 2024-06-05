#ifndef NOISE_H_
#define NOISE_H_
#include <raylib.h>
#include <stdlib.h>
#include <math.h>

typedef float (*fade_fn)(float);

struct perlin_spec {
  int seed;
  fade_fn fn;
  int octaves;
  float lacunarity;
  float persistance;
  float scale;
};

static inline float lerp(float t, float a, float b) { return a + t * (b - a); }

static inline float fd_perlin(float t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}

static inline float fd_linear(float t) { return t; }

static inline int hash(int x, int y, int seed) {
  srand(x + y * 57 + seed);
  return rand() % 100;
}

float perlin_compose(float x, float y, struct perlin_spec spec);

#endif // NOISE_H_
