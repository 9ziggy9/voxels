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
#define SZ_CHECKER 4

#define WORLD_ORIGIN ((Vector3){0, 0, 0})
#define CAM_ORIGIN \
  ((Vector3){SZ_WORLD * 0.66, SZ_WORLD * 0.375, SZ_WORLD * 0.5})

Camera3D cam_init_scene(void);
void DrawBuilding(Vector3 *, float, float, float, float, Color);
void DrawBuildingL(Vector3 *, float, float, float, float, Color);

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
  Model mdl_test = LoadModelFromMesh(voxel_generate_mesh_from_colors((Color []){
        (Color){255, 0, 0, 255},
        (Color){0, 255, 0, 255},
        (Color){0, 0, 255, 255},
      }));
  Camera3D cam_scene = cam_init_scene();

  Vector3 my_position = {0, 1.0f, 0};
  Vector3 world_position = WORLD_ORIGIN;

#define SZ_VOXEL 2.0f
  while(!WindowShouldClose()) {
    poll_key_presses(&cam_scene, &world_position);
    poll_mouse_movement(&cam_scene);
    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode3D(cam_scene);
          DrawModel(mdl_cb, world_position, 1.0f, WHITE);
          DrawModel(mdl_test, world_position, SZ_VOXEL, WHITE);
          DrawModel(mdl_test, Vector3Add(world_position,
                                         (Vector3){0,SZ_VOXEL,0}),
                    SZ_VOXEL, WHITE);
          DrawSphere(my_position, 1.0f, RED);
        EndMode3D();
      PROC_INFO_DRAW(PROC_INFO_FLAG_ALL);
    EndDrawing();
  }
  CloseWindow();
  return EXIT_SUCCESS;
}

void poll_mouse_movement(Camera3D *cam) {
  #define ZOOM_RATE 10.0f
  cam->fovy -= GetMouseWheelMove() * ZOOM_RATE;
  cam->fovy = Clamp(cam->fovy, 8.0f, 92.5f);
}

void poll_key_presses(Camera3D *cam, Vector3 *pos) {
  if (IsKeyDown(KEY_W))    pos->z += 0.1f;
  if (IsKeyDown(KEY_A))    pos->x += 0.1f;
  if (IsKeyDown(KEY_S))    pos->z -= 0.1f;
  if (IsKeyDown(KEY_D))    pos->x -= 0.1f;
  if (IsKeyDown(KEY_UP))   cam->position.y -= 0.5f;
  if (IsKeyDown(KEY_DOWN)) cam->position.y += 0.5f;

  if (IsKeyPressed(KEY_ESCAPE)) CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
  if (IsKeyPressed(KEY_Q))      CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
}

Camera3D cam_init_scene(void) {
  return (Camera3D) {
    .position   = CAM_ORIGIN,
    .target     = WORLD_ORIGIN,
    .up         = (Vector3){0.0f, 1.0f, 0.0f},
    .fovy       = 66.0f,
    .projection = CAMERA_ORTHOGRAPHIC,
  };
}

Model mdl_gen_checkerboard(void) {
  Image img_checked = GenImageChecked(SZ_WORLD, SZ_WORLD, SZ_CHECKER,
                                      SZ_CHECKER, DARKGRAY, RAYWHITE);
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
