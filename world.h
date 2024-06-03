#ifndef WORLD_H_
#define WORLD_H_
#include "voxel.h"

#define SZ_WORLD (LAST_X_CHUNK * CHUNK_X) * (LAST_Z_CHUNK * CHUNK_Z)
#define WORLD_ORIGIN ((Vector3){0, 0, 0})
#define WORLD_SEED 9001
#define WORLD_PATH "./world/"

#endif // WORLD_H_
