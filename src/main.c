#include <genesis.h>
#include "splash.h"
#include "game.h"

typedef enum { SPLASH, TITLE, GAME, GAME_OVER } AppState;
AppState appState;

void app_init() {
  appState = SPLASH;
  VDP_setScreenWidth320();
  PAL_setColors(0, (u16 *)palette_black, 64, DMA);
}

int main(bool hardReset) {
  app_init();

  while (true) {
    if (appState == SPLASH) {
      splash();
      appState = GAME;
    } else if (appState == GAME) {
      game_run();
    } else {
      // nothing yet
    }
  }

  return (0);
}