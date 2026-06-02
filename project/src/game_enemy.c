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

    // Blink if invincible or dying
    if (enemy->invincibility_timer > 0 && (enemy->invincibility_timer / 5) % 2 == 0) {
        return 0;
    }

    int current_phase = enemy->animation_phase % 4;
    int current_direction = enemy->sprite_dir % 4;

    int sprite_index = SPRITE_ENEMY_1_STANDING + (current_phase * 4) + current_direction;

    if (sprite_index >= SPRITE_CACHE_SIZE || scaled_sprite_cache[sprite_index].bytes == NULL)
        return 1;

    xpm_image_t img = scaled_sprite_cache[sprite_index];
    int32_t draw_x = board_start_x + enemy->pos.x;
    int32_t draw_y = board_start_y + enemy->pos.y;

    hw_vbe_draw_xpm(video, img.bytes, img, draw_x, draw_y);
    return 0;
}

bool enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir) {
    if (game == NULL || enemy == NULL) return false;

    t_tuple next = enemy->board_pos;

    if (dir == DIR_LEFT) next.x--;
    else if (dir == DIR_RIGHT) next.x++;
    else if (dir == DIR_UP) next.y--;
    else next.y++;

    return !collision(game->board, next);
}

// enemies tentam andar sempre em frente ou virar em curvas com chance igual, mas tem 5% de chance de inverter a direção (para evitar que fiquem presos em loops pequenos)
void choose_enemy_direction(t_game_state *game, enemy_t *enemy) {

    if (game == NULL || enemy == NULL) return;

    direction_t opposite = opposite_dir(enemy->dir);

    // 5% de chance de inverter direção
    if ((rand() % 20) == 1) {
        enemy->dir = opposite;
        return;
    }

    direction_t valid_dirs[4];

    int count = 0;

    direction_t dirs[4] = {
        DIR_LEFT,
        DIR_RIGHT,
        DIR_UP,
        DIR_DOWN
    };

    for (int i = 0; i < 4; i++) {
        direction_t dir = dirs[i];

        if (dir == opposite) continue;

        if (enemy_can_move(game, enemy, dir)) {
            valid_dirs[count] = dir;
            count++;
        }
    }

    // obrigado a inverter se não tiver outras opções
    if (count == 0) {
        if (enemy_can_move(game, enemy, opposite)) enemy->dir = opposite;
        return;
    }

    // escolher direção aleatória entre as válidas
    enemy->dir = valid_dirs[rand() % count];
}

static void enemy_on_snap(t_game_state *game, entity_t *enemy) {
    choose_enemy_direction(game, enemy);
    enemy->sprite_dir = enemy->dir;
}

void enemy_init(t_game_state *game, enemy_t *enemy, t_tuple spawnpoint) {
    if (!game || !enemy) return;

    enemy->pos.x = (spawnpoint.x * game->tile_size) + (game->tile_size / 2);
    enemy->pos.y = (spawnpoint.y * game->tile_size) + (game->tile_size / 2);

    uint32_t ew = (game->tile_size * 8) / 12;
    uint32_t eh = ew;

    if (sprites_initialized && sprite_cache[SPRITE_ENEMY_1_STANDING].bytes != NULL) {
        uint32_t img_w = sprite_cache[SPRITE_ENEMY_1_STANDING].width;
        uint32_t img_h = sprite_cache[SPRITE_ENEMY_1_STANDING].height;
        eh = (img_h * ew) / img_w;
    }

    enemy->size.x = ew;
    enemy->size.y = eh;

    enemy->board_pos = spawnpoint;
    enemy->active = true;
    enemy->is_moving = true;

    direction_t valid_dirs[4];
    int count = get_valid_directions(game->board, enemy->board_pos, valid_dirs);

    if (count > 0) {
        direction_t dir = valid_dirs[rand() % count];
        enemy->dir = dir;
        enemy->sprite_dir = dir;
    } else {
        enemy->dir = DIR_UP;
        enemy->sprite_dir = DIR_UP;
        enemy->is_moving = false;
    }

    enemy->animation_phase = 0;
    enemy->speed = ENEMY_SPEED;
    enemy->on_snap = enemy_on_snap;
    enemy->invincibility_timer = 0;
    enemy->lives = 1;
}

void update_enemy_lives(t_game_state *game, enemy_t *enemy, int change) {
    if (enemy == NULL || !enemy->active || enemy->lives == 0) return;

    if (change < 0) {
        if (enemy->invincibility_timer > 0) return;
        enemy->invincibility_timer = GAME_TICKS_PER_SECOND / 2;
    }

    int new_lives = (int)enemy->lives + change;
    if (new_lives < 0) new_lives = 0;
    enemy->lives = (uint8_t)new_lives;

    if (enemy->lives == 0) {
        enemy->invincibility_timer = GAME_TICKS_PER_SECOND; // 1 second blink before death
        if (game) game->score += 100;
    }
}

void update_enemy_movement(t_game_state *game, enemy_t *enemy) {
    if (!enemy || !enemy->active || !enemy->is_moving || enemy->lives == 0) return;
    update_entity_movement(game, enemy);
}

void update_enemy_animation(enemy_t *enemy, uint32_t logical_ticks) {
    if (enemy == NULL || !enemy->active) return;

    if (enemy->invincibility_timer > 0) {
        enemy->invincibility_timer--;
        
        if (enemy->lives == 0 && enemy->invincibility_timer == 0) {
            enemy->active = false;
        }
    }

    if (enemy->lives == 0) return; // Stop animation if dying

    if (logical_ticks % 30 == 0) {
        enemy->animation_phase = (enemy->animation_phase + 1) % 4;
    }

    if (!enemy->is_moving) return;
}
