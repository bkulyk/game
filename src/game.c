#include <genesis.h>
#include "resources.h"

typedef enum { PL_STATE_DIED, PL_STATE_NORMAL } PlayerState;

#define PLAYER_HEIGHT                   40          // Player object height
#define PLAYER_WIDTH                    40          // Player object width
#define PLAYER_SPAWN_POS_X               0          // Player spawn x position
#define PLAYER_SPAWN_POS_Y              32          // Player spawn y position

typedef struct {
  Sprite *sprite;     // Sprite reference
  fix16 x, y;         // Position (fixed point)
  u16 w, h;           // Hit box width and height
} GameObject;

typedef struct {
  GameObject;         // Inherits base GameObject properties
  PlayerState state;  // Current player state
  u16 coolDownTimer;  // Shooting cooldown counter
  u16 respawnTimer;   // Timer for re-spawning after death
} Player;

typedef struct{ Player player; } GameState;
GameState game;

void gameObject_init(GameObject *object, s16 x, s16 y) {
  SPR_setPosition(object->sprite, x, y);

  // Reset sprite animation state
  SPR_setVisibility(object->sprite, VISIBLE);
  SPR_setFrame(object->sprite, 0);

  // Set object properties
  object->x = FIX16(x);
  object->y = FIX16(y);
}

void gameObject_create(GameObject *object, const SpriteDefinition *spriteDef, u16 pal, s16 x, s16 y, u16 w, u16 h) {
  // Create new sprite
  object->sprite = SPR_addSprite(spriteDef, x, y, TILE_ATTR(pal, FALSE, FALSE, FALSE));

  // Set object properties
  object->w = w;
  object->h = h;

  gameObject_init(object, x, y);
}

void player_spawn(){
  gameObject_init((GameObject *) &game.player, PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y);
  game.player.state = PL_STATE_NORMAL;
}

void player_create() {
  gameObject_create((GameObject *) &game.player, &player_sprite, PAL1,
                      PLAYER_SPAWN_POS_X, PLAYER_SPAWN_POS_Y, PLAYER_WIDTH, PLAYER_HEIGHT);
  player_spawn();
}

void game_init() {
  SPR_init();
  PAL_setPalette(PAL1, player_sprite.palette->data, DMA);

  player_create();
}

void player_update() { }

void player_update_position() {
  game.player.x = clamp(game.player.x, FIX16(0), FIX16(VDP_getScreenWidth() - game.player.w));
  game.player.y = clamp(game.player.y, FIX16(0), FIX16(VDP_getScreenHeight() - game.player.h));

  // Update sprite position
  SPR_setPosition(game.player.sprite, fix16ToInt(game.player.x), fix16ToInt(game.player.y));
}

void run_game() {
  game_init();
  while (true) {
    player_update();
    player_update_position();
    SPR_update();
    SYS_doVBlankProcess();
  }
}