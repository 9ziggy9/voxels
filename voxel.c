#include "voxel.h"
#include <raymath.h>
#include <stdint.h>

#define VERT(N, X, Y, Z) \
  mesh.vertices[N] = X; mesh.vertices[N+1] = Y; mesh.vertices[N+2] = Z
#define IDX(N, I)  mesh.indices[N] = I

static Model MDL_VOXEL_GRASS;

static Mesh voxel_generate_mesh_from_colors(Color *colors) {
  Mesh mesh = { 0 };
  mesh.triangleCount = 6;  // 3 faces, 2 triangles per face
  mesh.vertexCount = 12;   // 4 vertices per face, 3 faces

  mesh.vertices = (float *)    MemAlloc(3*mesh.vertexCount*sizeof(float));
  mesh.indices  = (uint16_t *) MemAlloc(3*mesh.triangleCount*sizeof(uint16_t));
  mesh.colors   = (uint8_t *)  MemAlloc(4*mesh.vertexCount*sizeof(uint8_t));

  // four to a face
  VERT(0, -0.5f, 0.0f, 0.5f);  VERT(3, 0.5f, 0.0f, 0.5f);
  VERT(6, 0.5f, 1.0f, 0.5f);   VERT(9, -0.5f, 1.0f, 0.5f);
  VERT(12, 0.5f, 0.0f, 0.5f);  VERT(15, 0.5f, 0.0f, -0.5f);
  VERT(18, 0.5f, 1.0f, -0.5f); VERT(21, 0.5f, 1.0f, 0.5f);
  VERT(24, -0.5f, 1.0f, 0.5f); VERT(27, 0.5f, 1.0f, 0.5f);
  VERT(30, 0.5f, 1.0f, -0.5f); VERT(33, -0.5f, 1.0f, -0.5f);

  // six to a face
  IDX(0, 0);  IDX(1, 1);   IDX(2, 2);
  IDX(3, 0);  IDX(4, 2);   IDX(5, 3);
  IDX(6, 4);  IDX(7, 5);   IDX(8, 6);
  IDX(9, 4);  IDX(10, 6);  IDX(11, 7);
  IDX(12, 8); IDX(13, 9);  IDX(14, 10);
  IDX(15, 8); IDX(16, 10); IDX(17, 11);

  for (int face = 0; face < 3; face++) {
    Color color = colors[face];
    for (int vert = 0; vert < 4; vert++) {
      int idx = face * 4 + vert;
      mesh.colors[idx * 4]     = color.r;
      mesh.colors[idx * 4 + 1] = color.g;
      mesh.colors[idx * 4 + 2] = color.b;
      mesh.colors[idx * 4 + 3] = color.a;
    }
  }

  UploadMesh(&mesh, false);
  return mesh;
}

void VOXEL_MODELS_INIT(void) {
  MDL_VOXEL_GRASS = LoadModelFromMesh(voxel_generate_mesh_from_colors((Color []){
        GetColor(0x332212FF),
        GetColor(0x472F19FF),
        GetColor(0x1B4A17FF),
      }));
}

Voxel voxel_new(vxl_t type, Vector3 coord) {
  switch (type) {
  case VXL_GRASS: return (Voxel) { .type = type, .coord = coord };
  default:        return (Voxel) { .type = type, .coord = coord };
  }
}

VoxelScape voxel_gen_noise_perlin(int X, int Z, int seed, fade_fn fn) {
  VoxelScape vxl_scape = (VoxelScape){ .X = X, .Z = Z, .Y = MAX_HEIGHT};
  Voxel *map = MemAlloc(X * Z * MAX_HEIGHT * sizeof(Voxel));
  float scale = 8.0f;
  for (int z = 0; z < Z; z += SZ_VOXEL) {
    for (int x = 0; x < X; x += SZ_VOXEL) {
      float noise = perlin_noise((float) x / scale,
                                 (float) z / scale,
                                 seed, fn);
      noise = (noise + 1.0f) / 2.0f; // map [-1, 1] -> [0, 1]
      size_t height = (int) (noise * MAX_HEIGHT);
      for (size_t lvl = 0; lvl < height; lvl++) {
        size_t idx = (z * X * MAX_HEIGHT) + (x * MAX_HEIGHT) + lvl;
        map[idx] = voxel_new(VXL_GRASS, (Vector3) { .x = x, .z = z, .y = lvl });
      }
      for (size_t lvl = height; lvl < MAX_HEIGHT; lvl++) {
        size_t idx = (z * X * MAX_HEIGHT) + (x * MAX_HEIGHT) + lvl;
        map[idx] = voxel_new(VXL_EMPTY, (Vector3) { .x = x, .z = z, .y = lvl });
      }
    }
  }
  vxl_scape.vxls = map;
  return vxl_scape;
}

