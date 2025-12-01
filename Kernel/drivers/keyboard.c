// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "keyboard.h"
#include "lib.h"
#include "naiveConsole.h"
#include "pipes.h"
#include "scheduler.h"
#include "semaphores.h"
#include "video.h"

#define KEYBOARD_SEM_NAME "keyboard"

#define TO_UPPER(x) ( ('a' <= x && x <= 'z') ? (x - 'a' + 'A' ) : x )

extern uint8_t snapshot_saved;
extern uint8_t pressed_key;
extern register_info_t * reg_array[];

static uint8_t caps_lock = 0;

static uint16_t buffer_start = 0;
static uint16_t buffer_end = 0; 
static uint16_t buffer_current_size = 0; 

static uint8_t buffer[BUFFER_LENGTH];

static uint8_t is_pressed[KEYS_COUNT] = {0};


// 0: not supported key
// 1: up arrow
// 2: left arrow
// 3: right arrow
// 4: down arrow

static const char lower_keys[] = {
	0,    27,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    '\n', 0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // F10
	0, 0, 0, 1, 0, 0, 2, 0, 3, 0, 0, 4
};

static const char upper_keys[] = {
	0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',  '\n', 
	0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',  '~',  0,   '|',
	'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',  0, '*', 
	0,  ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // F10
	0, 0, 0, 1, 0, 0, 2, 0, 3, 0, 0, 4
};

static const char *scancode_to_ascii[] = {lower_keys, upper_keys};

static void write_buffer(unsigned char c);

void init_keyboard_sem() {
  sem_open(KEYBOARD_SEM_NAME, 0); // Empieza en 0 (sin caracteres disponibles)
}

static void write_buffer(unsigned char c) {
  buffer[buffer_end] = c;
  buffer_end =
      (buffer_end + 1) %
      BUFFER_LENGTH; // si hay buffer overflow, se pisa lo del principio
  buffer_current_size = (buffer_current_size + 1) % BUFFER_LENGTH;

  // Post al semáforo para indicar que hay un carácter disponible
  sem_post(KEYBOARD_SEM_NAME);
}

uint8_t kb_get_char_from_buffer() {
  if (buffer_current_size == 0) {
    return -1;
  }
  --buffer_current_size;
  uint8_t result = buffer[buffer_start];
  buffer_start = (buffer_start + 1) % BUFFER_LENGTH;
  return result;
}

void kb_flush_buffer() {
    buffer_end = buffer_start = buffer_current_size = 0;
    sem_reset(KEYBOARD_SEM_NAME, 0);
}


// copia en el buff lo que hay en el buffer de teclado hasta count y va vaciando
// el buffer de teclado Bloquea hasta tener TODOS los caracteres pedidos
// (comportamiento idéntico a pipes)
uint64_t kb_read_buffer(char *buff_copy, uint64_t count) {

  for (int i = 0; i < count; i++) {
    sem_wait(KEYBOARD_SEM_NAME); // Bloquea hasta que haya un carácter disponible
    buff_copy[i] = kb_get_char_from_buffer();
  }
  return count;
}



void handle_pressed_key() {
	// mark or unmark it as pressed
	if (pressed_key < KEYS_COUNT) {
		is_pressed[pressed_key] = 1;
	} else if (pressed_key > BREAKCODE_OFFSET && 
		pressed_key - BREAKCODE_OFFSET < KEYS_COUNT) {
			is_pressed[pressed_key - BREAKCODE_OFFSET] = 0;
	}

	// handle special commands
	if (is_pressed[LEFT_CONTROL]) {
		if (pressed_key == KEY_C) {
			sch_kill_foreground_process();
		} else if (pressed_key == KEY_D) {
			write_buffer(EOF);
		}

		return;
	}


	if (pressed_key == caps_lock) {
		caps_lock = !caps_lock;
		return;
	}
	
	if (pressed_key < KEYS_COUNT) {
		uint8_t shift = is_pressed[LEFT_SHIFT] || is_pressed[RIGHT_SHIFT];
		char c = scancode_to_ascii[shift][pressed_key];
		if (caps_lock) {
			c = TO_UPPER(c);
		}
		write_buffer(c);
	}

}




uint8_t kb_is_pressed(uint8_t scancode) {
	if (scancode >= KEYS_COUNT) {
		return 0;
	}
	return is_pressed[scancode];
}

// devuelve la cantidad de caracteres escritos
uint32_t uint64_to_register_format(uint64_t value, char *dest) {
  int64_t zeros_to_pad = 16;
  uint64_t aux = value;

  // Calcular cuántos ceros hay que agregar
  while (aux) {
    aux >>= 4;
    zeros_to_pad--;
  }

  uint32_t j = 0;
  // Agregar los ceros necesarios
  for (int i = 0; i < zeros_to_pad; i++) {
    dest[j++] = '0';
  }

  // Escribir la parte significativa si value ≠ 0
  if (value) {
    j += uintToBase(value, dest + j, 16);
  }

  dest[j] = 0; // null-terminador por si hace falta usarlo como string
  return j;    // devuelve la cantidad de caracteres escritos
}


// -1 si todavia no se saco snapshot
int kb_get_snapshot(register_info_t * buffer)
{
	if (!snapshot_saved) {
		return -1;
	}
	memcpy(buffer, reg_array, sizeof(register_info_t));
	return 0;
}