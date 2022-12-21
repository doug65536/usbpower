#pragma once
#include <stdint.h>

void render_init();

using coord = uint8_t;
void render_rect(coord sx, coord sy, coord ex, coord ey);
