#include <genesis.h>

#ifndef GameObject

typedef struct {
  Sprite *sprite;     // Sprite reference
  fix16 x, y;         // Position (fixed point)
  u16 w, h;           // Hit box width and height
} GameObject;

#endif

void gameObject_init(GameObject *object, s16 x, s16 y);
void gameObject_create(GameObject *object, const SpriteDefinition *spriteDef, u16 pal, s16 x, s16 y, u16 w, u16 h);
bool gameObject_collides(GameObject *obj1, GameObject *obj2);
