# Adding New Syscalls

This guide explains how to add a new syscall to ramOS and make it accessible from Userland.

## Overview

A syscall travels through these layers:

1. Kernel header declaration (static in syscall_dispatcher.h)
2. Implementation in syscall_dispatcher.c
3. Added to the syscalls[] dispatch table with an ID
4. Userland prototype (extern) in Userland/include/syscalls.h
5. Assembly stub using the SYSCALL macro in Userland/asm/usrlib.asm
6. Called by user programs via the C prototype

Syscalls use a numeric ID (their index in the syscalls[] array). Registers are loaded by the asm stub and the kernel handler executes the function referenced at that index.

---

## Step-by-Step Guide

### 1. Pick an ID

Locate the last entry in Kernel/idt/syscall_dispatcher.c (array `syscalls[]`). Your new syscall ID is its index after you append your function pointer.

Example: If the last entry is ID 47, your new syscall becomes 48.

### 2. Declare the Kernel Static (Header)

Add a static prototype in Kernel/include/syscall_dispatcher.h.

Example (adding a syscall to return total processes):

```c
// ...existing code...
static int sys_mysyscall();
// ...existing code...
```

### 3. Implement the Function

In Kernel/idt/syscall_dispatcher.c:

```c
// ...existing code...
static int sys_mysyscall(void) {
    // your implementation
}
// ...existing code...
```

### 4. Register It in the Dispatch Table

Insert pointer in the `syscalls[]` array (Kernel/idt/syscall_dispatcher.c):

```c
void *syscalls[] = {
    // ...existing entries...
    &sys_lastsyscall, // 47
    &sys_mysyscall, // 48  <-- NEW
};
```

Record the numeric ID (48 in this example).

### 5. Add Userland Prototype

In Userland/include/syscalls.h add:

```c
// ...existing code...
extern int sys_total_processes(void);
// ...existing code...
```

### 6. Add Assembly Stub

In Userland/asm/usrlib.asm append:

```asm
; 48 - int sys_total_processes(void)
global sys_total_processes
sys_total_processes:
    SYSCALL 48
```

Place it after the last existing syscall stub.

### 7. Call It From a Program

Example usage in a user program:

```c
#include <syscalls.h>
#include <usrlib.h>

int show_proc_count(int argc, char *argv[]) {
    int n = sys_total_processes();
    printf("Processes: %d\n", n);
    return 0;
}
```

---

## Example: Simple Void Syscall

Suppose we want `sys_reset_colors()` that resets shell colors.

Kernel header:

```c
// ...existing code...
static void sys_reset_colors(void);
// ...existing code...
```

Kernel implementation:

```c
// ...existing code...
static void sys_reset_colors(void) {
    vd_reset_colors();
}
// ...existing code...
```

Dispatch table add (assume new ID 49):

```c
// ...existing code...
&sys_total_processes, // 48
&sys_reset_colors,    // 49
// ...existing code...
```

Userland prototype:

```c
// ...existing code...
extern void sys_reset_colors(void);
// ...existing code...
```

Asm stub:

```asm
; 49 - void sys_reset_colors(void)
global sys_reset_colors
sys_reset_colors:
    SYSCALL 49
```

Userland call:

```c
sys_reset_colors();
```

---

## Example: Syscall With Arguments and Return

Add `sys_add(int a, int b)` returning their sum.

Kernel header:

```c
// ...existing code...
static int sys_add(int a, int b);
// ...existing code...
```

Kernel implementation:

```c
// ...existing code...
static int sys_add(int a, int b) {
    return a + b;
}
// ...existing code...
```

Dispatch table (next ID, e.g. 50):

```c
// ...existing code...
&sys_reset_colors, // 49
&sys_add,          // 50
// ...existing code...
```

Userland prototype:

```c
// ...existing code...
extern int sys_add(int a, int b);
// ...existing code...
```

Asm stub:

```asm
; 50 - int sys_add(int a, int b)
global sys_add
sys_add:
    SYSCALL 50
```

Usage:

```c
int r = sys_add(7, 5); // r = 12
```

---

## Conventions

- Kernel-side functions are `static` and only referenced via dispatch table.
- Keep ordering consistent: update comment with IDs in array for clarity.
- Validate inputs in kernel implementation; return -1 on error when appropriate.
- Use fixed-size buffers responsibly (avoid overflow).

## Troubleshooting

Issue: Userland call returns garbage.
Fix: Ensure prototype matches implementation signature exactly (types/order).

Issue: Wrong syscall executes.
Fix: Check dispatch table index matches asm stub SYSCALL ID.

Issue: Build error: undefined reference.
Fix: Added prototype but missed `global` + stub in usrlib.asm.

Issue: Crash after call.
Fix: Verify pointer arguments are valid (not userland stack freed or NULL).

---

## Quick Template

Add new syscall named X:

1. Header: `static RET_TYPE sys_X(ARG_TYPES);`
2. Implement: `static RET_TYPE sys_X(ARG_TYPES) { /* logic */ }`
3. Append to `syscalls[]` with comment ID.
4. Userland: `extern RET_TYPE sys_X(ARG_TYPES);`
5. Asm: 
```asm
global sys_X
sys_X:
    SYSCALL <ID>
```
6. Call from program.

---

This mirrors existing patterns (see sys_write, sys_ticks, sys_create_process for argument variety). Keep consistency to reduce errors.