#include "game/game.h"
#include <string.h>

static const int dCol[4] = {0, 0, -1, 1};
static const int dRow[4] = {-1, 1, 0, 0};

int spawn_enemies_singleplayer(uint8_t *board, t_tuple player, int n, t_tuple out[MAX_ENEMIES]) {
    if (n < 1) n = 1;
    if (n > MAX_ENEMIES) n = MAX_ENEMIES;

    // BFS from player
    int  dist[TOTAL_CELLS];
    bool visited[TOTAL_CELLS];
    memset(dist,    -1,    sizeof(dist));
    memset(visited, false, sizeof(visited));

    int qx[TOTAL_CELLS], qy[TOTAL_CELLS];
    int front = 0, back = 0;

    int start = player.y * BOARD_COLS + player.x;
    dist[start]    = 0;
    visited[start] = true;
    qx[back] = player.x;
    qy[back] = player.y;
    back++;

    while (front < back) {
        int cx = qx[front];
        int cy = qy[front];
        front++;

        for (int k = 0; k < 4; k++) {
            int nx = cx + dCol[k];
            int ny = cy + dRow[k];

            if (nx < 0 || ny < 0 || nx >= BOARD_COLS || ny >= BOARD_ROWS)
                continue;

            int idx = ny * BOARD_COLS + nx;
            if (visited[idx] || board[idx] != 0)
                continue;

            visited[idx] = true;
            dist[idx]    = dist[BOARD_IDX(cx, cy)] + 1;
            qx[back] = nx;
            qy[back] = ny;
            back++;
        }
    }

    // receive candidate cells with dist >= MIN_DIST_FROM_PLAYER
    int candidates_x[TOTAL_CELLS];
    int candidates_y[TOTAL_CELLS];
    int n_candidates = 0;

    for (int y = 0; y < BOARD_ROWS; y++) {
        for (int x = 0; x < BOARD_COLS; x++) {
            int idx = BOARD_IDX(x, y);
            if (dist[idx] >= MIN_DIST_FROM_PLAYER) {
                candidates_x[n_candidates] = x;
                candidates_y[n_candidates] = y;
                n_candidates++;
            }
        }
    }

    if (n_candidates == 0)
        return 0;

    // spread enemies evenly among candidates
    int n_placed = (n_candidates < n) ? n_candidates : n;
    int offset = (player.x * 31 + player.y * 17) % n_candidates;

    for (int i = 0; i < n_placed; i++) {
        int idx = (offset + i * (n_candidates / (n_placed + 1) + 1)) % n_candidates;
        out[i].x = candidates_x[idx];
        out[i].y = candidates_y[idx];
    }

    return n_placed;
}

int spawn_enemies_multiplayer(uint8_t *board, int n, t_tuple out[MAX_ENEMIES]) {
    if (board == NULL || out == NULL) return 0;

    if (n < 1) n = 1;
    if (n > MAX_ENEMIES) n = MAX_ENEMIES;

    int center_x = BOARD_COLS / 2;
    int center_y = BOARD_ROWS / 2;

    int candidates_x[TOTAL_CELLS];
    int candidates_y[TOTAL_CELLS];
    int n_candidates = 0;

    for (int y = 1; y < BOARD_ROWS - 1; y++) {
        for (int x = 1; x < BOARD_COLS - 1; x++) {
            int idx = BOARD_IDX(x, y);

            if (board[idx] != 0) {
                continue;
            }

            int dx = x - center_x;
            int dy = y - center_y;

            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;

            if (dx + dy <= MULTIPLAYER_ENEMY_SPAWN_RADIUS) {
                candidates_x[n_candidates] = x;
                candidates_y[n_candidates] = y;
                n_candidates++;
            }
        }
    }

    if (n_candidates == 0) {
        return 0;
    }

    int n_placed = (n_candidates < n) ? n_candidates : n;

    for (int i = 0; i < n_placed; i++) {
        int pick = rand() % n_candidates;

        out[i].x = candidates_x[pick];
        out[i].y = candidates_y[pick];

        candidates_x[pick] = candidates_x[n_candidates - 1];
        candidates_y[pick] = candidates_y[n_candidates - 1];
        n_candidates--;
    }

    return n_placed;
}

static int player_on_cell(t_game_state *game, int x, int y) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->players[i].board_pos.x == x && game->players[i].board_pos.y == y) {
            return 1;
        }
    }

    return 0;
}

static int enemy_on_cell(t_game_state *game, int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active &&
            game->enemies[i].board_pos.x == x &&
            game->enemies[i].board_pos.y == y) {
            return 1;
        }
    }

    return 0;
}

int spawn_new_enemy(t_game_state *game, t_tuple *out) {
    if (game == NULL || out == NULL) return 0;

    t_tuple spawnpoints[5];

    spawnpoints[0].x = 1;
    spawnpoints[0].y = 1;

    spawnpoints[1].x = BOARD_COLS - 2;
    spawnpoints[1].y = 1;

    spawnpoints[2].x = 1;
    spawnpoints[2].y = BOARD_ROWS - 2;

    spawnpoints[3].x = BOARD_COLS - 2;
    spawnpoints[3].y = BOARD_ROWS - 2;

    spawnpoints[4].x = BOARD_COLS / 2;
    spawnpoints[4].y = BOARD_ROWS / 2;

    int n_spawnpoints = 5;

    while (n_spawnpoints > 0) {
        int pick = rand() % n_spawnpoints;

        int x = spawnpoints[pick].x;
        int y = spawnpoints[pick].y;
        int idx = BOARD_IDX(x, y);

        if (game->board[idx] == 0 && !player_on_cell(game, x, y) && !enemy_on_cell(game, x, y)) {
            out->x = x;
            out->y = y;
            return 1;
        }

        spawnpoints[pick] = spawnpoints[n_spawnpoints - 1];
        n_spawnpoints--;
    }

    return 0;
}
