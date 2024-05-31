#ifndef VOXEL_H_
#define VOXEL_H_
#include <raylib.h>
#include <string.h>
#include "noise.h"

#define SZ_VOXEL 1
#define MAX_HEIGHT 10

#define VXL_NUM_TYPES 2
typedef enum {
  VXL_EMPTY,
  VXL_GRASS,
} vxl_t;

typedef struct {
  vxl_t type;
  Vector3 coord;
  bool occ;
} Voxel;

typedef struct { Voxel *vxls; int X, Y, Z; } VoxelScape;

void VOXEL_MODELS_INIT(void);
Voxel voxel_new(vxl_t, bool, Vector3);
VoxelScape voxel_gen_perlin_scape(int, int, int, fade_fn);
void voxel_cull_occluded(VoxelScape *);
Model voxel_terrain_model_from_scape(VoxelScape *);

#endif //VOXEL_H_
