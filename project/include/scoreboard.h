#ifndef SCOREBOARD_H
#define SCOREBOARD_H 

#include <stdint.h> 

#define SCOREBOARD_MAX_ENTRIES 5  

typedef struct {
    char player_name[32];
    uint32_t score;
    uint32_t duration_ticks; 
} score_entry_t;  

typedef struct {
    uint32_t count;
    score_entry_t entries[SCOREBOARD_MAX_ENTRIES];
} scoreboard_t;

void scoreboard_init (void); 
void scoreboard_set_current_player(const char *name);
const char *scoreboard_current_player(void);
void scoreboard_submit(const char *player_name, uint32_t score, uint32_t duration_ticks);
const score_entry_t *scoreboard_entries(void);
uint32_t scoreboard_count(void);
void scoreboard_save(const char *path);
void scoreboard_load(const char *path);

#endif  

