#include <genesis.h>

#include "resources.h"

// show the given image at the given position
void showAnImage(Image theImg, u16 x, u16 y) {
    VDP_loadTileSet(theImg.tileset, TILE_USER_INDEX, DMA);
    TileMap *img_map = unpackTileMap(theImg.tilemap, NULL);
    VDP_setTileMapEx(BG_A, img_map, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, TILE_USER_INDEX), 6, 6,  0, 0, x, y, CPU);
    PAL_setColors(0,  (u16*)theImg.palette->data, 16, CPU);
}

void splash() {
    int frames = 0;
    // show the SGDK logo, you know, like a SEGA splash screen
    showAnImage(logo_sgdk, 30, 12);
    // so some text
    VDP_drawText("Hello, World!!!!", 12, 20);

    // hold for a few seconds
    while(frames < 180) {
        SYS_doVBlankProcess();
        frames++;
    }

    // clear
    VDP_clearPlane(BG_A, TRUE);
}
