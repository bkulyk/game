#include <genesis.h>
#include "game_object.h"
#include "resources.h"

typedef enum { PL_STATE_DIED, PL_STATE_NORMAL } PlayerState;
typedef enum { POPCORN_1 } EnemyType;

#define PLAYER_HEIGHT                   40              // Player object height
#define PLAYER_WIDTH                    40              // Player object width
#define PLAYER_SPAWN_POS_X              320/2-20        // Player spawn x position
#define PLAYER_SPAWN_POS_Y              (224/5)*4 - 20  // Player spawn y position
#define PLAYER_SPEED                    FIX16(3)        // Player movement speedPLAYER_SPEED
#define PROJECTILE_COOLDOWN_TIMER       6               // Frames between shots
#define PROJECTILE_POOL_SIZE            10
#define PROJECTILE_WIDTH                16              // Projectile object width
#define PROJECTILE_HEIGHT               8               // Projectile object height
#define PROJECTILE_SPEED                FIX16(10)       // Projectile movement speed
#define POPCORN_HEIGHT                  24
#define POPCORN_WIDTH                   24
#define POPCORN_1_HP                    8

// Helper macro for iterating through all objects in a specific pool (used for initialisation object)
#define FOREACH_IN_POOL(ObjectType, object, pool) \
  for (ObjectType **ptr = (ObjectType **)pool->allocStack, *object = *ptr;  /* Initialize pointer to start of pool's allocation stack and get first object */  \
    ptr < (ObjectType **)pool->allocStack + pool->size;                     /* Continue while pointer is within pool bounds (start + size) */ \
    object = *++ptr)

typedef struct {
  GameObject;         // Inherits base GameObject properties
  PlayerState state;  // Current player state
  u16 coolDownTimer;  // Shooting cooldown counter
  u16 respawnTimer;   // Timer for re-spawning after death
  u16 joyPad;
} Player;

typedef struct {
  GameObject;
  u16 hit_points;
  EnemyType enemy_type;
} Enemy;

typedef struct { GameObject; u8 damage_points; } Projectile;

typedef struct {
  Player player;
  Pool *projectile_pool;
  Pool *enemy_pool;
  Pool *explosion_pool;
} GameState;

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

  if (projectile) {
    gameObject_init((GameObject *) projectile, x, y);
    SPR_setAlwaysOnTop(projectile->sprite);
  }
}

void enemy_spawn(EnemyType enemy_type, s16 x, s16 y) {
  Enemy *enemy = (Enemy *) POOL_allocate(game.enemy_pool);

  if (enemy) {
    gameObject_init((GameObject *) enemy, x, y);
    enemy->enemy_type = enemy_type;

    if (enemy_type == POPCORN_1) {
      enemy->hit_points = POPCORN_1_HP;
    }
  }
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
  // handle cooldown
  u16 input = JOY_readJoypad(player->joyPad);

  // left/right movement
  if (input & BUTTON_LEFT) {
    player->x -= PLAYER_SPEED;
  } else if (input & BUTTON_RIGHT) {
    player->x += PLAYER_SPEED;
  }

  // up/down movement
  if (input & BUTTON_UP) {
    player->y -= PLAYER_SPEED;
  } else if (input & BUTTON_DOWN) {
    player->y += PLAYER_SPEED;
  }

  // shooting
  if (input & BUTTON_A) {
    player_try_shoot(player);
  }
}

void weapon_cooldown_update(Player *player) {
  if (player->coolDownTimer > 0) {
    player->coolDownTimer--;
  }
}

void player_update_position() {
  game.player.x = clamp(game.player.x, FIX16(0), FIX16(VDP_getScreenWidth() - game.player.w));
  game.player.y = clamp(game.player.y, FIX16(0), FIX16(VDP_getScreenHeight() - game.player.h));

  SPR_setPosition(game.player.sprite, fix16ToInt(game.player.x), fix16ToInt(game.player.y));
}

