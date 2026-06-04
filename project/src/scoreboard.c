#include "scoreboard.h"
#include <string.h>
#include <stdio.h>

static score_entry_t entries[SCOREBOARD_MAX_ENTRIES];

static uint32_t count = 0; 

static char current_player[32] = "Unknown";

static void sort_entries(void) { 

    for (uint32_t i = 0; i < count; i++) {

        for(uint32_t j = i+1; j < count; j++) {

            if (entries[j].score > entries[i].score || 
                (entries[j].score == entries[i].score && entries[j].duration_ticks < entries[i].duration_ticks)) { 

                score_entry_t temp = entries[i]; 

                entries[i] = entries[j]; 

                entries[j] = temp;
            }
        }
    }

}

void scoreboard_init (void) { 
    memset(entries, 0, sizeof(entries));
    count = 0;
    strcpy (current_player, "Unknown");
}  

void scoreboard_set_current_player(const char *name) { 
    if (name == NULL) return; 

    strncpy (current_player, name, sizeof(current_player) - 1);

    current_player[sizeof(current_player) - 1] = '\0';
}   

const char *scoreboard_current_player(void) {
    return current_player;
}

void scoreboard_submit(const char *name, uint32_t score, uint32_t duration_ticks) { 
    if (name == NULL) return; 

    if (count < SCOREBOARD_MAX_ENTRIES) {
        strncpy(entries[count].player_name, name, sizeof(entries[count].player_name) - 1); 

        entries[count].player_name[sizeof(entries[count].player_name) - 1] = '\0'; 

        entries[count].score = score; 

        entries[count].duration_ticks = duration_ticks;
        
        count++;
    }  

    else { 
        if (score <= entries[count - 1].score) return;

        strncpy(entries[count - 1].player_name, name, sizeof(entries[count - 1].player_name) - 1);

        entries[count - 1].player_name[sizeof(entries[count - 1].player_name) - 1] = '\0';

        entries[count - 1].score = score;

        entries[count - 1].duration_ticks = duration_ticks;
    }  

    sort_entries();
}    

void scoreboard_save(const char *path) {
    FILE *f = fopen(path, "wb");
    if (f == NULL) return;
    fwrite(&count, sizeof(count), 1, f);
    fwrite(entries, sizeof(score_entry_t), count, f);
    fclose(f);
} 

void scoreboard_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return;
    fread(&count, sizeof(count), 1, f);
    if (count > SCOREBOARD_MAX_ENTRIES) count = SCOREBOARD_MAX_ENTRIES;
    fread(entries, sizeof(score_entry_t), count, f);
    fclose(f);
}

const score_entry_t *scoreboard_entries(void) {
    return entries;
}

uint32_t scoreboard_count(void) {
    return count;
}




