#ifndef TEXTURE_H_
#define TEXTURE_H_
#include <raylib.h>
#include <stdio.h>

struct atlas {
  Texture2D texture;
  size_t x, y;
  size_t sprite_sz;
};

struct atlas atlas_load(const char *, size_t);
Rectangle atlas_get_sprite(struct atlas, int, int);

/* static void _atlas_draw_sprite(struct atlas a, int x, int y, Vector2 pos) { */
/*   DrawTextureRec(a.texture, atlas_get_sprite(a, x, y), pos, WHITE); */
/* } */

#define SPRITE_GRASS_TOP  0,0
#define SPRITE_GRASS_SIDE 1,0
#define SPRITE_WATER_SIDE 2,0
#define SPRITE_SAND_SIDE 3,0

#define ATLAS_DRAW_SPRITE(S, P) _atlas_draw_sprite(atlas, S, P)


#endif // TEXTURE_H_
