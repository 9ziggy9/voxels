#include <stdio.h>

#include "config.h"

#define MEM_MAX_CAP (1024 * 1024 * 500)
#define LOAD_PROC_INFO
#include "proc.h"
#include "camera.h"
#include "voxel.h"
#include "world.h"

#include "texture.h"

typedef enum {
  SCENE, TEXTURE
} menu_t;

void exit_clean_procedure(int, void *);
void poll_key_presses(CamView *, Vector3 *, menu_t *);
void poll_mouse_movement(Camera3D *);

#define SZ_CHECKER 2
Model mdl_gen_checkerboard(Color, Color);

#define VXL_SCALE 10.0f
Mesh voxel_generate_mesh_from_colors(Color *colors);

int main(void) {
  // proc init
  PROC_INFO_BOOTSTRAP();
  PROC_INFO_CAP_MEM(MEM_MAX_CAP);

  // window init
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_NULL);

  Vector3 player_position = {0};
  Vector3 world_position = WORLD_ORIGIN;

  CamView cam = {
    .scene      = cam_init_scene(&player_position),
    .sun        = cam_init_sun(),
    .light_cone = 160.0f,
  };
  cam.current = &cam.scene;

  Model MDL_CB = mdl_gen_checkerboard(DARKGRAY, GRAY);
  Model MDL_VOXEL_GRASS =
    LoadModelFromMesh(voxel_generate_mesh_from_colors((Color [])
      {
        GetColor(0x332212FF),
        GetColor(0x472F19FF),
        GetColor(0x1B4A17FF),
      }));

  static menu_t menu = TEXTURE;
  struct atlas atlas = atlas_load("./assets/blockatlas.png", 16);

  while(!WindowShouldClose()) {
    poll_key_presses(&cam, &world_position, &menu);
    poll_mouse_movement(cam.current);
    BeginDrawing();
      switch (menu) {
      case SCENE: {
        BeginMode3D(*cam.current);
          ClearBackground(BLACK);
          DrawModel(MDL_CB, world_position, 1.0f, WHITE);
          DrawModel(MDL_VOXEL_GRASS, world_position, VXL_SCALE, WHITE);
        EndMode3D();
      } break;
      case TEXTURE: {
        ClearBackground(DARKGREEN);
        ATLAS_DRAW_SPRITE(SPRITE_GRASS_SIDE,
                          ((Vector2) {(float) 0.5f * SCREEN_WIDTH,
                                      (float) 0.5f * SCREEN_HEIGHT}));
      } break;
      default: break;
      }
      PROC_INFO_DRAW(PROC_INFO_FLAG_ALL);
    EndDrawing();
  }

  UnloadModel(MDL_CB);
  UnloadModel(MDL_VOXEL_GRASS);
  CloseWindow();
  return EXIT_SUCCESS;
}

Mesh voxel_generate_mesh_from_colors(Color *colors) {
#define VERT(N, X, Y, Z) \
  mesh.vertices[N] = X; mesh.vertices[N+1] = Y; mesh.vertices[N+2] = Z
#define IDX(N, I)  mesh.indices[N] = I

  Mesh mesh = { 0 };
  mesh.triangleCount = VXL_NUM_FACES * FACE_NUM_TRIS;
  mesh.vertexCount   = VXL_NUM_FACES * FACE_NUM_VERTS;

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

void exit_clean_procedure(int code, void *resources) {
  (void) resources;
  CloseWindow();
  switch (code) {
  case EXIT_SUCCESS: fprintf(stdout, "[EXIT] clean exit. :D\n"); break;
  case EXIT_FAILURE: fprintf(stderr, "[EXIT] bad exit! >.<\n");  break;
  }
  exit(code);
}

Model mdl_gen_checkerboard(Color c1, Color c2) {
  Image img_checked = GenImageChecked(LAST_X_CHUNK * CHUNK_X,
                                      LAST_Z_CHUNK * CHUNK_Z,
                                      SZ_CHECKER, SZ_CHECKER,
                                      c1, c2);

  Texture2D txtr_cb = LoadTextureFromImage(img_checked);
  UnloadImage(img_checked);

  Model mdl_cb = LoadModelFromMesh(GenMeshPlane(LAST_X_CHUNK * CHUNK_X,
                                                LAST_Z_CHUNK * CHUNK_Z, 1, 1));
  mdl_cb.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = txtr_cb;
  return mdl_cb;
}

void poll_mouse_movement(Camera3D *cam) {
#define ZOOM_RATE (5.0f)
  cam->fovy -= GetMouseWheelMove() * ZOOM_RATE;
  cam->fovy = Clamp(cam->fovy, 2.0f, 180.0f);
}

#define CLOSE_WITH(CODE, MSG)                     \
  do {                                            \
    fprintf(stdout, "[INFO] %s() :: ", __func__); \
    fprintf(stdout, "%s\n", MSG);                 \
    exit(CODE);                                   \
  } while(0)                                      \

void poll_key_presses(CamView *cv, Vector3 *pos, menu_t *menu) {
#define SPEED (0.25f * SCREEN_WIDTH / SCREEN_HEIGHT)
  Vector3 dp = {0};
  if (IsKeyDown(KEY_W)) {dp.z += 1; dp.x += 1;}
  if (IsKeyDown(KEY_A)) {dp.z -= 1; dp.x += 1;}
  if (IsKeyDown(KEY_S)) {dp.z -= 1; dp.x -= 1;}
  if (IsKeyDown(KEY_D)) {dp.x -= 1; dp.z += 1;}

  if (dp.x != 0 || dp.z != 0) {
    *pos = Vector3Add(*pos, Vector3Scale(Vector3Normalize(dp), SPEED));
  }

  if (IsKeyDown(KEY_UP))   cv->current->position.y -= 1.5f;
  if (IsKeyDown(KEY_DOWN)) cv->current->position.y += 1.5f;

  static size_t cam_idx = 0;
  if (IsKeyPressed(KEY_C)) {
    cam_idx = (cam_idx + 1) % 2;
    switch (cam_idx) {
    case 0: cv->current = &cv->scene; break;
    case 1: cv->current = &cv->sun;   break;
    default: CLOSE_WITH(EXIT_FAILURE, "huh?");
    }
  }

  if (IsKeyPressed(KEY_M)) *menu = (*menu == SCENE) ? TEXTURE : SCENE;

  if (IsKeyPressed(KEY_ESCAPE)) CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
  if (IsKeyPressed(KEY_Q))      CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
}

