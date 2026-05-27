#include "game.h"

bool collision(uint8_t *board, t_tuple pos) {

    if (pos.x < 0 || pos.y < 0 || pos.x >= BOARD_COLS || pos.y >= BOARD_ROWS) return true;

    return board[BOARD_IDX(pos.x, pos.y)] != TILE_TYPE_GRASS;
}

direction_t opposite_dir(direction_t dir) {

    if (dir == DIR_LEFT) return DIR_RIGHT;

    if (dir == DIR_RIGHT) return DIR_LEFT;

    if (dir == DIR_UP) return DIR_DOWN;

    return DIR_UP;
}

int get_valid_directions(uint8_t *board, t_tuple pos, direction_t out[4]) {

    if (board == NULL || out == NULL) return 0;

    int count = 0;

    t_tuple next;

    // LEFT
    next = pos;
    next.x--;

    if (!collision(board, next)) {
        out[count] = DIR_LEFT;
        count++;
    }

    // RIGHT
    next = pos;
    next.x++;

    if (!collision(board, next)) {
        out[count] = DIR_RIGHT;
        count++;
    }

    // UP
    next = pos;
    next.y--;

    if (!collision(board, next)) {
        out[count] = DIR_UP;
        count++;
    }

    // DOWN
    next = pos;
    next.y++;

    if (!collision(board, next)) {
        out[count] = DIR_DOWN;
        count++;
    }

    return count;
}

t_tuple get_entering_cell(const t_game_state *game, const entity_t *entity) {
    if (game == NULL || entity == NULL) return (t_tuple){0,0};

    int dx = (entity->dir == DIR_RIGHT) - (entity->dir == DIR_LEFT);
    int dy = (entity->dir == DIR_DOWN) - (entity->dir == DIR_UP);

    int tile = game->tile_size;
    int half = tile / 2;
    t_tuple result = (t_tuple){ (entity->pos.x - half) / tile, (entity->pos.y - half) / tile };
    int half_w = (int)entity->w / 2;
    int half_h = (int)entity->h / 2;

    if (dx > 0)
        result.x = (entity->pos.x + half_w) / tile;
    else if (dx < 0)
        result.x = (entity->pos.x - half_w - 1) / tile;

    if (dy > 0)
        result.y = (entity->pos.y + half_h) / tile;
    else if (dy < 0)
        result.y = (entity->pos.y - half_h - 1) / tile;

    return result;
}

t_tuple get_board_pos(const t_game_state *game, const entity_t *entity) {
    if (game == NULL || entity == NULL) return (t_tuple){0,0};

    int dx = (entity->dir == DIR_RIGHT) - (entity->dir == DIR_LEFT);
    int dy = (entity->dir == DIR_DOWN) - (entity->dir == DIR_UP);

    int tile = game->tile_size;
    int offset = tile / 10;
    t_tuple out;

    if (dx > 0) out.x = (entity->pos.x - offset) / tile;
    else if (dx < 0) out.x = (entity->pos.x + offset) / tile;
    else out.x = entity->pos.x / tile;

    if (dy > 0) out.y = (entity->pos.y - offset) / tile;
    else if (dy < 0) out.y = (entity->pos.y + offset) / tile;
    else out.y = entity->pos.y / tile;

    return out;
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

    if (collision(game->board, new_board_pos)){
        entity->is_moving = false;
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
