#include "board_generator.h"
#include "game.h"
#include <stdlib.h>
#include <stdbool.h>

#define INNER_ROWS (BOARD_ROWS - 2)
#define INNER_COLS (BOARD_COLS - 2)

static int dRow[4] = {1, -1, 0, 0};
static int dCol[4] = {0, 0, 1, -1};

static bool isBoardValid(int innerBoard[INNER_ROWS][INNER_COLS]) {
    if (innerBoard[0][0] != TILE_TYPE_GRASS) return false;

    bool visited[INNER_ROWS][INNER_COLS] = {{false}};
    int queueRow[INNER_ROWS * INNER_COLS], queueCol[INNER_ROWS * INNER_COLS];
    int front = 0, back = 0;

    queueRow[back] = 0; 
    queueCol[back] = 0; 
    back++;
    visited[0][0] = true;

    while (front < back) {
        int i = queueRow[front];
        int j = queueCol[front];
        front++;

        for (int k = 0; k < 4; k++) {
            int ni = i + dRow[k];
            int nj = j + dCol[k];

            if (ni < 0 || nj < 0 || ni >= INNER_ROWS || nj >= INNER_COLS) continue;
            if (visited[ni][nj]) continue;
            if (innerBoard[ni][nj] != TILE_TYPE_GRASS) continue;

            visited[ni][nj] = true;
            queueRow[back] = ni;
            queueCol[back] = nj;
            back++;
        }
    }

    for (int i = 0; i < INNER_ROWS; i++) {
        for (int j = 0; j < INNER_COLS; j++) {
            if (innerBoard[i][j] == TILE_TYPE_GRASS && !visited[i][j]) return false;
        }
    }

    return true;
}

static void generateInnerBoard(int innerBoard[INNER_ROWS][INNER_COLS]) {
    for (int i = 0; i < INNER_ROWS; i++) {
        for (int j = 0; j < INNER_COLS; j++) {
            if (i % 2 == 1 && j % 2 == 1) innerBoard[i][j] = TILE_TYPE_WALL;
            else innerBoard[i][j] = TILE_TYPE_GRASS;
        }
    }

    for (int i = 0; i < INNER_ROWS; i++) {
        for (int j = 0; j < INNER_COLS; j++) {
            if (innerBoard[i][j] != TILE_TYPE_GRASS) continue;

            if ((rand() % 100) < 30) {
                innerBoard[i][j] = TILE_TYPE_BRICK;

                if (!isBoardValid(innerBoard)) {
                    innerBoard[i][j] = TILE_TYPE_GRASS;
                }
            }
        }
    }
}

void generateBoard(char *board) {
    int innerBoard[INNER_ROWS][INNER_COLS];

    generateInnerBoard(innerBoard);

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {
            if (y == 0 || y == (BOARD_ROWS - 1) || x == 0 || x == (BOARD_COLS - 1)) {
                board[y * BOARD_COLS + x] = TILE_TYPE_WALL;
            } else {
                board[y * BOARD_COLS + x] = innerBoard[y - 1][x - 1];
            }
        }
    }
}

t_tuple door_spawnpoint_generator(uint8_t *board) {
    int brick_positions[INNER_ROWS * INNER_COLS][2];
    int count = 0;

    for (int y = 1; y < BOARD_ROWS - 1; y++) {
        for (int x = 1; x < BOARD_COLS - 1; x++) {
            if (board[y * BOARD_COLS + x] == TILE_TYPE_BRICK) {
                brick_positions[count][0] = x;
                brick_positions[count][1] = y;
                count++;
            }
        }
    }

    if (count == 0) return (t_tuple){-1, -1};
    
    int index = rand() % count;

    return (t_tuple){brick_positions[index][0], brick_positions[index][1]};
}
