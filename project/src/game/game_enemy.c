#include "game/game.h"
#include "view/assets_cache.h"
#include <stdint.h>
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

bool enemy_can_move(t_game_state *game, enemy_t *enemy, direction_t dir) {
    if (game == NULL || enemy == NULL) return false;

    t_tuple next = enemy->board_pos;

    if (dir == DIR_LEFT) next.x--;
    else if (dir == DIR_RIGHT) next.x++;
    else if (dir == DIR_UP) next.y--;
    else next.y++;

    return !collision(game, enemy, next);
}

// enemis always try to move forward or turn at corners, but have a 5% chance of inverting direction (to avoid getting stuck in small loops)
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

    // no option but to go back
    if (count == 0) {
        if (enemy_can_move(game, enemy, opposite)) enemy->dir = opposite;
        return;
    }

    // choose random valid direction
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
    enemy->bomb_max = 0;
    enemy->bomb_available = 0;

    direction_t valid_dirs[4];
    int count = get_valid_directions(game, enemy->board_pos, valid_dirs);

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

void update_enemy_animation(t_game_state *game, enemy_t *enemy, uint32_t logical_ticks) {
    if (enemy == NULL || !enemy->active) return;

    if (enemy->invincibility_timer > 0) {
        enemy->invincibility_timer--;
        
        if (enemy->lives == 0 && enemy->invincibility_timer == 0) {
            enemy->active = false;

            // power-up drop
            if (game != NULL) {
                int active_enemies = 0;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (game->enemies[i].active) active_enemies++;
                }

                // 20% + 5% per enemy, cap at 50%
                int drop_chance = 20 + (active_enemies * 5);
                if (drop_chance > 50) drop_chance = 50;

                int r = rand() % 100;
                if (r < drop_chance / 2) {
                    game->board[enemy->board_pos.y * BOARD_COLS + enemy->board_pos.x] = TILE_TYPE_POWERUP_REACH;
                } else if (r < drop_chance) {
                    game->board[enemy->board_pos.y * BOARD_COLS + enemy->board_pos.x] = TILE_TYPE_POWERUP_COUNT;
                }
            }
        }
    }

    if (enemy->lives == 0) return; // Stop animation if dying

    if (logical_ticks % 30 == 0) {
        enemy->animation_phase = (enemy->animation_phase + 1) % 4;
    }

    if (!enemy->is_moving) return;
}
