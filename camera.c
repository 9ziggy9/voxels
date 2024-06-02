#include "camera.h"

Camera3D cam_init_scene(Vector3 *player_position) {
  return (Camera3D) {
    .position   = CAM_ORIGIN,
    .target     = *player_position,
    .up         = (Vector3){0.0f, 1.0f, 0.0f},
    .fovy       = 52.5f,
    .projection = CAMERA_ORTHOGRAPHIC,
  };
}

void cam_get_frustum(Camera3D cam, Frustum *fstm) {
  Matrix mv = MatrixLookAt(cam.position, cam.target, cam.up);
  Matrix mp = MatrixPerspective(cam.fovy * DEG2RAD,
                                (float) GetScreenWidth() / GetScreenHeight(),
                                0.01f, 1000.0f);

  if (cam.projection == CAMERA_ORTHOGRAPHIC) {
    float width = 20.0f;
    float height = 20.0f * (float) GetScreenHeight() / GetScreenWidth();
    mp = MatrixOrtho(-width / 2, width / 2,
                     -height / 2, height / 2,
                     0.01f, 1000.0f);
  }

  Matrix mvp = MatrixMultiply(mp, mv);

  // left
  fstm->left.distance = mvp.m3 + mvp.m7;
  fstm->left.normal = (Vector3) { mvp.m0 + mvp.m4, mvp.m1 + mvp.m5,
                                  mvp.m2 + mvp.m6 };

  // right
  fstm->right.distance = mvp.m3 - mvp.m7;
  fstm->right.normal = (Vector3) { mvp.m0 - mvp.m4, mvp.m1 - mvp.m5,
                                   mvp.m2 - mvp.m6 };

  // bottom
  fstm->bottom.distance = mvp.m3 + mvp.m12;
  fstm->bottom.normal = (Vector3) { mvp.m0 + mvp.m1, mvp.m4 + mvp.m5,
                                    mvp.m8 + mvp.m9 };

  // top
  fstm->top.distance = mvp.m3 - mvp.m12;
  fstm->top.normal = (Vector3) { mvp.m0 - mvp.m1, mvp.m4 - mvp.m5,
                                 mvp.m8 - mvp.m9 };

  // near
  fstm->near.distance = mvp.m11;
  fstm->near.normal = (Vector3) { mvp.m8, mvp.m9, mvp.m10 };

  // far
  fstm->far.distance = mvp.m3 - mvp.m11;
  fstm->far.normal = (Vector3) { mvp.m0 - mvp.m8, mvp.m1 - mvp.m9,
                                 mvp.m2 - mvp.m10 };

  Plane *planes[6] = { &fstm->left,   &fstm->right,
                       &fstm->bottom, &fstm->top,
                       &fstm->near,   &fstm->far };

  // normalize
  for (int p = 0; p < 6; p++) {
    float length      = Vector3Length(planes[p]->normal);
    planes[p]->normal = Vector3Scale(planes[p]->normal, 1.0f / length);
    planes[p]->distance /= length;
  }
}