static Model *voxel_mdl_from_type(vxl_t type) {
  switch (type) {
  case VXL_GRASS: return &MDL_VOXEL_GRASS;
  default:        return &MDL_VOXEL_GRASS;
  }
}

void draw_voxel_scape(VoxelScape *scape, Vector3 *wpos) {
  for (int n = 0; n < scape->X * scape->Z * scape->Y; n++) {
    Voxel vxl = scape->vxls[n];
    if (vxl.type != VXL_EMPTY) {
      DrawModel(*voxel_mdl_from_type(vxl.type),
                Vector3Add(*wpos, vxl.coord), SZ_VOXEL, WHITE);
    }
  }
}

#if 0 // experimental purposes
Mesh _voxel_generate_full_mesh_no_normals(void) {
  Mesh mesh = { 0 };
  mesh.vertexCount = 8;    // 8 vertices for a cube
  mesh.triangleCount = 12; // 6 faces, 2 triangles per face

  mesh.vertices = (float *)    MemAlloc(mesh.vertexCount * 3 * sizeof(float));
  mesh.indices  = (uint16_t *) MemAlloc(mesh.triangleCount * 3 *
                                       sizeof(uint16_t));
  mesh.colors   = (uint8_t *)  MemAlloc(mesh.vertexCount * 4 * sizeof(uint8_t));

  mesh.vertices[0] = -0.5f; mesh.vertices[1] = 0.0f; mesh.vertices[2] = -0.5f;
  mesh.vertices[3] =  0.5f; mesh.vertices[4] = 0.0f; mesh.vertices[5] = -0.5f;
  mesh.vertices[6] =  0.5f; mesh.vertices[7] = 1.0f; mesh.vertices[8] = -0.5f;
  mesh.vertices[9] = -0.5f; mesh.vertices[10] = 1.0f; mesh.vertices[11] = -0.5f;
  mesh.vertices[12] = -0.5f; mesh.vertices[13] = 0.0f; mesh.vertices[14] = 0.5f;
  mesh.vertices[15] =  0.5f; mesh.vertices[16] = 0.0f; mesh.vertices[17] = 0.5f;
  mesh.vertices[18] =  0.5f; mesh.vertices[19] = 1.0f; mesh.vertices[20] = 0.5f;
  mesh.vertices[21] = -0.5f; mesh.vertices[22] = 1.0f; mesh.vertices[23] = 0.5f;

  mesh.indices[0] = 0; mesh.indices[1] = 1; mesh.indices[2] = 2;
  mesh.indices[3] = 2; mesh.indices[4] = 3; mesh.indices[5] = 0;
  mesh.indices[6] = 1; mesh.indices[7] = 5; mesh.indices[8] = 6;
  mesh.indices[9] = 6; mesh.indices[10] = 2; mesh.indices[11] = 1;
  mesh.indices[12] = 5; mesh.indices[13] = 4; mesh.indices[14] = 7;
  mesh.indices[15] = 7; mesh.indices[16] = 6; mesh.indices[17] = 5;
  mesh.indices[18] = 4; mesh.indices[19] = 0; mesh.indices[20] = 3;
  mesh.indices[21] = 3; mesh.indices[22] = 7; mesh.indices[23] = 4;
  mesh.indices[24] = 4; mesh.indices[25] = 5; mesh.indices[26] = 1;
  mesh.indices[27] = 1; mesh.indices[28] = 0; mesh.indices[29] = 4;
  mesh.indices[30] = 7; mesh.indices[31] = 3; mesh.indices[32] = 2;
  mesh.indices[33] = 2; mesh.indices[34] = 6; mesh.indices[35] = 7;

  for (int i = 0; i < mesh.vertexCount * 4; i += 4) {
    mesh.colors[i] = 0;
    mesh.colors[i + 1] = 255;
    mesh.colors[i + 2] = 0;
    mesh.colors[i + 3] = 255;
  }

  UploadMesh(&mesh, false);
  return mesh;
}
#endif
