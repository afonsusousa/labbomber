#include "scoreboard.h"
#include <string.h>

static score_entry_t entries[SCOREBOARD_MAX_ENTRIES];

static uint32_t count = 0; 

static char current_player[32] = "Unknown";

static void sort_entries(void) { 

    for (uint32_t i = 0; i < count; i++) {

        for(uint32_t j = i+1; j < count; j++) {

            if (entries[j].score > entries[i].score) { 

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

void scoreboard_submit(const char *name, uint32_t score) { 
    if (name == NULL) return; 

    if (count < SCOREBOARD_MAX_ENTRIES) {
        strncpy(entries[count].player_name, name, sizeof(entries[count].player_name) - 1); 

        entries[count].player_name[sizeof(entries[count].player_name) - 1] = '\0'; 

        entries[count].score = score;
        
        count++;
    }  

    else { 
        if (score <= entries[count - 1].score) return;

        strncpy(entries[count - 1].player_name, name, sizeof(entries[count - 1].player_name) - 1);

        entries[count - 1].player_name[sizeof(entries[count - 1].player_name) - 1] = '\0';

        entries[count - 1].score = score;
    }  

    sort_entries();
}   

const score_entry_t *scoreboard_entries(void) {
    return entries;
}

uint32_t scoreboard_count(void) {
    return count;
}




