#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define CAPS_LOCK 0x3A
#define LEFT_CONTROL 0x1D

#define KEY_C 0x2E
#define KEY_D 0x20

#define BREAKCODE_OFFSET 0x80

#define KEYS_COUNT 81
#define BUFFER_LENGTH 256
#define LETTERS 26


typedef struct register_info {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} register_info_t;


void     init_keyboard_sem();
void     kb_flush_buffer();

uint64_t kb_read_buffer(char *buff_copy, uint64_t count);
uint8_t kb_get_char_from_buffer();
void     handle_pressed_key();
uint8_t  kb_is_pressed(uint8_t scancode);

void     print_registers();
int kb_get_snapshot(register_info_t * buffer);
uint32_t uint64_to_register_format(uint64_t value, char *dest);

#endif
