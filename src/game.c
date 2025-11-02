#include <genesis.h>
#include "game_object.h"
#include "resources.h"
#include "enemy.h"
#include "maps.h"
#include "game.h"
#include "level_resources.h"

#define PLAYER_HEIGHT                   32              // Player object height
#define PLAYER_WIDTH                    32              // Player object width
#define PLAYER_SPAWN_POS_X              320/2-20        // Player spawn x position
#define PLAYER_SPAWN_POS_Y              (224/5)*4 - 20  // Player spawn y position
#define PLAYER_SPEED                    FIX16(3)        // Player movement speedPLAYER_SPEED
#define BG_V_SPEED                      -4              // Background scroll speed
#define BG_H_SPEED                      0               // Background scroll speed
#define PROJECTILE_COOLDOWN_TIMER       8               // Frames between shots
#define PROJECTILE_POOL_SIZE            8
#define PROJECTILE_WIDTH                16              // Projectile object width
#define PROJECTILE_HEIGHT               8               // Projectile object height
#define PROJECTILE_SPEED                FIX16(10)       // Projectile movement speed
#define POPCORN_POOL_SIZE               37
#define ENEMY_PROJECTILE_POOL_SIZE      24
#define ENEMY_PROJECTILE_WIDTH          8
#define ENEMY_PROJECTILE_HEIGHT         8

// Helper macro for iterating through all objects in a specific pool (used for initialisation object)
#define FOREACH_IN_POOL(ObjectType, object, pool) \
  for (ObjectType **ptr = (ObjectType **)pool->allocStack, *object = *ptr;  /* Initialize pointer to start of pool's allocation stack and get first object */  \
    ptr < (ObjectType **)pool->allocStack + pool->size;                     /* Continue while pointer is within pool bounds (start + size) */ \
    object = *++ptr)

GameState game;
Camera camera;

void player_spawn() {
  game.player = (Player *) POOL_allocate(game.player_pool);
  if (game.player) {
    gameObject_init((GameObject *) game.player, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y);
    game.player->state = PL_STATE_NORMAL;
    game.player->joy_pad = JOY_1;
    game.player->cool_down_timer = 0;
  }
}

void projectile_spawn(s16 x, s16 y) {
  Projectile *projectile = (Projectile *) POOL_allocate(game.projectile_pool);

  if (projectile) {
    gameObject_init((GameObject *) projectile, x, y);
    SPR_setAlwaysOnTop(projectile->sprite);
  }
}

// abstract pooling away from the enemy module
void enemy_spawn(EnemyType enemy_type, s16 x, s16 y) {
  Enemy *enemy = (Enemy *) POOL_allocate(game.enemy_pool);

  if (enemy) { enemy_init(enemy, x, y, enemy_type); }
}

void player_try_shoot(Player *player) {
  if (player->cool_down_timer > 0) {
    return;
  }

  player->cool_down_timer = PROJECTILE_COOLDOWN_TIMER;
  projectile_spawn(
    F16_toInt(player->x) + (player->width / 2) - (PROJECTILE_WIDTH / 2),
    F16_toInt(player->y) - PROJECTILE_HEIGHT
  );
}

void player_handle_input(Player *player) {
  // handle cooldown
  u16 input = JOY_readJoypad(player->joy_pad);

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
  if (player->cool_down_timer > 0) {
    player->cool_down_timer--;
  }
}

void player_update_position() {
  game.player->x = clamp(game.player->x, FIX16(0), FIX16(VDP_getScreenWidth() - game.player->width));
  game.player->y = clamp(game.player->y, FIX16(0), FIX16(VDP_getScreenHeight() - game.player->height));

  SPR_setPosition(game.player->sprite, F16_toInt(game.player->x), F16_toInt(game.player->y));
}

void player_update() {
  weapon_cooldown_update(game.player);
  player_handle_input(game.player);
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

    enemy_update(enemy, &game);

    // Remove enemy if it goes off-screen
    if (enemy->to_be_removed) {
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
        // collision detected
        if (projectile->damage_points >= enemy->hit_points) {
          enemy->hit_points -= projectile->damage_points;
        } else {
          enemy->hit_points = 0;
        }

        // hide projectile
        release_pooled_object(game.projectile_pool, (void *) projectile);

        // check if enemy is dead
        if (enemy->hit_points == 0) {
          release_pooled_object(game.enemy_pool, (void *) enemy);
        }

        break; // Move to next projectile after collision
      }
    }
  }
}

