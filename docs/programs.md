# Adding New Programs

This guide explains how to add a new user program to ramOS.

## Overview

Programs in ramOS are located in `Userland/programs/` and are executed as separate processes. Follow these steps to add a new program:

## Step-by-Step Guide

### 1. Create Your Program File

Create your program in `Userland/programs/`. For simple programs, a single `.c` file is sufficient.

**Example: Simple program (`hello.c`)**
```c
#include <usrlib.h>

int hello(int argc, char *argv[]) {
    print("hello world!\n");
    return 0;
}
```

### 2. For Multi-File Programs

If your program requires multiple source files, place them in a subfolder under `Userland/programs/`. The build picks up all `.c` files recursively, so no Makefile changes are needed.

**Example: Complex program structure**
```
Userland/programs/myprogram/
├── main.c
├── helper.c
└── helper.h
```

Optional (manual inclusion, only if you prefer not to rely on auto-discovery):
```makefile
# In Userland/Makefile
# ...existing code...
SOURCES_MYPROGRAM := programs/myprogram/main.c programs/myprogram/helper.c
OBJECTS_MYPROGRAM := $(SOURCES_MYPROGRAM:.c=.o)

$(MODULE): $(OBJECTS_MYPROGRAM)
# ...existing code...
```

### 3. Declare the Entry Point

Add your program's entry point to `Userland/include/programs.h`:

```c
// ...
int echo_main(int argc, char *argv[]); // example
int your_program_main(int argc, char *argv[]);  // Add this line
// ...
```

### 4. Register the Command

Add your program to the external programs array in `Userland/shell/commands.c`:

```c
static const ExternalProgram external_programs[] = {
    // ...
    {"echo", "prints to STDOUT its params", &echo_main}, // example
    {"your_program", "your program description", &your_program_main},  // Add this line
    // ... other programs
};
```

## Notes

- Program entry points should follow: `int program_name(int argc, char *argv[])`
- Return `0` for success, non-zero for errors
- Use syscalls from `syscalls.h` for I/O operations
- Helper functions are available in `usrlib.h`