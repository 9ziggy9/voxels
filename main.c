#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>

#include "config.h"
#include "voxel.h"
#include "camera.h"
#include "world.h"
#include "texture.h"

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
void poll_key_presses(CamView *, Vector3 *, Shader);
void poll_mouse_movement(Camera3D *);

#define SZ_CHECKER 2

Model mdl_gen_checkerboard(void);

int main(void) {
  on_exit(exit_clean_procedure, NULL);
  PROC_INFO_BOOTSTRAP();
  PROC_INFO_CAP_MEM(MEM_MAX_CAP);

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

  Shader shade_sun = LoadShader("shaders/sun.vs", "shaders/sun.fs");
  SetShaderValue(shade_sun, GetShaderLocation(shade_sun, "sunPos"),
                 &cam.sun.position, SHADER_UNIFORM_VEC3);
  SetShaderValue(shade_sun, GetShaderLocation(shade_sun, "coneAngleDegs"),
                 &cam.light_cone, SHADER_UNIFORM_FLOAT);

  struct atlas atlas = atlas_load("./assets/blockatlas.png", 16);

  VoxelScape vxl_scape = voxel_gen_perlin_scape(LAST_X_CHUNK * CHUNK_X,
                                                LAST_Z_CHUNK * CHUNK_Z,
                                                CHUNK_Y, WORLD_SEED, fd_perlin);
  voxel_cull_occluded(&vxl_scape);
  TerrainView terrain = voxel_load_terrain_models(&vxl_scape, shade_sun, atlas);

  while(!WindowShouldClose()) {
    poll_key_presses(&cam, &world_position, shade_sun);
    poll_mouse_movement(cam.current);
    BeginDrawing();
      ClearBackground(BLACK);
      BeginMode3D(*cam.current);
        for (uint32_t n = 0; n < terrain.count; n++) {
          DrawModel(terrain.views[n], world_position, 0.5f, WHITE);
        }
      EndMode3D();
      PROC_INFO_DRAW(PROC_INFO_FLAG_ALL);
    EndDrawing();
  }

  voxel_unload_terrain_models(&terrain);
  voxel_destroy_scape(&vxl_scape);
  CloseWindow();
  return EXIT_SUCCESS;
}

void poll_mouse_movement(Camera3D *cam) {
#define ZOOM_RATE (5.0f)
  cam->fovy -= GetMouseWheelMove() * ZOOM_RATE;
  cam->fovy = Clamp(cam->fovy, 2.0f, 180.0f);
}

void poll_key_presses(CamView *cv, Vector3 *pos, Shader sh) {
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

  if (IsKeyPressed(KEY_ESCAPE)) CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");
  if (IsKeyPressed(KEY_Q))      CLOSE_WITH(EXIT_SUCCESS, "Exit key pressed.");


  static const float orbit_vel = 0.1f;
  static const float orbit_rad = SUN_DISTANCE;
  static float theta = 0.0f;

  if (IsKeyDown(KEY_K)) {
    cv->sun.up.y = (cv->sun.position.x <= 0 && cv->sun.position.z <= 0)
      ? -1 : 1;
    float dtheta = (cv->sun.position.y < 0) // TODO: calculate something better
      ? 1.0f  * orbit_vel
      : 0.125f * orbit_vel;
    theta += dtheta;
    cv->sun.position.x = orbit_rad * cos(theta) * sqrt(2) / 2;
    cv->sun.position.y = orbit_rad * sin(theta);
    cv->sun.position.z = orbit_rad * cos(theta) * sqrt(2) / 2;
    SetShaderValue(sh, GetShaderLocation(sh, "sunPos"),
                  &cv->sun.position, SHADER_UNIFORM_VEC3);
  }
  if (IsKeyDown(KEY_J)) {
    cv->sun.up.y = (cv->sun.position.x <= 0 && cv->sun.position.z <= 0)
      ? -1 : 1;
    float dtheta = (cv->sun.position.y < 0)
      ? 1.0f  * orbit_vel
      : 0.125f * orbit_vel;
    theta -= dtheta;
    cv->sun.position.x = orbit_rad * cos(theta) * sqrt(2) / 2;
    cv->sun.position.y = orbit_rad * sin(theta);
    cv->sun.position.z = orbit_rad * cos(theta) * sqrt(2) / 2;
    SetShaderValue(sh, GetShaderLocation(sh, "sunPos"),
                  &cv->sun.position, SHADER_UNIFORM_VEC3);
  }
}

Model mdl_gen_checkerboard(void) {
  Image img_checked = GenImageChecked(LAST_X_CHUNK * CHUNK_X,
                                      LAST_Z_CHUNK * CHUNK_Z,
                                      SZ_CHECKER, SZ_CHECKER,
                                      BLUE, DARKBLUE);

  Texture2D txtr_cb = LoadTextureFromImage(img_checked);
  UnloadImage(img_checked);

  Model mdl_cb = LoadModelFromMesh(GenMeshPlane(LAST_X_CHUNK * CHUNK_X,
                                                LAST_Z_CHUNK * CHUNK_Z, 1, 1));
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
