#ifndef VOXEL_H_
#define VOXEL_H_
#include <raylib.h>
#include <string.h>
#include "noise.h"

#define SZ_VOXEL 1
#define MAX_HEIGHT 10

typedef enum {
  VXL_EMPTY,
  VXL_GRASS,
} vxl_t;

typedef struct { vxl_t type; Vector3 coord; } Voxel;
typedef struct { Voxel *vxls; int X, Y, Z; } VoxelScape;

void VOXEL_MODELS_INIT(void);
Voxel voxel_new(vxl_t, Vector3);
VoxelScape voxel_gen_noise_perlin(int, int, int, fade_fn);
void draw_voxel_scape(VoxelScape *, Vector3 *);

#endif //VOXEL_H_
