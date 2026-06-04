#ifndef LCOM_PROJECT_BOARD_GENERATOR_H
#define LCOM_PROJECT_BOARD_GENERATOR_H

#include <stdint.h>
#include "game/game.h"

void generateBoard(char *board);

t_tuple door_spawnpoint_generator(uint8_t *board);

#endif /* LCOM_PROJECT_BOARD_GENERATOR_H */
