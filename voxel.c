#include "voxel.h"
#include <raymath.h>
#include <stdint.h>

#define VERT(N, V) mesh.vertices[N] = V
#define IDX(N, I)  mesh.indices[N] = I

/* NOTE:
   This does not contain any information about surface normals and therefore
   will not currently work for any light shading implementations. */
Mesh voxel_generate_min_mesh(void) {
  Mesh mesh = { 0 };
  mesh.triangleCount = 6;  // 3 faces, 2 triangles per face
  mesh.vertexCount = 7;    // 7 vertices for 3 faces

  mesh.vertices = (float *)    MemAlloc(3*mesh.vertexCount*sizeof(float));
  mesh.indices  = (uint16_t *) MemAlloc(3*mesh.triangleCount*sizeof(uint16_t));
  mesh.colors   = (uint8_t *)  MemAlloc(4*mesh.vertexCount*sizeof(uint8_t));

  // Define the vertices
  VERT(0, -0.5f); VERT(1,  1.0f); VERT(2,  0.5f);
  VERT(3, -0.5f); VERT(4,  0.0f); VERT(5,  0.5f);
  VERT(6,  0.5f); VERT(7,  0.0f); VERT(8,  0.5f);
  VERT(9,  0.5f); VERT(10, 1.0f); VERT(11, 0.5f);
  VERT(12, 0.5f); VERT(13, 0.0f); VERT(14,-0.5f);
  VERT(15, 0.5f); VERT(16, 1.0f); VERT(17,-0.5f);
  VERT(18,-0.5f); VERT(19, 1.0f); VERT(20,-0.5f);

  // Define the indices
  IDX(0, 0);  IDX(1, 1);  IDX(2, 2);
  IDX(3, 0);  IDX(4, 2);  IDX(5, 3);
  IDX(6, 2);  IDX(7, 4);  IDX(8, 5);
  IDX(9, 2);  IDX(10, 5); IDX(11, 3);
  IDX(12, 0); IDX(13, 3); IDX(14, 5);
  IDX(15, 0); IDX(16, 5); IDX(17, 6);

  // Set the color of each vertex to green
  for (int i = 0; i < mesh.vertexCount * 4; i += 4) {
    mesh.colors[i] = 0;
    mesh.colors[i + 1] = 255;
    mesh.colors[i + 2] = 0;
    mesh.colors[i + 3] = 255;
  }

  UploadMesh(&mesh, false);
  return mesh;
}

Mesh voxel_generate_full_mesh_no_normals(void) {
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
