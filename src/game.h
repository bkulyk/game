#ifndef GAME_H
#define GAME_H

#include "game_object.h"
#include "map.h"

typedef enum { PL_STATE_DIED, PL_STATE_NORMAL } PlayerState;

typedef struct {
  GameObject;           // Inherits base GameObject properties
  PlayerState state;    // Current player state
  u16 cool_down_timer;  // Shooting cooldown counter
  u16 respawn_timer;    // Timer for re-spawning after death
  u16 joy_pad;
} Player;

typedef struct {
  GameObject;         // Inherits base GameObject properties
} Camera;

typedef struct { GameObject; u8 damage_points; } Projectile;

typedef struct {
  char *name;
  f32 width;
  f32 height;
  f32 tile_width;
  f32 tile_height;
} Level;

typedef struct {
  Player *player;
  Pool *player_pool;
  Pool *projectile_pool;
  Pool *enemy_pool;
  Pool *explosion_pool;
  Pool *enemy_projectile_pool;
  u16 bg_x;
  u16 bg_y;
  Level level;
  Camera *camera;
  Map *bgMap;
} GameState;

void game_run();

#endif // GAME_H