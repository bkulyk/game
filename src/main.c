#include <genesis.h>

#include "resources.h"
#include "splash.h"

enum AppState { ST_SPLASH, TITLE, GAME };
enum AppState appState = ST_SPLASH;

void game() {
    VDP_drawText("Do game stuff.", 12, 20);

    SYS_enableInts();

    // game loop
    while (true) {
        SYS_doVBlankProcess();
    }
}

int main(bool hardReset) {
    SYS_disableInts();
    VDP_setScreenWidth320();
    PAL_setColors(0, (u16*) palette_black, 64, DMA);

    while (true) {
        if (appState == ST_SPLASH) {
            splash();
            appState = GAME;
        } else if (appState == GAME) {
            game();
        }
    }
    return (0);
}