void game_pools_create() {
  // player -- it's probably odd to have the player in a pool of size 1 but pooling is doing some
  //  memory stuff and the player was not appearing, player is most important, so make sure there is
  //  memory for the player
  game.player_pool = POOL_create(1, sizeof(Player));
  FOREACH_IN_POOL(Projectile, player, game.player_pool) {
    gameObject_create((GameObject *) player, &player_sprite, PAL1, 0, 0, PLAYER_HEIGHT, PLAYER_WIDTH);
  }

  // projectiles
  game.projectile_pool = POOL_create(PROJECTILE_POOL_SIZE, sizeof(Projectile));
  FOREACH_IN_POOL(Projectile, projectile, game.projectile_pool) {
    gameObject_create((GameObject *) projectile, &projectile_sprite, PAL1, 0, 0, PROJECTILE_WIDTH, PROJECTILE_HEIGHT);
    SPR_setVisibility(projectile->sprite, HIDDEN);
  }

  // enemies
  game.enemy_pool = POOL_create(POPCORN_POOL_SIZE, sizeof(Enemy));
  FOREACH_IN_POOL(Projectile, enemy, game.enemy_pool) {
    gameObject_create((GameObject *) enemy, &popcorn_sprite, PAL2, 0, 0, POPCORN_WIDTH, POPCORN_HEIGHT);
    SPR_setVisibility(enemy->sprite, HIDDEN);
  }

  // enemy projectiles
  game.enemy_projectile_pool = POOL_create(ENEMY_PROJECTILE_POOL_SIZE, sizeof(Projectile));
  FOREACH_IN_POOL(Projectile, projectile, game.enemy_projectile_pool) {
    gameObject_create((GameObject *) projectile, &enemy_projectile_sprite, PAL1, 0, 0, ENEMY_PROJECTILE_WIDTH, ENEMY_PROJECTILE_HEIGHT);
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
    SPR_setPosition(projectile->sprite, F16_toInt(projectile->x), F16_toInt(projectile->y));

    // Remove it's off-screen
    if (projectile->y < FIX16(0)) {
      release_pooled_object(game.projectile_pool, (void *) projectile);
    }
  }
}

// Convert Tiled object coordinates to screen coordinates relative to camera
void tiled_coords_to_camera(f32 tiled_x, f32 tiled_y, s16 *screen_x, s16 *screen_y) {
  // Convert Tiled coordinates to world coordinates
  s16 world_x = F32_toInt(tiled_x);
  s16 world_y = F32_toInt(tiled_y);

  // Convert world coordinates to screen coordinates
  *screen_x = world_x - F32_toInt(camera.x);
  *screen_y = world_y - F32_toInt(camera.y);
}

void game_draw_hud(GameState *game) {
  char camera_text[30];
  sprintf(camera_text, "Cam: %d,%d", F32_toInt(camera.x), F32_toInt(camera.y));
  VDP_drawTextBG(BG_B, camera_text, 1, 0);
}

void bg_init() {
  PAL_setPalette(PAL3, bg_img.palette->data, DMA);
  VDP_drawImageEx(BG_A, &bg_img, TILE_ATTR_FULL(PAL3, false, false, false, TILE_USER_INDEX), 0, 0, false, DMA);
  VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
  VDP_setVerticalScroll(BG_A, 0);
  VDP_setHorizontalScroll(BG_A, 0);
}

void bg_update(GameState *game) {
  game->bg_y += BG_V_SPEED;
  game->bg_x += BG_H_SPEED;
  VDP_setVerticalScroll(BG_A, game->bg_y);
  VDP_setHorizontalScroll(BG_A, game->bg_x);
}

void camera_init() {
  camera.x = game.level.width / 2 - FIX32(VDP_getScreenWidth()) / 2;
  camera.y = game.level.height - FIX32(VDP_getScreenHeight());
  camera.width = VDP_getScreenWidth();
  camera.height = VDP_getScreenHeight();
  game.camera = &camera;
}

// void enemy_spawn_test() {
//   // make a grid of enemies like space invaders
//   for (u16 row = 0; row < 8; row++) {
//     for (u16 col = 0; col < 8; col++) {
//       s16 x = col * (POPCORN_WIDTH + 8) + 40;
//       s16 y = row * (POPCORN_HEIGHT + 8) + 20;
//       enemy_spawn(BOUNCE_POPCORN_1, x, y);
//     }
//   }
// }

void enemy_spawn_test() {
  kprintf("Number of enemies in level 1: %d\n", LEVEL_1_ENEMY_COUNT);

  for (u16 i = 0; i < LEVEL_1_ENEMY_COUNT; i++) {
    EnemySpawnPoint *esp = enemies_level_1[i];
    s16 screen_x, screen_y;
    tiled_coords_to_camera(esp->x, esp->y, &screen_x, &screen_y);
    kprintf("Spawning enemy type %d at screen position (%d, %d)", esp->enemy_type, screen_x, screen_y);
    enemy_spawn(esp->enemy_type, screen_x, screen_y);
  }
}

void map_init() {
  game.level = get_level(1);
}

void game_run() {
  JOY_init();
  SPR_init();

  PAL_setPalette(PAL1, player_sprite.palette->data, DMA);
  PAL_setPalette(PAL2, popcorn_sprite.palette->data, DMA);

  map_init();
  camera_init();
  bg_init();
  game_pools_create();
  player_spawn();
  enemy_spawn_test();

  while (true) {
    projectiles_update();
    player_update();
    enemies_update();
    projectile_collision_update();
    game_draw_hud(&game);
    bg_update(&game);
    SPR_update();
    SYS_doVBlankProcess();
  }
}