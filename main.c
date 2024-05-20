#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#include "config.h"

#define MEM_MAX_CAP (1024 * 1024 * 500)
#define LOAD_PROC_INFO
#include "proc.h"

#include "voxel.h"

#define CLOSE_WITH(CODE, MSG)                     \
  do {                                            \
    fprintf(stdout, "[INFO] %s() :: ", __func__); \
    fprintf(stdout, "%s\n", MSG);                 \
    exit(CODE);                                   \
  } while(0)                                      \

void exit_clean_procedure(int, void *);
void poll_key_presses(Camera3D *, Vector3 *);
void poll_mouse_movement(Camera3D *);
Texture2D gen_mdl_checkerboard(void);

#define SZ_WORLD   100
#define SZ_CHECKER 4

#define WORLD_ORIGIN ((Vector3){0, 0, 0})
#define CAM_ORIGIN \
  ((Vector3){SZ_WORLD * 0.5f, SZ_WORLD * 0.40f, SZ_WORLD * 0.75f})

Camera3D cam_init_scene(void);
Texture2D txtr_gen_checkerboard(void);
void DrawBuilding(Vector3 *, float, float, float, float, Color);
void DrawBuildingL(Vector3 *, float, float, float, float, Color);

int main(void) {
  on_exit(exit_clean_procedure, NULL);
  PROC_INFO_BOOTSTRAP();
  PROC_INFO_CAP_MEM(MEM_MAX_CAP);

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
  SetTargetFPS(TARGET_FPS);
  SetExitKey(KEY_NULL);

  Texture2D txtr_cb = txtr_gen_checkerboard();
  Model mdl_cb = LoadModelFromMesh(GenMeshPlane(SZ_WORLD * 10.0f,
                                                SZ_WORLD * 10.0f,
                                                100, 100));
  mdl_cb.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = txtr_cb;

  Texture2D atlas   = LoadTexture("assets/blockatlas.png");
  Model voxel_grass = voxel_model_from_atlas(atlas, (Rectangle []){
      { 0, 0, 16, 16 },  // front
      { 16, 0, 16, 16 }, // back
      { 32, 0, 16, 16 }, // top
      { 48, 0, 16, 16 }, // bottom
      { 64, 0, 16, 16 }, // left
      { 80, 0, 16, 16 }  // right
    });

  Camera3D cam_scene = cam_init_scene();

  Vector3 my_position = {0, 1.0f, 0};
  Vector3 world_position = WORLD_ORIGIN;

  while(!WindowShouldClose()) {
    poll_key_presses(&cam_scene, &world_position);
    poll_mouse_movement(&cam_scene);
    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode3D(cam_scene);
          DrawModel(mdl_cb, world_position, 0.5f, WHITE);
          DrawModel(voxel_grass, world_position, 1.0f, WHITE);
          DrawSphere(my_position, 1.0f, BLUE);
        EndMode3D();
      PROC_INFO_DRAW(PROC_INFO_FLAG_ALL);
    EndDrawing();
  }
  CloseWindow();
  return EXIT_SUCCESS;
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

void poll_mouse_movement(Camera3D *cam) {
  #define ZOOM_RATE 10.0f
  cam->fovy -= GetMouseWheelMove() * ZOOM_RATE;
  cam->fovy = Clamp(cam->fovy, 8.0f, 92.5f);
}

void poll_key_presses(Camera3D *cam, Vector3 *pos) {
  if (IsKeyDown(KEY_W)) pos->z += 0.1f;
  if (IsKeyDown(KEY_A)) pos->x += 0.1f;
  if (IsKeyDown(KEY_S)) pos->z -= 0.1f;
  if (IsKeyDown(KEY_D)) pos->x -= 0.1f;
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
    .fovy       = 90.0f,
    .projection = CAMERA_ORTHOGRAPHIC,
  };
}

Texture2D txtr_gen_checkerboard(void) {
  RenderTexture2D target = LoadRenderTexture(SZ_WORLD * SZ_CHECKER,
                                             SZ_WORLD * SZ_CHECKER);
  BeginTextureMode(target);
    for (int z = 0; z < SZ_WORLD; z++) {
      for (int x = 0; x < SZ_WORLD; x++) {
        DrawRectangle(x * SZ_CHECKER, z * SZ_CHECKER,
                          SZ_CHECKER, SZ_CHECKER,
                      ((x+z) % 2) ? RAYWHITE : DARKGRAY);
      }
    }
  EndTextureMode();
  return target.texture;
}

void DrawBuilding(Vector3 *world, float x, float y,
             float height, float width, Color c)
{
  DrawCubeV(Vector3Add(*world, (Vector3){x, height * 0.5f, y}),
            (Vector3){width, height, width}, c);
}

void DrawBuildingL(Vector3 *world, float x, float y,
             float height, float width, Color c)
{
  DrawCubeWiresV(Vector3Add(*world, (Vector3){x, height * 0.5f, y}),
            (Vector3){width, height, width}, c);
}
