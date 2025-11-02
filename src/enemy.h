#ifndef ENEMY_H
#define ENEMY_H

#include "game_object.h"
#include "game.h"

#define POPCORN_HEIGHT     24
#define POPCORN_WIDTH      24
#define POPCORN_1_HP       8
#define POPCORN_SHOT_COOLDOWN 30

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 224
#define EDGE_MARGIN 8
#define BASE_SPEED FIX16(1.0)

typedef enum { POPCORN_1, BOUNCE_POPCORN_1 } EnemyType;

typedef struct {
  GameObject;
  u16 hit_points;
  EnemyType enemy_type;
  u16 cool_down_timer;
  bool to_be_removed;
  fix16 angle;
  fix16 speed;
} Enemy;

void enemy_init(Enemy *enemy, s16 x, s16 y, EnemyType enemy_type);
void enemy_update(Enemy *enemy, GameState *game);

#endif // ENEMY_H