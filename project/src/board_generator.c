#include "board_generator.h"
#include <stdlib.h>
#include <stdbool.h>

#define ROWS 9
#define COLS 15

static int dRow[4] = {1, -1, 0, 0};
static int dCol[4] = {0, 0, 1, -1};

static bool isBoardValid(int innerBoard[ROWS][COLS]) {
    if (innerBoard[0][0] != 0) return false;

    bool visited[ROWS][COLS] = {{false}};
    int queueRow[ROWS * COLS], queueCol[ROWS * COLS];
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

            if (ni < 0 || nj < 0 || ni >= ROWS || nj >= COLS) continue;
            if (visited[ni][nj]) continue;
            if (innerBoard[ni][nj] != 0) continue;

            visited[ni][nj] = true;
            queueRow[back] = ni;
            queueCol[back] = nj;
            back++;
        }
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (innerBoard[i][j] == 0 && !visited[i][j]) return false;
        }
    }

    return true;
}

static void generateInnerBoard(int innerBoard[ROWS][COLS], int seed) {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i % 2 == 1 && j % 2 == 1) innerBoard[i][j] = 1;
            else innerBoard[i][j] = 0;
        }
    }

    srand(seed);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (innerBoard[i][j] != 0) continue;

            if ((rand() % 100) < 30) {
                innerBoard[i][j] = 2;

                if (!isBoardValid(innerBoard)) {
                    innerBoard[i][j] = 0;
                }
            }
        }
    }
}

void generateBoard(char *board, int day, int month, int year) {
    int innerBoard[ROWS][COLS];

    int seed = year * 10000 + month * 100 + day;

    generateInnerBoard(innerBoard, seed);

    for (int y = 0; y < 11; y++) {
        for (int x = 0; x < 17; x++) {
            if (y == 0 || y == 10 || x == 0 || x == 16) {
                board[y * 17 + x] = 1;
            } else {
                board[y * 17 + x] = innerBoard[y - 1][x - 1];
            }
        }
    }
}
