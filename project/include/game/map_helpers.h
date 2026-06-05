#ifndef LCOM_PROJECT_MAP_HELPERS_H
#define LCOM_PROJECT_MAP_HELPERS_H

#include <stdint.h>

int decide_grass_sprite(const uint8_t *board, int rows, int cols, int x, int y);
int decide_wall_sprite(const uint8_t *board, int rows, int cols, int x, int y);

#endif /* LCOM_PROJECT_MAP_HELPERS_H */
