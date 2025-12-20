#include "usrlib.h"
#include "scancodes.h"
#include "music.h"

#define MAP_SIZE 8

typedef struct key_note {
    uint32_t freq;
    uint8_t scancode;
} key_note_t;

static key_note_t key_note_map[] = {
    {DO, KEY_1},
    {RE, KEY_2},
    {MI, KEY_3},
    {FA, KEY_4},
    {SOL, KEY_5},
    {LA, KEY_6},
    {SI, KEY_7},
    {DO5, KEY_8}
};



int piano_main(int argc, char *argv[]) {

    uint8_t time = argc > 0 && strcmp(argv[0], "time") == 0;

    print("1    2    3    4    5    6    7    8\n");
    print("DO   RE   MI   FA  SOL   LA   SI   DO\n\n");
    print("q: quit\n\n");
    

    int prev = -1;
    uint64_t prev_timestamp = sys_ms_elapsed();

    while (!sys_is_pressed(KEY_Q)) {
        int current = -1;
        uint8_t found = 0;

        for (int i = 0; i < MAP_SIZE && !found; i++) {
            if (sys_is_pressed(key_note_map[i].scancode)) {
                current = i;
                found = 1;
            }
        }

        if (!found) {
            sys_speaker_stop();
        } else {
            sys_speaker_start(key_note_map[current].freq);
        }


        if (time && current != prev) {
            uint64_t current_timestamp = sys_ms_elapsed();
            printf("played %d for %d ms\n", prev+1, current_timestamp - prev_timestamp);
            prev = current;
            prev_timestamp = current_timestamp;
        }

        
    }

    return 0;
}

