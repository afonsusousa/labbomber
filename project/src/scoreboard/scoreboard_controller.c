#include "scoreboard/scoreboard_controller.h"
#include <string.h>
#include <stdio.h>

static score_entry_t entries[SCOREBOARD_MAX_ENTRIES];
static uint32_t      count = 0;
static char          current_player[32] = "Unknown";

static void sort_entries(void) {
    for (uint32_t i = 0; i < count; i++) {
        for (uint32_t j = i + 1; j < count; j++) {
            if (entries[j].score > entries[i].score ||
               (entries[j].score == entries[i].score &&
                entries[j].duration_ticks < entries[i].duration_ticks)) {
                score_entry_t tmp = entries[i];
                entries[i]        = entries[j];
                entries[j]        = tmp;
            }
        }
    }
}

void scoreboard_init(void) {
    memset(entries, 0, sizeof(entries));
    count = 0;
    strcpy(current_player, "Unknown");
}

void scoreboard_set_current_player(const char *name) {
    if (name == NULL) return;
    strncpy(current_player, name, sizeof(current_player) - 1);
    current_player[sizeof(current_player) - 1] = '\0';
}

const char *scoreboard_current_player(void) {
    return current_player;
}

void scoreboard_submit(const char *name, uint32_t score, uint32_t duration_ticks, uint8_t day, uint8_t month, uint8_t year) {
    if (name == NULL) return;

    for (uint32_t i = 0; i < count; i++) {
        
        if (strncmp(entries[i].player_name, name, sizeof(entries[i].player_name)) == 0) {

            if (score > entries[i].score || (score == entries[i].score && duration_ticks < entries[i].duration_ticks)) {

                entries[i].score = score;
                entries[i].duration_ticks = duration_ticks;
                entries[i].day   = day;
                entries[i].month = month;
                entries[i].year  = year;
                sort_entries();
            }
            return;
        }
    }

    if (count < SCOREBOARD_MAX_ENTRIES) {
        strncpy(entries[count].player_name, name, sizeof(entries[count].player_name) - 1);
        entries[count].player_name[sizeof(entries[count].player_name) - 1] = '\0';
        entries[count].score          = score;
        entries[count].duration_ticks = duration_ticks;
        entries[count].day            = day;
        entries[count].month          = month;
        entries[count].year           = year;
        count++;
    } else {
        if (score <= entries[count - 1].score) return;
        strncpy(entries[count - 1].player_name, name, sizeof(entries[count - 1].player_name) - 1);
        entries[count - 1].player_name[sizeof(entries[count - 1].player_name) - 1] = '\0';
        entries[count - 1].score          = score;
        entries[count - 1].duration_ticks = duration_ticks;
        entries[count - 1].day            = day;
        entries[count - 1].month          = month;
        entries[count - 1].year           = year;
    }

    sort_entries();
}

const score_entry_t *scoreboard_entries(void) {
    return entries;
}

uint32_t scoreboard_count(void) {
    return count;
}

void scoreboard_save(const char *path) {
    if (path == NULL) return;
    FILE *f = fopen(path, "wb");
    if (f == NULL) return;
    fwrite(&count,  sizeof(count),        1,     f);
    fwrite(entries, sizeof(score_entry_t), count, f);
    fclose(f);
}

void scoreboard_load(const char *path) {
    if (path == NULL) return;
    FILE *f = fopen(path, "rb");
    if (f == NULL) return;

    uint32_t loaded_count = 0;
    if (fread(&loaded_count, sizeof(loaded_count), 1, f) != 1) {
        fclose(f);
        return;
    }
    if (loaded_count > SCOREBOARD_MAX_ENTRIES) loaded_count = SCOREBOARD_MAX_ENTRIES;

    if (fread(entries, sizeof(score_entry_t), loaded_count, f) != loaded_count) {
        fclose(f);
        return;
    }

    count = loaded_count;
    fclose(f);
}
