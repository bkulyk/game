#include "game.h"
#include "maps.h"

Level get_level(int level_id) {
  Level level;
  switch (level_id) {
    case 1:
    default:
      level.name = "Level 1";
      level.width = FIX32(LEVEL_1_WIDTH_PX);
      level.height = FIX32(LEVEL_1_HEIGHT_PX);
      level.tile_width = FIX32(TILE_SIZE);
      level.tile_height = FIX32(TILE_SIZE);
      break;
  }
  return level;
}
