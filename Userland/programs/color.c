#include "usrlib.h"

#define USE "Use: color <color>\nAvailable colors: red, green, blue, cyan, magenta, yellow\nYou can also use the number of the fd, example: 'color 3' prints to STDGREEN\n"

int color_main(int argc, char * argv[]) {
    if (argc < 1) {
        print_err("Missing Argument\n");
        print_err(USE);
        return -1;
    }

    int fd;
    if(strcmp(argv[0], "red") == 0) {
        fd = STDERR;
    } else if (strcmp(argv[0], "green") == 0) {
        fd = STDGREEN;
    } else if (strcmp(argv[0], "blue") == 0) {
        fd = STDBLUE;
    } else if (strcmp(argv[0], "cyan") == 0) {
        fd = STDCYAN;
    } else if (strcmp(argv[0], "magenta") == 0) {
        fd = STDMAGENTA;
    } else if (strcmp(argv[0], "yellow") == 0) {
        fd = STDYELLOW;
    } else {
        fd = satoi(argv[0]);
    }

    if (fd < 2 || fd >= FDS_COUNT) {
        print_err("Invalid Argument\n");
        print_err(USE);
        return -1;
    }

    char c;
	while ((c = getchar()) != EOF) {
		sys_write(fd, &c, 1);
	}

	return 0;
}