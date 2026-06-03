#ifndef LCOM_PROJECT_BOARD_GENERATOR_H
#define LCOM_PROJECT_BOARD_GENERATOR_H

#include <stdint.h>
#include "game.h"

void generateBoard(char *board, int day, int month, int year);

t_tuple door_spawnpoint_generator(uint8_t *board, uint32_t click_count, int day, int month, int year);

#endif /* LCOM_PROJECT_BOARD_GENERATOR_H */
