#include "voxel.h"
#define VOXEL_VERTS_PER_FACE 4
#define VOXEL_NUM_FACES      6

static Mesh voxel_generate_mesh(void) {
  Mesh mesh = { 0 };

  mesh.vertexCount   = VOXEL_NUM_FACES * VOXEL_VERTS_PER_FACE;
  mesh.triangleCount = 12;

  float vertices[] = {
    -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, // front
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, // back
     0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, // top
     0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, // bottom
     0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, //right
     0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, // left
    -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f,
  };
  float texcoords[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // front
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // back
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // top
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // bottom
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, //right
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, // left
  };
  float normals[] = {
     0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // front
     0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f, // back
     0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,
     0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // top
     0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, // bottom
     0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,
     1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // right
     1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
    -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // left
    -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
  };
  unsigned short indices[] = {
     0 ,  1 ,  2 ,  0 ,  2 ,  3 , // front
     4 ,  5 ,  6 ,  4 ,  6 ,  7 , // back
     8 ,  9 , 10 ,  8 , 10 , 11 , // top
    12 , 13 , 14 , 12 , 14 , 15 , // bottom
    16 , 17 , 18 , 16 , 18 , 19 , // right
    20 , 21 , 22 , 20 , 22 , 23   // left
  };

  mesh.vertices  = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
  mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
  mesh.normals   = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
  mesh.indices   = (unsigned short*)MemAlloc(mesh.triangleCount * 3 *
                                             sizeof(unsigned short));

  memcpy(mesh.vertices, vertices, mesh.vertexCount * 3 * sizeof(float));
  memcpy(mesh.texcoords, texcoords, mesh.vertexCount * 2 * sizeof(float));
  memcpy(mesh.normals, normals, mesh.vertexCount * 3 * sizeof(float));
  memcpy(mesh.indices, indices, mesh.triangleCount * 3 * sizeof(unsigned short));

  UploadMesh(&mesh, true);
  return mesh;
}

static float *voxel_get_tex_coords(Rectangle rect) {
  float *texcoords = (float *) MemAlloc(VOXEL_VERTS_PER_FACE * 2 *
                                        sizeof(float));
  texcoords[0] = rect.x;
  texcoords[1] = rect.y;
  texcoords[2] = rect.x + rect.width;
  texcoords[3] = rect.y;
  texcoords[4] = rect.x + rect.width;
  texcoords[5] = rect.y + rect.height;
  texcoords[6] = rect.x;
  texcoords[7] = rect.y + rect.height;
  return texcoords;
}

static void voxel_set_tex_coords(Mesh *mesh, Rectangle *rects) {
  float *texcoords = (float *) MemAlloc(mesh->vertexCount * 2 * sizeof(float));

  for (int i = 0; i < VOXEL_NUM_FACES; i++) {
    float *face_texcoords = voxel_get_tex_coords(rects[i]);
    for (int j = 0; j < VOXEL_VERTS_PER_FACE * 2; j++) {
      texcoords[i * VOXEL_VERTS_PER_FACE * 2 + j] = face_texcoords[j];
    }
    MemFree(face_texcoords);
  }
  mesh->texcoords = texcoords;
}

Model voxel_model_from_atlas(Texture2D atlas, Rectangle *rects) {
  Mesh mesh = voxel_generate_mesh();
  voxel_set_tex_coords(&mesh, rects);
  Model model = LoadModelFromMesh(mesh);
  model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = atlas;
  return model;
}
