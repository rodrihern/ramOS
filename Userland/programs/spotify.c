#include "usrlib.h"
#include "music.h"

void tetris_song();
void mario_song();

typedef void (* song_t)();

song_t songs[] = {
    &tetris_song,
    &mario_song
};

int songs_count = sizeof(songs) / sizeof(song_t);

int spotify_main(int argc, char * argv[]) {
    int song_idx;

    if (argc > 0) {
        song_idx = satoi(argv[0]);
        if (song_idx < 0 || song_idx >= songs_count) {
            print_err("Invaild argument");
            printf("Valid arguments range from 0 to %d", songs_count-1);
        }
    } else {
        print("Select a song:\n");
        print("0: Tetris Song\n");
        print("1: Mario Song\n");

        char c;
        do {
            c = getchar();
            song_idx = c - '0';
        } while (song_idx < 0 || song_idx >= songs_count);
    }

    songs[song_idx](); // play the song

    return 0;
}

void tetris_song() {
    print("Playing Tetris spotify\n");
    beep(1320, 500);
    beep(990, 250);
    beep(1056, 250);
    beep(1188, 250);
    beep(1320, 125);
    beep(1188, 125);
    beep(1056, 250);
    beep(990, 250);
    beep(880, 500);
    beep(880, 250);
    beep(1056, 250);
    beep(1320, 500);
    beep(1188, 250);
    beep(1056, 250);
    beep(990, 750);
    beep(1056, 250);
    beep(1188, 500);
    beep(1320, 500);
    beep(1056, 500);
    beep(880, 500);
    beep(880, 500);

}

void mario_song() {
    print("Playing mario song");

    beep(659, 125); beep(659, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(523, 125); beep(659, 125); sys_sleep(125); beep(784, 125); sys_sleep(375); beep(392, 125); sys_sleep(375); beep(523, 125); sys_sleep(250); beep(392, 125); sys_sleep(250); beep(330, 125); sys_sleep(250); beep(440, 125); sys_sleep(125); beep(494, 125); sys_sleep(125); beep(466, 125); sys_sleep(42); beep(440, 125); sys_sleep(125); beep(392, 125); sys_sleep(125); beep(659, 125); sys_sleep(125); beep(784, 125); sys_sleep(125); beep(880, 125); sys_sleep(125); beep(698, 125); beep(784, 125); sys_sleep(125); beep(659, 125); sys_sleep(125); beep(523, 125); sys_sleep(125); beep(587, 125); beep(494, 125); sys_sleep(125); beep(523, 125); sys_sleep(250); beep(392, 125); sys_sleep(250); beep(330, 125); sys_sleep(250); beep(440, 125); sys_sleep(125); beep(494, 125); sys_sleep(125); beep(466, 125); sys_sleep(42); beep(440, 125); sys_sleep(125); beep(392, 125); sys_sleep(125); beep(659, 125); sys_sleep(125); beep(784, 125); sys_sleep(125); beep(880, 125); sys_sleep(125); beep(698, 125); beep(784, 125); sys_sleep(125); beep(659, 125); sys_sleep(125); beep(523, 125); sys_sleep(125); beep(587, 125); beep(494, 125); sys_sleep(375); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(415, 125); beep(440, 125); beep(523, 125); sys_sleep(125); beep(440, 125); beep(523, 125); beep(587, 125); sys_sleep(250); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(698, 125); sys_sleep(125); beep(698, 125); beep(698, 125); sys_sleep(625); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(415, 125); beep(440, 125); beep(523, 125); sys_sleep(125); beep(440, 125); beep(523, 125); beep(587, 125); sys_sleep(250); beep(622, 125); sys_sleep(250); beep(587, 125); sys_sleep(250); beep(523, 125); sys_sleep(1125); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(415, 125); beep(440, 125); beep(523, 125); sys_sleep(125); beep(440, 125); beep(523, 125); beep(587, 125); sys_sleep(250); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(698, 125); sys_sleep(125); beep(698, 125); beep(698, 125); sys_sleep(625); beep(784, 125); beep(740, 125); beep(698, 125); sys_sleep(42); beep(622, 125); sys_sleep(125); beep(659, 125); sys_sleep(167); beep(415, 125); beep(440, 125); beep(523, 125); sys_sleep(125); beep(440, 125); beep(523, 125); beep(587, 125); sys_sleep(250); beep(622, 125); sys_sleep(250); beep(587, 125); sys_sleep(250); beep(523, 125);
}