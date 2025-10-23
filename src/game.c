#include <genesis.h>
#include "game_object.h"
#include "resources.h"

typedef enum { PL_STATE_DIED, PL_STATE_NORMAL } PlayerState;

#define PLAYER_HEIGHT                   40          // Player object height
#define PLAYER_WIDTH                    40          // Player object width
#define PLAYER_SPAWN_POS_X               0          // Player spawn x position
#define PLAYER_SPAWN_POS_Y              32          // Player spawn y position
#define PLAYER_SPEED                    FIX16(3)    // Player movement speedPLAYER_SPEED

typedef struct {
  GameObject;         // Inherits base GameObject properties
  PlayerState state;  // Current player state
  u16 coolDownTimer;  // Shooting cooldown counter
  u16 respawnTimer;   // Timer for re-spawning after death
} Player;

typedef struct{ Player player; } GameState;
GameState game;

void player_spawn(){
  gameObject_init((GameObject *) &game.player, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y);
  game.player.state = PL_STATE_NORMAL;
}

void player_create() {
  gameObject_create((GameObject *) &game.player, &player_sprite, PAL1, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y, PLAYER_WIDTH, PLAYER_HEIGHT);
  player_spawn();
}

void player_try_shoot(Player *player) {
  // if (player->coolDownTimer == 0) {
  //   // Implement shooting logic here (e.g., create a bullet object)
  //   player->coolDownTimer = 20; // Set cooldown (example value)
  // }
}

void player_get_input(Player *player) {
  u16 input = JOY_readJoypad(JOY_1);

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
  player_get_input(&game.player);
}

void player_update_position() {
  game.player.x = clamp(game.player.x, FIX16(0), FIX16(VDP_getScreenWidth() - game.player.w));
  game.player.y = clamp(game.player.y, FIX16(0), FIX16(VDP_getScreenHeight() - game.player.h));

  // Update sprite position
  SPR_setPosition(game.player.sprite, fix16ToInt(game.player.x), fix16ToInt(game.player.y));
}

void game_run() {
  SPR_init();
  PAL_setPalette(PAL1, player_sprite.palette->data, DMA);
  player_create();

  while (true) {
    player_update();
    player_update_position();
    SPR_update();
    SYS_doVBlankProcess();
  }
}