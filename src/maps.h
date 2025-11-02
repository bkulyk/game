#ifndef OBJECT_TYPES_H
#define OBJECT_TYPES_H

#include <genesis.h>
#include "game.h"
#include "game_object.h"
#include "enemy.h"

#define TILE_SIZE                  8
#define LEVEL_1_WIDTH_TILES       64
#define LEVEL_1_HEIGHT_TILES     512
#define LEVEL_1_WIDTH_PX         LEVEL_1_WIDTH_TILES * TILE_SIZE
#define LEVEL_1_HEIGHT_PX        LEVEL_1_HEIGHT_TILES * TILE_SIZE
#define LEVEL_1_ENEMY_COUNT      (sizeof(enemies_level_1)/sizeof(enemies_level_1[0]))

typedef struct {
  char *name;
  f16 enemy_type;       // Object type index
  GameObject;                 // Object type index
} EnemySpawnPoint;

Level get_level(int level_id);

#endif // OBJECT_TYPES_H