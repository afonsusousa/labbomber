#ifndef LCOM_PROJECT_BOARD_GENERATOR_H
#define LCOM_PROJECT_BOARD_GENERATOR_H

#include <stdint.h>
#include "game.h"

void generateBoard(char *board, uint32_t seed);

t_tuple door_spawnpoint_generator(uint8_t *board, uint32_t click_count, uint32_t seed);

#endif /* LCOM_PROJECT_BOARD_GENERATOR_H */