void player_update() {
  weapon_cooldown_update(&game.player);
  player_handle_input(&game.player);
  player_update_position();
}

void release_pooled_object(Pool *pool, void *object) {
  SPR_setVisibility(((GameObject *) object)->sprite, HIDDEN);
  POOL_release(pool, object, true);
}

void enemies_update() {
  FOREACH_IN_POOL(Enemy, enemy, game.enemy_pool) {
    // Simple enemy logic can be added here
    if (!enemy || SPR_getVisibility(enemy->sprite) == HIDDEN) {
      continue;
    }

    switch (enemy->enemy_type) {
      case POPCORN_1:
      default:
        // move downwards
        enemy->y += FIX16(1);
        break;
    }

    SPR_setPosition(enemy->sprite, fix16ToInt(enemy->x), fix16ToInt(enemy->y));

    // Remove enemy if it goes off-screen
    if (enemy->y > FIX16(VDP_getScreenHeight())) {
      release_pooled_object(game.enemy_pool, (void *) enemy);
    }
  }
}

void projectile_collision_update() {
  // Simple collision detection between projectiles and enemies
  FOREACH_IN_POOL(Projectile, projectile, game.projectile_pool) {
    if (!projectile || SPR_getVisibility(projectile->sprite) == HIDDEN) {
      continue;
    }

    FOREACH_IN_POOL(Enemy, enemy, game.enemy_pool) {
      if (!enemy || SPR_getVisibility(enemy->sprite) == HIDDEN) {
        continue;
      }

      if (gameObject_collides((GameObject *) projectile, (GameObject *) enemy)) {
        // Collision detected
        if (projectile->damage_points >= enemy->hit_points) {
          enemy->hit_points -= projectile->damage_points;
        } else {
          enemy->hit_points = 0;
        }

        // Hide projectile
        release_pooled_object(game.projectile_pool, (void *) projectile);

        // Check if enemy is dead
        if (enemy->hit_points <= 0) {
          release_pooled_object(game.enemy_pool, (void *) enemy);
        }

        break; // Move to next projectile after collision
      }
    }
  }
}

void game_pools_create() {
  // projectiles
  game.projectile_pool = POOL_create(PROJECTILE_POOL_SIZE, sizeof(Projectile));
  FOREACH_IN_POOL(Projectile, projectile, game.projectile_pool) {
    gameObject_create((GameObject *) projectile, &projectile_sprite, PAL1, 0, 0, PROJECTILE_WIDTH, PROJECTILE_HEIGHT);
    SPR_setVisibility(projectile->sprite, HIDDEN);
  }

  // enemies
  game.enemy_pool = POOL_create(10, sizeof(Enemy));
  FOREACH_IN_POOL(Projectile, enemy, game.enemy_pool) {
    gameObject_create((GameObject *) enemy, &popcorn_sprite, PAL2, 0, 0, POPCORN_WIDTH, POPCORN_HEIGHT);
    SPR_setVisibility(enemy->sprite, HIDDEN);
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
      release_pooled_object(game.projectile_pool, (void *) projectile);
    }
  }
}

void game_draw_hud() {
  VDP_drawTextBG(BG_B, "press a to shoot", 15, 0);
}

void game_run() {
  JOY_init();
  SPR_init();

  PAL_setPalette(PAL1, player_sprite.palette->data, DMA);
  PAL_setPalette(PAL2, popcorn_sprite.palette->data, DMA);

  game_pools_create();
  player_create();

  enemy_spawn(POPCORN_1, 100, 50);
  enemy_spawn(POPCORN_1, 200, 80);
  enemy_spawn(POPCORN_1, 150, 120);
  enemy_spawn(POPCORN_1, 120, 160);

  while (true) {
    projectiles_update();
    player_update();
    enemies_update();
    projectile_collision_update();
    game_draw_hud();
    SPR_update();
    SYS_doVBlankProcess();
  }
}