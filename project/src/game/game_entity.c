#include "game/game.h"

bool collision(t_game_state *game, const entity_t *entity, t_tuple pos) {

    if (game == NULL) return true;
    if (pos.x < 0 || pos.y < 0 || pos.x >= BOARD_COLS || pos.y >= BOARD_ROWS) return true;

    uint8_t tile = game->board[BOARD_IDX(pos.x, pos.y)];
    if (tile != TILE_TYPE_GRASS && 
        tile != TILE_TYPE_DOOR && 
        tile != TILE_TYPE_POWERUP_REACH && 
        tile != TILE_TYPE_POWERUP_COUNT) {
        return true;
    }

    for (int i = 0; i < MAX_BOMBS; i++) {
        const bomb_t *bomb = &game->bomb[i];
        if (!bomb->active || bomb->state == BOMB_FIRE) continue;
        if (bomb->board_pos.x == pos.x && bomb->board_pos.y == pos.y) {
            if (entity &&
                entity->board_pos.x == pos.x &&
                entity->board_pos.y == pos.y) {
                continue;
            }
            return true;
        }
    }

    return false;
}

direction_t opposite_dir(direction_t dir) {

    if (dir == DIR_LEFT) return DIR_RIGHT;

    if (dir == DIR_RIGHT) return DIR_LEFT;

    if (dir == DIR_UP) return DIR_DOWN;

    return DIR_UP;
}

int get_valid_directions(t_game_state *game, t_tuple pos, direction_t out[4]) {

    if (game == NULL || out == NULL) return 0;

    int count = 0;

    t_tuple next;

    // LEFT
    next = pos;
    next.x--;

    if (!collision(game, NULL, next)) {
        out[count] = DIR_LEFT;
        count++;
    }

    // RIGHT
    next = pos;
    next.x++;

    if (!collision(game, NULL, next)) {
        out[count] = DIR_RIGHT;
        count++;
    }

    // UP
    next = pos;
    next.y--;

    if (!collision(game, NULL, next)) {
        out[count] = DIR_UP;
        count++;
    }

    // DOWN
    next = pos;
    next.y++;

    if (!collision(game, NULL, next)) {
        out[count] = DIR_DOWN;
        count++;
    }

    return count;
}

t_tuple get_board_pos(const t_game_state *game, const entity_t *entity) {
    if (game == NULL || entity == NULL) return (t_tuple){0,0};

    int tile = game->tile_size;
    t_tuple out;

    out.x = entity->pos.x / tile;
    out.y = entity->pos.y / tile;

    return out;
}

bool entity_overlaps(t_tuple pos_a, t_tuple size_a, t_tuple pos_b, t_tuple size_b) {
    int32_t a_left   = pos_a.x - (int32_t)(size_a.x / 2);
    int32_t a_right  = pos_a.x + (int32_t)(size_a.x / 2);
    int32_t a_top    = pos_a.y - (int32_t)(size_a.y / 2);
    int32_t a_bottom = pos_a.y + (int32_t)(size_a.y / 2);

    int32_t b_left   = pos_b.x - (int32_t)(size_b.x / 2);
    int32_t b_right  = pos_b.x + (int32_t)(size_b.x / 2);
    int32_t b_top    = pos_b.y - (int32_t)(size_b.y / 2);
    int32_t b_bottom = pos_b.y + (int32_t)(size_b.y / 2);

    return !(a_right < b_left || b_right < a_left || a_bottom < b_top || b_bottom < a_top);
}

bool player_collides_with_enemy(const t_game_state *game, const player_t *player) {
    if (game == NULL || player == NULL) return false;

    for (int i = 0; i < game->enemy_count; i++) {
        const enemy_t *enemy = &game->enemies[i];
        if (!enemy->active) continue;

        if (entity_overlaps(player->pos, player->size, enemy->pos, enemy->size)) return true;
    }
    return false;
}

void update_entity_movement(t_game_state *game, entity_t *entity) {
    if (!entity || !entity->is_moving) return;

    t_tuple new_pos = entity->pos;
    int tile = game->tile_size;
    int half = tile / 2;

    int dx = (entity->dir == DIR_RIGHT) - (entity->dir == DIR_LEFT);
    int dy = (entity->dir == DIR_DOWN) - (entity->dir == DIR_UP);

    new_pos.x += dx * entity->speed;
    new_pos.y += dy * entity->speed;

    t_tuple new_board_pos = entity->board_pos;
    int t = 0;

    if (dx > 0) {
        new_board_pos.x = (entity->pos.x - half) / tile + 1;
        t = new_board_pos.x * tile + half;
        if (new_pos.x > t) new_pos.x = t;
    } else if (dx < 0) {
        new_board_pos.x = (entity->pos.x - half - 1) / tile;
        t = new_board_pos.x * tile + half;
        if (new_pos.x < t) new_pos.x = t;
    } else if (dy > 0) {
        new_board_pos.y = (entity->pos.y - half) / tile + 1;
        t = new_board_pos.y * tile + half;
        if (new_pos.y > t) new_pos.y = t;
    } else if (dy < 0) {
        new_board_pos.y = (entity->pos.y - half - 1) / tile;
        t = new_board_pos.y * tile + half;
        if (new_pos.y < t) new_pos.y = t;
    }

    if (collision(game, entity, new_board_pos)){
        entity->is_moving = false;
        entity->pos.x = (entity->board_pos.x * tile) + half;
        entity->pos.y = (entity->board_pos.y * tile) + half;
        if (entity->on_snap) entity->on_snap(game, entity);
        return;
    }

    int snap_x = ((new_pos.x - half) % tile) == 0;
    int snap_y = ((new_pos.y - half) % tile) == 0;

    if ((dx != 0 && snap_y) || (dy != 0 && snap_x)) {
        entity->pos = new_pos;
        if (snap_x && snap_y) {
            if (entity->on_snap) entity->on_snap(game, entity);
        }
    }
    entity->board_pos = get_board_pos(game, entity);
}
