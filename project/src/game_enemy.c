#include "draw.h"
#include "assets_cache.h"
#include "game.h"
#include "vbe.h"
#include <lcom/xpm.h>
#include <stdint.h>
#include <string.h>

static const int dCol[4] = {0, 0, -1, 1};
static const int dRow[4] = {-1, 1, 0, 0};

int spawn_enemies(uint8_t *board, t_tuple player, int n, t_tuple out[MAX_ENEMIES]) {
    if (n < 1) n = 1;
    if (n > MAX_ENEMIES) n = MAX_ENEMIES;

    // BFS desde player
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

    // recolher células candidatas com dist >= MIN_DIST_FROM_PLAYER
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

    // distribuir inimigos pelos candidatos
    int n_placed = (n_candidates < n) ? n_candidates : n;
    int offset = (player.x * 31 + player.y * 17) % n_candidates;

    for (int i = 0; i < n_placed; i++) {
        int idx = (offset + i * (n_candidates / (n_placed + 1) + 1)) % n_candidates;
        out[i].x = candidates_x[idx];
        out[i].y = candidates_y[idx];
    }

    return n_placed;
}

int draw_enemy(enemy_t *enemy, hw_video_t *video, int32_t board_start_x, int32_t board_start_y) {
    if (enemy == NULL || !enemy->active || !sprites_initialized) return 1;

    int current_phase = enemy->animation_phase % 4;
    int current_direction = enemy->sprite_dir % 4;
    
    int sprite_index = SPRITE_ENEMY_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL)
        return 1;

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    int32_t draw_x = board_start_x + enemy->pos.x - (img.width / 2);
    int32_t draw_y = board_start_y + enemy->pos.y - (img.height / 2);

    hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    return 0;
}

void update_enemy_movement(t_game_state *game, enemy_t *enemy) {

    if (!enemy || !enemy->active || !enemy->is_moving) return;

    t_tuple new_pos = enemy->pos;

    int tile = game->tile_size;
    int half = tile / 2;
    int speed = 1;

    int dx = (enemy->dir == DIR_RIGHT) - (enemy->dir == DIR_LEFT);

    int dy = (enemy->dir == DIR_STANDING) - (enemy->dir == DIR_BACK);

    new_pos.x += dx * speed;
    new_pos.y += dy * speed;

    t_tuple new_board_pos = enemy->board_pos;

    int t = 0;

    if (dx > 0) {
        new_board_pos.x = (enemy->pos.x - half) / tile + 1;

        t = new_board_pos.x * tile + half;

        if (new_pos.x > t) new_pos.x = t;
    }

    else if (dx < 0) {
        new_board_pos.x = (enemy->pos.x - half - 1) / tile;

        t = new_board_pos.x * tile + half;

        if (new_pos.x < t) new_pos.x = t;
    }

    else if (dy > 0) {
        new_board_pos.y = (enemy->pos.y - half) / tile + 1;

        t = new_board_pos.y * tile + half;

        if (new_pos.y > t) new_pos.y = t;
    }

    else if (dy < 0) {

        new_board_pos.y = (enemy->pos.y - half - 1) / tile;

        t = new_board_pos.y * tile + half;

        if (new_pos.y < t) new_pos.y = t;
    }

    if (collision(game->board, new_board_pos)) {
        enemy->is_moving = false;
        return;
    }

    int snap_x = ((new_pos.x - half) % tile) == 0;

    int snap_y = ((new_pos.y - half) % tile) == 0;

    if ((dx != 0 && snap_y) || (dy != 0 && snap_x)) {

        enemy->pos = new_pos;

        if (snap_x && snap_y) enemy->sprite_dir = enemy->dir;
    }

    int offset = tile / 10;

    if (dx > 0) enemy->board_pos.x = (enemy->pos.x - offset) / tile;

    else if (dx < 0) enemy->board_pos.x = (enemy->pos.x + offset) / tile;

    else enemy->board_pos.x = enemy->pos.x / tile;

    if (dy > 0) enemy->board_pos.y = (enemy->pos.y - offset) / tile;

    else if (dy < 0) enemy->board_pos.y = (enemy->pos.y + offset) / tile;

    else enemy->board_pos.y = enemy->pos.y / tile;
}

void update_enemy_animation(enemy_t *enemy, uint32_t logical_ticks) {
    if (enemy == NULL) return;

    if (logical_ticks % 30 == 0) {
        enemy->animation_phase = (enemy->animation_phase + 1) % 4;
    }

    if (!enemy->is_moving) return;
}
