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
    switch (appState) {
      case SPLASH:
        splash();
        appState = GAME;
        break;
      case GAME:
        game_run();
        break;
      default:
        break;
    }
  }

  return (0);
}