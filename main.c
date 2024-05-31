#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#include "config.h"
#include "voxel.h"

#define MEM_MAX_CAP (1024 * 1024 * 500)
#define LOAD_PROC_INFO
#include "proc.h"

#define CLOSE_WITH(CODE, MSG)                     \
  do {                                            \
    fprintf(stdout, "[INFO] %s() :: ", __func__); \
    fprintf(stdout, "%s\n", MSG);                 \
    exit(CODE);                                   \
  } while(0)                                      \

void exit_clean_procedure(int, void *);
void poll_key_presses(Camera3D *, Vector3 *);
void poll_mouse_movement(Camera3D *);

#define SZ_WORLD   500
#define SZ_CHECKER 2

#define WORLD_ORIGIN ((Vector3){0, 0, 0})
#define CAM_ORIGIN \
  ((Vector3){SZ_WORLD * 0.66, SZ_WORLD * 0.375, SZ_WORLD * 0.5})

int SEED = 69420;

Camera3D cam_init_scene(Vector3 *);
Model mdl_gen_checkerboard(void);

int main(void) {
  on_exit(exit_clean_procedure, NULL);
  PROC_INFO_BOOTSTRAP();
  PROC_INFO_CAP_MEM(MEM_MAX_CAP);

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_NULL);

  Model mdl_cb = mdl_gen_checkerboard();

  Vector3 player_position = {0};
  Vector3 world_position = WORLD_ORIGIN;

  Camera3D cam_scene = cam_init_scene(&player_position);

  VoxelScape vxl_scape = voxel_gen_perlin_scape(CHUNK_X, CHUNK_Z, CHUNK_Y,
                                                SEED, fd_perlin);
  voxel_cull_occluded(&vxl_scape);
  Model mdl_terrain_chunk = voxel_terrain_model_from_scape(&vxl_scape);


  while(!WindowShouldClose()) {
    poll_key_presses(&cam_scene, &world_position);
    poll_mouse_movement(&cam_scene);
    BeginDrawing();
      ClearBackground(BLACK);

      BeginMode3D(cam_scene);
        DrawModel(mdl_cb, world_position, 1.0f, WHITE);
        DrawModel(mdl_terrain_chunk, world_position, 1.0f, WHITE);
      EndMode3D();

      PROC_INFO_DRAW(PROC_INFO_FLAG_ALL);
    EndDrawing();
  }

  UnloadModel(mdl_terrain_chunk);
  voxel_destroy_scape(&vxl_scape);
  CloseWindow();
  return EXIT_SUCCESS;
}

void poll_mouse_movement(Camera3D *cam) {
#define ZOOM_RATE (5.0f)
  cam->fovy -= GetMouseWheelMove() * ZOOM_RATE;
  cam->fovy = Clamp(cam->fovy, 2.0f, 180.0f);
}

void poll_key_presses(Camera3D *cam, Vector3 *pos) {
#define SPEED (0.25f * SCREEN_WIDTH / SCREEN_HEIGHT)
  Vector3 dp = {0};
  if (IsKeyDown(KEY_W))    {dp.z += 1; dp.x += 1;}
  if (IsKeyDown(KEY_A))    {dp.z -= 1; dp.x += 1;}
  if (IsKeyDown(KEY_S))    {dp.z -= 1; dp.x -= 1;}
  if (IsKeyDown(KEY_D))    {dp.x -= 1; dp.z += 1;}

  if (IsKeyDown(KEY_N)) {
    int *s_ptr = &SEED;
    *s_ptr = GetRandomValue(420, 69420);
  }

  if (dp.x != 0 || dp.z != 0) {
    *pos = Vector3Add(*pos, Vector3Scale(Vector3Normalize(dp), SPEED));
  }

  if (IsKeyDown(KEY_UP))   cam->position.y -= 1.5f;
  if (IsKeyDown(KEY_DOWN)) cam->position.y += 1.5f;

  if (IsKeyPressed(KEY_ESCAPE)) CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
  if (IsKeyPressed(KEY_Q))      CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
}

Camera3D cam_init_scene(Vector3 *player_position) {
  return (Camera3D) {
    .position   = CAM_ORIGIN,
    .target     = *player_position,
    .up         = (Vector3){0.0f, 1.0f, 0.0f},
    .fovy       = 66.0f,
    .projection = CAMERA_ORTHOGRAPHIC,
  };
}

Model mdl_gen_checkerboard(void) {
  Image img_checked = GenImageChecked(SZ_WORLD, SZ_WORLD, SZ_CHECKER,
                                      SZ_CHECKER, BLUE, DARKBLUE);
  Texture2D txtr_cb = LoadTextureFromImage(img_checked);
  UnloadImage(img_checked);

  Model mdl_cb = LoadModelFromMesh(GenMeshPlane(SZ_WORLD, SZ_WORLD, 1, 1));
  mdl_cb.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = txtr_cb;
  return mdl_cb;
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
