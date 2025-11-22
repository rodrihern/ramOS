// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "keyboard.h"
#include "lib.h"
#include "naiveConsole.h"
#include "pipes.h"
#include "scheduler.h"
#include "synchro.h"
#include "video_driver.h"

#define KEYBOARD_SEM_NAME "keyboard"

static int shift = 0;
static int caps_lock = 0;
static int control = 0;
extern uint8_t snapshot_saved;
extern uint8_t pressed_key;

uint16_t buffer_start = 0; // índice del buffer del próximo carácter a leer
uint16_t buffer_end = 0; // índice del buffer donde se va a escribir el próximo
                         // caracter recibido en el teclado
uint16_t buffer_current_size =
    0; // cantidad de caracteres en el buffer actual (listos para ser leídos)

static uint8_t buffer[BUFFER_LENGTH];

static void write_buffer(unsigned char c);

static const char lower_keys[] = {
    0,    27,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-',  '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']',
    '\n', 0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,    '*',
    0,    ' ',  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
    0,    0,    38,  0,   '-', 37,  0,   39,  '+', 0,   40,  0,   0,    0,
    0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,
};

static const char upper_keys[] = {
    0,   27,   '!',  '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',
    '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
    '{', '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
    ':', '"',  '~',  0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<',
    '>', '?',  0,    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,
    '+', 0,    0,    0,   0,   0,   0,   0,   0,   0,   0,   0};

static const char *scancode_to_ascii[] = {lower_keys, upper_keys};

static uint8_t pressed_keys[LETTERS] = {0};

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

void clear_buffer() {
  buffer_end = buffer_start = buffer_current_size = 0;

  // Resetear el semáforo: cerrar y reabrir en 0
  sem_close(KEYBOARD_SEM_NAME);
  sem_open(KEYBOARD_SEM_NAME, 0);
}

uint8_t get_char_from_buffer() {
  if (buffer_current_size == 0) {
    return -1;
  }
  --buffer_current_size;
  uint8_t result = buffer[buffer_start];
  buffer_start = (buffer_start + 1) % BUFFER_LENGTH;
  return result;
}

// copia en el buff lo que hay en el buffer de teclado hasta count y va vaciando
// el buffer de teclado Bloquea hasta tener TODOS los caracteres pedidos
// (comportamiento idéntico a pipes)
uint64_t read_keyboard_buffer(char *buff_copy, uint64_t count) {

  for (int i = 0; i < count; i++) {
    sem_wait(
        KEYBOARD_SEM_NAME); // Bloquea hasta que haya un carácter disponible
    buff_copy[i] = get_char_from_buffer();
  }
  return count;
}

void handle_pressed_key() {

  if (pressed_key == LEFT_SHIFT || pressed_key == RIGHT_SHIFT) {
    shift = 1;
  } else if (pressed_key == LEFT_SHIFT + BREAKCODE_OFFSET ||
             pressed_key == RIGHT_SHIFT + BREAKCODE_OFFSET) {
    shift = 0;
  } else if (pressed_key == LEFT_CONTROL) {
    control = 1;
  } else if (pressed_key == LEFT_CONTROL + BREAKCODE_OFFSET) {
    control = 0;
  } else if (pressed_key == CAPS_LOCK) {
    caps_lock = !caps_lock;
  } else if (pressed_key == 0) {
    return;
  } else if (pressed_key > BREAKCODE_OFFSET) { // se soltó una tecla o es un
                                            // caracter no imprimible
    char raw = lower_keys[pressed_key - BREAKCODE_OFFSET];
    if (raw >= 'a' && raw <= 'z') {
      pressed_keys[raw - 'a'] = 0; // marcamos la tecla como no presionada
    }
    return;
  } else {
    int index;
    char raw = lower_keys[pressed_key];
    int is_letter = (raw >= 'a' && raw <= 'z');

    if (is_letter && raw == 'c' && control) {
      if (!pressed_keys['c' - 'a']) { // para que solo se llame una vez
        scheduler_kill_foreground_process();
      }
      pressed_keys['c' - 'a'] = 1; // marcamos como presionada
      return;                      // para no meter la 'c' en el buffer
    }

    if (is_letter && raw == 'd' && control) {
      if (!pressed_keys['d' - 'a']) { // para que solo se llame una vez
        write_buffer(EOF);
      }
      pressed_keys['d' - 'a'] = 1; // marcamos como presionada
      return;                      // para no meter la 'd' en el buffer
    }

    if (is_letter) {
      index = shift ^ caps_lock;
      pressed_keys[raw - 'a'] = 1;
    } else {
      index = shift;
    }

    write_buffer(scancode_to_ascii[index][pressed_key]);
  }

  return;
}

void write_string_in_buffer(const char *str) {
  while (*str) {
    write_buffer((unsigned char)*str);
    str++;
  }
}

uint8_t is_pressed_key(char c) {
  if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
    c = (c < 'a') ? c - 'A' + 'a' : c; // Convertir a minúscula si es mayúscula
    return pressed_keys[c - 'a']; // Devuelve 1 si la tecla está presionada, 0 si no
  }
  return 0; // Si el char es inválido, retornamos 0
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
int copy_registers(register_info_t * buffer)
{
	if (!snapshot_saved) {
		return -1;
	}
	memcpy(buffer, reg_array, sizeof(register_info_t));
	return 0;
}