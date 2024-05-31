#include "voxel.h"
#include <raymath.h>
#include <stdint.h>

#define VXL_NUM_FACES  3
#define FACE_NUM_VERTS 4
#define TRI_NUM_VERTS  3
#define FACE_NUM_TRIS  2
#define TRI_NUM_IDXS   3

#define VERT(N, X, Y, Z) \
  mesh.vertices[N] = X; mesh.vertices[N+1] = Y; mesh.vertices[N+2] = Z
#define IDX(N, I)  mesh.indices[N] = I
#define VXL_ACCESS(x, y, z, XM, ZM) ((x) + ((z) * (XM)) + ((y) * (XM) * (ZM)))
#define VXL_EDGE(V, X, Y, Z) ((V.x == 0 || V.x >= X - 1) \
  || (V.y == 0 || V.y >= Y - 1) \
  || (V.z == 0 || V.z >= Z - 1))

static float VXL_VERT_LOOKUP[VXL_NUM_FACES][FACE_NUM_VERTS][TRI_NUM_VERTS] =
  {
    {{-0.5f, 0.0f, 0.5f}, {0.5f, 0.0f, 0.5f}, // front
     {0.5f, 1.0f, 0.5f}, {-0.5f, 1.0f, 0.5f}},
    {{0.5f, 0.0f, 0.5f}, {0.5f, 0.0f, -0.5f}, // right
     {0.5f, 1.0f, -0.5f}, {0.5f, 1.0f, 0.5f}},
    {{-0.5f, 1.0f, 0.5f}, {0.5f, 1.0f, 0.5f}, // top
     {0.5f, 1.0f, -0.5f}, {-0.5f, 1.0f, -0.5f}}
  };

static uint16_t VXL_IDX_LOOKUP[VXL_NUM_FACES][FACE_NUM_TRIS][TRI_NUM_IDXS] =
  {
    {{0, 1, 2},  {0, 2, 3}},   // front
    {{4, 5, 6},  {4, 6, 7}},   // right
    {{8, 9, 10}, {8, 10, 11}}  // top
  };

static Color VXL_CLR_LOOKUP[VXL_NUM_TYPES][VXL_NUM_FACES] =
  {
    /*VXL_EMPTY*/ {{0,0,0,0},{0,0,0,0},{0,0,0,0}},
    /*VXL_GRASS*/ {{51, 13, 18, 255}, {71, 47, 25, 255}, {27, 74, 23, 255}},
  };

Voxel voxel_new(vxl_t type, bool occ, Vector3 coord) {
  switch (type) {
  case VXL_GRASS: return (Voxel) { .type = type, .coord = coord, .occ = occ };
  default:        return (Voxel) { .type = type, .coord = coord, .occ = occ };
  }
}

VoxelScape voxel_gen_perlin_scape(int X, int Z, int seed, fade_fn fn) {
  VoxelScape vxl_scape = (VoxelScape){ .X = X, .Z = Z, .Y = MAX_HEIGHT};
  Voxel *vxls = MemAlloc(X * Z * MAX_HEIGHT * sizeof(Voxel));
  float scale = 8.0f;
  for (int z = 0; z < Z; z += SZ_VOXEL) {
    for (int x = 0; x < X; x += SZ_VOXEL) {
      float noise = perlin_noise((float) x/scale, (float) z/scale, seed, fn);
      noise = (noise + 1.0f) / 2.0f;
      size_t height = (int) (noise * MAX_HEIGHT);
      for (size_t lvl = 0; lvl < MAX_HEIGHT; lvl++) {
        if (lvl < height)
          vxls[VXL_ACCESS(x,lvl,z,X,Z)] =
            voxel_new(VXL_GRASS, false, (Vector3) {.x = x, .z = z, .y = lvl});
        else
          vxls[VXL_ACCESS(x,lvl,z,X,Z)] =
            voxel_new(VXL_EMPTY, true, (Vector3) {.x = x, .z = z, .y = lvl});
      }
    }
  }
  vxl_scape.vxls = vxls;
  return vxl_scape;
}

void voxel_cull_occluded(VoxelScape *vs) {
  for (int z = 0; z < vs->Z; z++) {
    for (int x = 0; x < vs->X; x++) {
      for (int y = 0; y < vs->Y; y++) {
        Voxel *v = &vs->vxls[VXL_ACCESS(x, y, z, vs->X, vs->Z)];
        if (v->type == VXL_EMPTY) { v->occ = true; continue; }
        if (VXL_EDGE((*v).coord, vs->X, vs->Y, vs->Z)) {
          v->occ = false;
          continue;
        }

        uint8_t occ = 0; // shift 1s for occluded faces

        Voxel *v_above = &vs->vxls[VXL_ACCESS(x, y + 1, z, vs->X, vs->Z)];
        Voxel *v_front = &vs->vxls[VXL_ACCESS(x + 1, y, z, vs->X, vs->Z)];
        Voxel *v_right = &vs->vxls[VXL_ACCESS(x, y, z + 1, vs->X, vs->Z)];

        if (v_above->type != VXL_EMPTY) occ |= (1 << 0);
        if (v_front->type != VXL_EMPTY) occ |= (1 << 1);
        if (v_right->type != VXL_EMPTY) occ |= (1 << 2);

        v->occ = (occ == 7); // 7 == 0b111
      }
    }
  }
}

