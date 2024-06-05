#include "noise.h"

static float grad(int hash, float x, float y) {
  float u, v;
  switch (hash % 4) {
    case 0:  u =  x; v =  y; break;
    case 1:  u = -x; v =  y; break;
    case 2:  u =  x; v = -y; break;
    default: u = -x; v = -y; break;
  }
  return u + v;
}

static float perlin_noise(float x, float y, int seed, fade_fn fn) {
  int x0 = (int) floorf(x);
  int x1 = x0 + 1;
  int y0 = (int) floorf(y);
  int y1 = y0 + 1;

  float sx = x - (float) x0;
  float sy = y - (float) y0;

  int hash00 = hash(x0, y0, seed);
  int hash01 = hash(x0, y1, seed);
  int hash10 = hash(x1, y0, seed);
  int hash11 = hash(x1, y1, seed);

  float n0 = grad(hash00, sx,     sy);
  float n1 = grad(hash10, sx - 1, sy);
  float n2 = grad(hash01, sx,     sy - 1);
  float n3 = grad(hash11, sx - 1, sy - 1);

  float n_x0y0 = lerp(fn(sx), n0, n1);
  float n_x1y0 = lerp(fn(sx), n2, n3);

  return lerp(fn(sy), n_x0y0, n_x1y0);
}

float perlin_compose(float x, float y, struct perlin_spec spec) {
  float sum  = 0;
  float freq = 1.0f;
  float amp  = 1.0f;
  float max  = 0;
  while (spec.octaves-- >= 0) {
    sum += amp * perlin_noise(x * freq / spec.scale,
                              y * freq / spec.scale,
                              spec.seed, spec.fn);
    max  += amp;
    amp  *= spec.persistance;
    freq *= spec.lacunarity;
  }
  return sum / max;
}
