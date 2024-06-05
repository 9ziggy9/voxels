#include "texture.h"

struct atlas atlas_load(const char *fp, size_t sprite_sz) {
  Texture2D texture = LoadTexture(fp);
  return (struct atlas) {
    .texture = texture,
    .x = texture.width / sprite_sz,
    .y = texture.height/ sprite_sz,
    .sprite_sz = sprite_sz
  };
}

Rectangle atlas_get_sprite(struct atlas a, int x, int y) {
  return (Rectangle) {
    .x = x * a.sprite_sz, .y = y * a.sprite_sz,
    .width = a.sprite_sz, .height = a.sprite_sz
  };
}
