#include <genesis.h>
#include "game_object.h"
#include "resources.h"

typedef enum { PL_STATE_DIED, PL_STATE_NORMAL } PlayerState;

#define PLAYER_HEIGHT                   40          // Player object height
#define PLAYER_WIDTH                    40          // Player object width
#define PLAYER_SPAWN_POS_X               0          // Player spawn x position
#define PLAYER_SPAWN_POS_Y              32          // Player spawn y position
#define PLAYER_SPEED                    FIX16(3)    // Player movement speedPLAYER_SPEED
#define PROJECTILE_COOLDOWN_TIMER       10          // Frames between shots
#define PROJECTILE_POOL_SIZE            10
#define PROJECTILE_WIDTH                16          // Projectile object width
#define PROJECTILE_HEIGHT               8           // Projectile object height
#define PROJECTILE_SPEED                FIX16(10)    // Projectile movement speed

// Helper macro for iterating through all objects in a specific pool (used for initialisation object)
#define FOREACH_IN_POOL(ObjectType, object, pool) \
  for (ObjectType **ptr = (ObjectType **)pool->allocStack, *object = *ptr;  /* Initialize pointer to start of pool's allocation stack and get first object */  \
    ptr < (ObjectType **)pool->allocStack + pool->size;                  /* Continue while pointer is within pool bounds (start + size) */ \
    object = *++ptr)

typedef struct {
  GameObject;         // Inherits base GameObject properties
  PlayerState state;  // Current player state
  u16 coolDownTimer;  // Shooting cooldown counter
  u16 respawnTimer;   // Timer for re-spawning after death
  u16 joyPad;
} Player;

typedef struct { GameObject; u8 damagePoints; } Projectile;

typedef struct { Player player; Pool *projectile_pool } GameState;
GameState game;

void player_spawn() {
  gameObject_init((GameObject *) &game.player, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y);
  game.player.state = PL_STATE_NORMAL;
}

void player_create() {
  gameObject_create((GameObject *) &game.player, &player_sprite, PAL1, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y, PLAYER_WIDTH, PLAYER_HEIGHT);
  game.player.joyPad = JOY_1;
  game.player.coolDownTimer = 0;
  player_spawn();
}

void projectile_spawn(s16 x, s16 y) {
  Projectile *projectile = (Projectile *) POOL_allocate(game.projectile_pool);

  if (!projectile) {
    return;
  }

  gameObject_init((GameObject *) projectile, x, y);
  SPR_setAlwaysOnTop(projectile->sprite);
}

void player_try_shoot(Player *player) {
  if (player->coolDownTimer > 0) {
    return;
  }

  player->coolDownTimer = PROJECTILE_COOLDOWN_TIMER;
  projectile_spawn(
    fix16ToInt(player->x) + (player->w / 2) - (PROJECTILE_WIDTH / 2),
    fix16ToInt(player->y) - PROJECTILE_HEIGHT
  );
}

void player_handle_input(Player *player) {
  u16 input = JOY_readJoypad(player->joyPad);
  if (player->coolDownTimer > 0) {
    player->coolDownTimer--;
  }

  // Simple left/right movement
  if (input & BUTTON_LEFT) {
    player->x -= PLAYER_SPEED;
  } else if (input & BUTTON_RIGHT) {
    player->x += PLAYER_SPEED;
  }

  if (input & BUTTON_UP) {
    player->y -= PLAYER_SPEED;
  } else if (input & BUTTON_DOWN) {
    player->y += PLAYER_SPEED;
  }

  if (input & BUTTON_A) {
    player_try_shoot(player);
  }
}

void player_update() {
  player_handle_input(&game.player);
}

void player_update_position() {
  game.player.x = clamp(game.player.x, FIX16(0), FIX16(VDP_getScreenWidth() - game.player.w));
  game.player.y = clamp(game.player.y, FIX16(0), FIX16(VDP_getScreenHeight() - game.player.h));

  SPR_setPosition(game.player.sprite, fix16ToInt(game.player.x), fix16ToInt(game.player.y));
}

void game_pools_create() {
  game.projectile_pool = POOL_create(PROJECTILE_POOL_SIZE, sizeof(Projectile));

  FOREACH_IN_POOL(Projectile, projectile, game.projectile_pool) {
    gameObject_create((GameObject *) projectile, &projectile_sprite, PAL1, 0, 0, PROJECTILE_WIDTH, PROJECTILE_HEIGHT);
    SPR_setVisibility(projectile->sprite, HIDDEN);
  }
}

void projectiles_update() {
  FOREACH_IN_POOL(Projectile, projectile, game.projectile_pool) {
    // Move projectile upwards
    if (!projectile || SPR_getVisibility(projectile->sprite) == HIDDEN) {
      continue;
    }

    projectile->y -= PROJECTILE_SPEED;
    SPR_setPosition(projectile->sprite, fix16ToInt(projectile->x), fix16ToInt(projectile->y));

    // Remove it's off-screen
    if (projectile->y < FIX16(0)) {
      SPR_setVisibility(projectile->sprite, HIDDEN);
      POOL_release(game.projectile_pool, (void *) projectile, true);
    }
  }
}

void game_run() {
  JOY_init();
  SPR_init();
  PAL_setPalette(PAL1, player_sprite.palette->data, DMA);
  game_pools_create();
  player_create();

  while (true) {
    projectiles_update();
    player_update();
    player_update_position();
    SPR_update();
    SYS_doVBlankProcess();
  }
}