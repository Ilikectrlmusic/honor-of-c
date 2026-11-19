#ifndef RIFT_LIVING_H
#define RIFT_LIVING_H
#include <stdint.h>
/* Render-only paper puppet. The input is tightly packed premultiplied BGRA.
   Model 0 is the isolated title swordsman; 1..5 are hero busts; 6..9 are scene props. */
enum { RIG_FLAME=6, RIG_BAMBOO, RIG_STANDARD, RIG_RAVEN, RIG_COUNT };
typedef struct Living Living;
Living *living_new(int model, int width, int height, const uint32_t *pixels);
void living_free(Living *rig);
const uint32_t *living_frame(Living *rig, float seconds);
#endif