Model voxel_terrain_model_from_scape(VoxelScape *vs) {
  int num_voxels = vs->X * vs->Y * vs->Z;
  int num_vertices = 0;
  int num_indices = 0;

  // compute number of verts/idxs needed
  for (int i = 0; i < num_voxels; i++) {
    Voxel v = vs->vxls[i];
    if (!v.occ) {
      num_vertices += FACE_NUM_VERTS * VXL_NUM_FACES;
      num_indices  += FACE_NUM_TRIS * TRI_NUM_IDXS * VXL_NUM_FACES;
    }
  }

  // create composite mesh
  Mesh mesh = { 0 };
  mesh.vertexCount = num_vertices;
  mesh.triangleCount = num_indices / 3;
  mesh.vertices = (float *) MemAlloc(num_vertices * 3 * sizeof(float));
  mesh.indices  = (uint16_t *) MemAlloc(num_indices * sizeof(uint16_t));
  mesh.colors   = (uint8_t *) MemAlloc(num_vertices * 4 * sizeof(uint8_t));

  // populate the mesh
  int vertex_idx = 0;
  int index_idx  = 0; // index index, lol
  for (int i = 0; i < num_voxels; i++) {
    Voxel v = vs->vxls[i];
    if (!v.occ) {
      Color *colors = VXL_CLR_LOOKUP[v.type];
      for (int face = 0; face < VXL_NUM_FACES; face++) {
        Color color = colors[face];
        for (int vert = 0; vert < FACE_NUM_VERTS; vert++) {
          int idx = vertex_idx * 3;
          mesh.vertices[idx] = v.coord.x     + VXL_VERT_LOOKUP[face][vert][0];
          mesh.vertices[idx + 1] = v.coord.y + VXL_VERT_LOOKUP[face][vert][1];
          mesh.vertices[idx + 2] = v.coord.z + VXL_VERT_LOOKUP[face][vert][2];

          idx = vertex_idx * 4;
          mesh.colors[idx] = color.r;
          mesh.colors[idx + 1] = color.g;
          mesh.colors[idx + 2] = color.b;
          mesh.colors[idx + 3] = color.a;

          vertex_idx++;
        }
        for (int tri = 0; tri < FACE_NUM_TRIS; tri++) {
          for (int idx = 0; idx < TRI_NUM_IDXS; idx++) {
            mesh.indices[index_idx++] = vertex_idx - 4
              + VXL_IDX_LOOKUP[face][tri][idx];
          }
        }
      }
    }
  }

  UploadMesh(&mesh, false);
  Model model = LoadModelFromMesh(mesh);
  return model;
}

/* DEPRECATED and/or EXPERIMENTAL */
#if 0
void draw_voxel_scape(VoxelScape *vs, Vector3 *wpos) {
  for (int n = 0; n < vs->X * vs->Z * vs->Y; n++) {
    Voxel vxl = vs->vxls[n];
    if (!vxl.occ) {
      DrawModel(*voxel_mdl_from_type(vxl.type),
                Vector3Add(*wpos, vxl.coord), SZ_VOXEL, WHITE);
    }
  }
}
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

/* DEPRECATED: for individual voxels -- TOO SLOW TO DRAW PER VOXEL */
static Model MDL_VOXEL_GRASS;

static Model *voxel_mdl_from_type(vxl_t type) {
  switch (type) {
  case VXL_GRASS: return &MDL_VOXEL_GRASS;
  default:        return &MDL_VOXEL_GRASS;
  }
}

static Mesh voxel_generate_mesh_from_colors(Color *colors) {
  Mesh mesh = { 0 };
  mesh.triangleCount = VXL_NUM_FACES * FACE_NUM_TRIS;
  mesh.vertexCount   = VXL_NUM_FACES * FACE_NUM_VERTS;

  mesh.vertices = (float *)    MemAlloc(3*mesh.vertexCount*sizeof(float));
  mesh.indices  = (uint16_t *) MemAlloc(3*mesh.triangleCount*sizeof(uint16_t));
  mesh.colors   = (uint8_t *)  MemAlloc(4*mesh.vertexCount*sizeof(uint8_t));

  /* NOTE:
     this could be refactorable from lookup table... but why?
     headache inducing. */

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
  MDL_VOXEL_GRASS = LoadModelFromMesh(voxel_generate_mesh_from_colors((Color [])
      {
        GetColor(0x332212FF),
        GetColor(0x472F19FF),
        GetColor(0x1B4A17FF),
      }));
}
#endif
