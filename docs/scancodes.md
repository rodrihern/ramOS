# Keyboard Scancodes

This document provides a reference for keyboard scancodes used in ramOS for a US QWERTY keyboard layout.

## Overview

Scancodes are the raw data sent by the keyboard controller when a key is pressed or released. The keyboard driver in `Kernel/drivers/keyboard.c` translates these scancodes into ASCII characters.

## Scancode Format

- **Make code**: Sent when a key is pressed
- **Break code**: Sent when a key is released (usually make code + 0x80)

## US QWERTY Keyboard Scancode Chart

### Alphanumeric Keys

| Scancode (Hex) | Key | ASCII Value |
|----------------|-----|-------------|
| 0x01 | ESC | 27 |
| 0x02 | 1 | 49 ('1') |
| 0x03 | 2 | 50 ('2') |
| 0x04 | 3 | 51 ('3') |
| 0x05 | 4 | 52 ('4') |
| 0x06 | 5 | 53 ('5') |
| 0x07 | 6 | 54 ('6') |
| 0x08 | 7 | 55 ('7') |
| 0x09 | 8 | 56 ('8') |
| 0x0A | 9 | 57 ('9') |
| 0x0B | 0 | 48 ('0') |
| 0x0C | - | 45 ('-') |
| 0x0D | = | 61 ('=') |
| 0x0E | Backspace | 8 |
| 0x0F | Tab | 9 |
| 0x10 | Q | 113 ('q') |
| 0x11 | W | 119 ('w') |
| 0x12 | E | 101 ('e') |
| 0x13 | R | 114 ('r') |
| 0x14 | T | 116 ('t') |
| 0x15 | Y | 121 ('y') |
| 0x16 | U | 117 ('u') |
| 0x17 | I | 105 ('i') |
| 0x18 | O | 111 ('o') |
| 0x19 | P | 112 ('p') |
| 0x1A | [ | 91 ('[') |
| 0x1B | ] | 93 (']') |
| 0x1C | Enter | 10 ('\n') |
| 0x1D | Left Ctrl | - |
| 0x1E | A | 97 ('a') |
| 0x1F | S | 115 ('s') |
| 0x20 | D | 100 ('d') |
| 0x21 | F | 102 ('f') |
| 0x22 | G | 103 ('g') |
| 0x23 | H | 104 ('h') |
| 0x24 | J | 106 ('j') |
| 0x25 | K | 107 ('k') |
| 0x26 | L | 108 ('l') |
| 0x27 | ; | 59 (';') |
| 0x28 | ' | 39 ('\'') |
| 0x29 | ` | 96 ('`') |
| 0x2A | Left Shift | - |
| 0x2B | \ | 92 ('\\') |
| 0x2C | Z | 122 ('z') |
| 0x2D | X | 120 ('x') |
| 0x2E | C | 99 ('c') |
| 0x2F | V | 118 ('v') |
| 0x30 | B | 98 ('b') |
| 0x31 | N | 110 ('n') |
| 0x32 | M | 109 ('m') |
| 0x33 | , | 44 (',') |
| 0x34 | . | 46 ('.') |
| 0x35 | / | 47 ('/') |
| 0x36 | Right Shift | - |
| 0x37 | Keypad * | 42 ('*') |
| 0x38 | Left Alt | - |
| 0x39 | Space | 32 (' ') |
| 0x3A | Caps Lock | - |

### Function Keys

| Scancode (Hex) | Key |
|----------------|-----|
| 0x3B | F1 |
| 0x3C | F2 |
| 0x3D | F3 |
| 0x3E | F4 |
| 0x3F | F5 |
| 0x40 | F6 |
| 0x41 | F7 |
| 0x42 | F8 |
| 0x43 | F9 |
| 0x44 | F10 |
| 0x57 | F11 |
| 0x58 | F12 |

### Special Keys

| Scancode (Hex) | Key |
|----------------|-----|
| 0x45 | Num Lock |
| 0x46 | Scroll Lock |
| 0x47 | Keypad 7 / Home |
| 0x48 | Keypad 8 / Up Arrow |
| 0x49 | Keypad 9 / PgUp |
| 0x4A | Keypad - |
| 0x4B | Keypad 4 / Left Arrow |
| 0x4C | Keypad 5 |
| 0x4D | Keypad 6 / Right Arrow |
| 0x4E | Keypad + |
| 0x4F | Keypad 1 / End |
| 0x50 | Keypad 2 / Down Arrow |
| 0x51 | Keypad 3 / PgDn |
| 0x52 | Keypad 0 / Insert |
| 0x53 | Keypad . / Delete |

## Extended Scancodes (E0 Prefix)

Some keys send a two-byte scancode starting with 0xE0:

| Scancode | Key |
|----------|-----|
| E0 1C | Keypad Enter |
| E0 1D | Right Ctrl |
| E0 35 | Keypad / |
| E0 38 | Right Alt |
| E0 47 | Home |
| E0 48 | Up Arrow |
| E0 49 | Page Up |
| E0 4B | Left Arrow |
| E0 4D | Right Arrow |
| E0 4F | End |
| E0 50 | Down Arrow |
| E0 51 | Page Down |
| E0 52 | Insert |
| E0 53 | Delete |


## Notes

- Break codes are typically the make code OR'd with 0x80
- Some special keys require handling multi-byte sequences
- The keyboard controller uses PS/2 Set 1 scancodes by default
- Modifier keys need state tracking to implement Shift, Ctrl, etc.

## References

- PS/2 Keyboard Scan Code Set 1
- OSDev Wiki: [Keyboard](https://wiki.osdev.org/Keyboard)
- Intel 8042 Keyboard Controller specification