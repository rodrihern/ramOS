#include "usrlib.h"
#include "music.h"

int spotify_main(int argc, char * argv[]) {
    print("Playing: Tetris song\n");
    play_note(1320, 500);
    play_note(990, 250);
    play_note(1056, 250);
    play_note(1188, 250);
    play_note(1320, 125);
    play_note(1188, 125);
    play_note(1056, 250);
    play_note(990, 250);
    play_note(880, 500);
    play_note(880, 250);
    play_note(1056, 250);
    play_note(1320, 500);
    play_note(1188, 250);
    play_note(1056, 250);
    play_note(990, 750);
    play_note(1056, 250);
    play_note(1188, 500);
    play_note(1320, 500);
    play_note(1056, 500);
    play_note(880, 500);
    play_note(880, 500);

    return 0;
}