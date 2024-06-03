#ifndef CAM_H_
#define CAM_H_
#include <raylib.h>
#include <raymath.h>
#include "voxel.h"

typedef struct { Vector3 normal; float distance; } Plane;

typedef struct {
    Plane left; Plane right;
    Plane top;  Plane bottom;
    Plane near; Plane far;
} Frustum;

typedef struct {
  Camera3D *current;
  Camera3D scene;
  Camera3D sun;
  float    light_cone;
} CamView;

#define CAM_ORIGIN ((Vector3){ LAST_X_CHUNK * CHUNK_X * 0.66,  \
                               CHUNK_Y * CHUNK_Z,              \
                               LAST_Z_CHUNK * CHUNK_Z * 0.5})

Camera3D cam_init_scene(Vector3 *);
Camera3D cam_init_sun(void);
void cam_get_frustum(Camera3D, Frustum *);

#endif // CAM_H_
