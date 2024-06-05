#ifndef VOXEL_H_
#define VOXEL_H_
#include <raylib.h>
#include <string.h>
#include <stdint.h>
#include "noise.h"
#include "texture.h"

#define SZ_VOXEL 1
#define CHUNK_X  16
#define CHUNK_Z  16
#define CHUNK_Y  10

#define LAST_X_CHUNK 20
#define LAST_Z_CHUNK 20

#define VXL_NUM_FACES  3
#define FACE_NUM_VERTS 4
#define TRI_NUM_VERTS  3
#define FACE_NUM_TRIS  2
#define TRI_NUM_IDXS   3

#define VXL_NUM_TYPES 3
typedef enum {
  VXL_EMPTY,
  VXL_GRASS,
  VXL_WATER,
} vxl_t;

typedef struct {
  vxl_t type;
  Vector3 coord;
  bool occ;
} Voxel;

typedef struct { Voxel *vxls; int X, Y, Z; } VoxelScape;

typedef struct { Model *views; uint32_t count; } TerrainView;

VoxelScape voxel_gen_perlin_scape(int, int, int, int, fade_fn);
void voxel_destroy_scape(VoxelScape *);
void voxel_cull_occluded(VoxelScape *);

Mesh voxel_terrain_mesh_from_region(VoxelScape *,
                                    struct atlas,
                                    int, int, int, int);
Model voxel_terrain_model_from_region(VoxelScape *, Shader,
                                      struct atlas,
                                      int, int, int, int);
TerrainView voxel_load_terrain_models(VoxelScape *, Shader, struct atlas);
void voxel_unload_terrain_models(TerrainView *tv);

#endif //VOXEL_H_
