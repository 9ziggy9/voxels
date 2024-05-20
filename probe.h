#ifndef VX_EXPERIMENTAL_H_
#define VX_EXPERIMENTAL_H_
#include <raylib.h>
#include <raymath.h>

void DrawBuilding(Vector3 *, float, float, float, float,  Color);
void DrawBuildingL(Vector3 *, float, float, float, float, Color);

#ifdef PROBE_IMPLS
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

#endif //PROB_IMPLS
#endif //VX_EXPERIMENTAL_H_
