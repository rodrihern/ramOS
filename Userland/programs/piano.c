#include "usrlib.h"
#include "scancodes.h"
#include "music.h"

int piano_main(int argc, char *argv[]) {
    print("1    2    3    4    5    6    7    8\n");
    print("DO   RE   MI   FA  SOL   LA   SI   DO\n\n");
    print("q: quit");

    while (!sys_is_pressed(KEY_Q)) {
        if (sys_is_pressed(KEY_1)) {
            sys_speaker_start(DO);
        } else if (sys_is_pressed(KEY_2)) {
            sys_speaker_start(RE);
        } else if (sys_is_pressed(KEY_3)) {
            sys_speaker_start(MI);
        } else if (sys_is_pressed(KEY_4)) {
            sys_speaker_start(FA);
        } else if (sys_is_pressed(KEY_5)) {
            sys_speaker_start(SOL);
        } else if (sys_is_pressed(KEY_6)) {
            sys_speaker_start(LA);
        } else if (sys_is_pressed(KEY_7)) {
            sys_speaker_start(SI);
        } else if (sys_is_pressed(KEY_8)) {
            sys_speaker_start(DO5);
        } else {
            sys_speaker_stop();
        }
    }

    return 0;
}