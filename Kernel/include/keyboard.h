#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define CAPS_LOCK 0x3A
#define LEFT_CONTROL 0x1D
#define BREAKCODE_OFFSET 0x80

#define KEYS_COUNT 69
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
void     print_registers();
void     clear_buffer();
uint8_t  get_char_from_buffer();
uint64_t read_keyboard_buffer(char *buff_copy, uint64_t count);
void     handle_pressed_key();

int copy_registers(register_info_t * buffer);
uint32_t uint64_to_register_format(uint64_t value, char *dest);
uint8_t  is_pressed_key(char c);

#endif
