#include <genesis.h>
#include "enemy.h"
#include "game.h"
#include "game_object.h"

void pick_new_direction(Enemy* enemy) {
  enemy->angle = random() % 1024;
}

void enemy_init(Enemy *enemy, s16 x, s16 y, EnemyType enemy_type) {
  gameObject_init((GameObject *) enemy, x, y);
  enemy->enemy_type = enemy_type;
  enemy->to_be_removed = false;
  enemy->speed = BASE_SPEED;

  switch (enemy->enemy_type) {
    case BOUNCE_POPCORN_1:
      pick_new_direction(enemy);
      enemy->hit_points = POPCORN_1_HP;
      enemy->cool_down_timer = POPCORN_SHOT_COOLDOWN;
      break;
    case POPCORN_1:
      enemy->angle = 45;
      enemy->hit_points = POPCORN_1_HP;
      enemy->cool_down_timer = POPCORN_SHOT_COOLDOWN;
      break;
    default:
      break;
  }

  kprintf("Spawned enemy type %d at (%d, %d) with angle=%d", enemy_type, x, y, enemy->angle);
}

void cool_down_timer_update(Enemy *enemy) {
  if (enemy->cool_down_timer == 0) { return; }
  enemy->cool_down_timer--;
}

void enemy_update(Enemy *enemy, GameState *game) {
  cool_down_timer_update(enemy);

  f16 dx = 0;
  f16 dy = 0;

  switch (enemy->enemy_type) {
    case BOUNCE_POPCORN_1:
    case POPCORN_1:
      // Calculate movement direction using sine/cosine
      // SGDK uses sinF16/cosF16 with 1024-based angle system
      dx = F16_mul(cosFix16(enemy->angle), enemy->speed);
      dy = F16_mul(sinFix16(enemy->angle), enemy->speed);
      break;
    default:
      break;
  }

  // Update position - move in straight line
  enemy->x += dx;
  enemy->y += dy;

  SPR_setPosition(enemy->sprite, F16_toInt(enemy->x), F16_toInt(enemy->y));
}