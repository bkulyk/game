#include "game_object.h"

void gameObject_create(GameObject *object, const SpriteDefinition *spriteDef, u16 pal, s16 x, s16 y, u16 w, u16 h) {
  // Create new sprite
  object->sprite = SPR_addSprite(spriteDef, x, y, TILE_ATTR(pal, FALSE, FALSE, FALSE));

  // Set object properties
  object->w = w;
  object->h = h;

  gameObject_init(object, x, y);
}

void gameObject_init(GameObject *object, s16 x, s16 y) {
  SPR_setPosition(object->sprite, x, y);

  // Reset sprite animation state
  SPR_setVisibility(object->sprite, VISIBLE);
  SPR_setFrame(object->sprite, 0);

  // Set object properties
  object->x = FIX16(x);
  object->y = FIX16(y);
}

bool gameObject_collides(GameObject *obj1, GameObject *obj2) {
  return !(
    F16_toInt(obj1->x) + obj1->w <= F16_toInt(obj2->x) ||
    F16_toInt(obj1->x) >= F16_toInt(obj2->x) + obj2->w ||
    F16_toInt(obj1->y) + obj1->h <= F16_toInt(obj2->y) ||
    F16_toInt(obj1->y) >= F16_toInt(obj2->y) + obj2->h
  );
}