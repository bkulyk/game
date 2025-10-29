#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <genesis.h>

typedef struct {
  Sprite *sprite;     // Sprite reference
  fix16 x, y;         // Position (fixed point)
  u16 w, h;           // Hit box width and height
} GameObject;

void gameObject_init(GameObject *object, s16 x, s16 y);
void gameObject_create(GameObject *object, const SpriteDefinition *spriteDef, u16 pal, s16 x, s16 y, u16 w, u16 h);
bool gameObject_collides(GameObject *obj1, GameObject *obj2);

#endif // GAME_OBJECT_H