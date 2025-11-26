// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "../include/shell.h"

#define UP_ARROW 1
#define DOWN_ARROW 4
#define HISTORY_SIZE 10  // Número de comandos a guardar en el historial

// Buffer para el comando actual
static char current_input[INPUT_MAX];
static char user_name[INPUT_MAX];

// Historial circular de comandos
static char history[HISTORY_SIZE][INPUT_MAX];
static int history_write_idx = 0;  // Dónde escribir el próximo comando
static int history_count = 0;       // Cuántos comandos tenemos (max HISTORY_SIZE)
static int history_nav_idx = -1;    // Posición actual al navegar (-1 = no navegando)

static void print_initial_message();
static void add_to_history(const char *cmd);
static const char *get_history_up(void);
static const char *get_history_down(void);

int main(void)
{
	sys_textmode();

	// Limpiar buffer de teclado y resetear semáforo antes de empezar
	sys_clear_input_buffer();

	print_initial_message();

	// Limpiar buffer de teclado y resetear semáforo antes de empezar
	sys_clear_input_buffer();

	while (1) {
		fprint(STDCYAN, user_name);
		print(PROMPT);
		read_line(current_input, INPUT_MAX - 1);
		putchar('\n');
		
		// Solo agregar al historial si no está vacío
		if (current_input[0] != '\0') {
			add_to_history(current_input);
		}
		
		process_line(current_input);
	}
}

/*-- FUNCIONES AUXILIARES --*/
static void print_initial_message()
{
	sys_increase_fontsize();
	fprint(STDMAGENTA, INITIAL_MESSAGE_1);
	putchar('\n');
	print(INITIAL_MESSAGE_2);
	read_line(user_name, USERNAME_MAX_LENGTH - 1);
	putchar('\n');
	fprint(STDMAGENTA, HELP_MESSAGE);
	putchar('\n');
	sys_decrease_fontsize();
}

void set_username(const char *new_name)
{
	int i;
	for (i = 0; i < USERNAME_MAX_LENGTH - 1 && new_name[i] != '\0'; i++) {
		user_name[i] = new_name[i];
	}
	user_name[i] = '\0';
}

void read_line(char *buf, uint64_t max)
{
	char     c;
	uint32_t idx = 0;

	// Limpiar el buffer al inicio
	for (int i = 0; i <= max; i++) {
		buf[i] = '\0';
	}

	// Resetear navegación del historial
	history_nav_idx = -1;

	while ((c = getchar()) != '\n') {
		if (c <= 0) {
			continue;
		}
		if (c == '\b') { // Backspace
			if (idx != 0) {
				idx--;
				buf[idx] = '\0'; // Limpiar el caracter borrado
				putchar('\b');   // borro el caracter
			}
		} else if (c == UP_ARROW) {
			const char *history_cmd = get_history_up();
			if (history_cmd != NULL) {
				while (idx-- > 0) {
					putchar('\b');
				}
				// Escribir comando del historial
				strcpy(buf, history_cmd);
				for (idx = 0; buf[idx]; idx++) {
					putchar(buf[idx]);
				}
			}
		} else if (c == DOWN_ARROW) {
			const char *history_cmd = get_history_down();
			// Borrar línea actual
			while (idx > 0) {
				idx--;
				putchar('\b');
			}
			
			if (history_cmd != NULL) {
				// Escribir comando del historial
				strcpy(buf, history_cmd);
				for (idx = 0; buf[idx]; idx++) {
					putchar(buf[idx]);
				}
			} else {
				// Volver a línea vacía
				buf[0] = '\0';
				idx = 0;
			}
		} else if (idx < max) {
			buf[idx++] = c;
			putchar(c);      // escribo el caracter
		}
	}

	buf[idx] = 0;
}

void incfont_cmd(int argc, char * argv[])
{
	sys_increase_fontsize();
	sys_clear();
}

void decfont_cmd(int argc, char * argv[])
{
	sys_decrease_fontsize();
	sys_clear();
}

void cls_cmd(int argc, char *argv[]) {
	sys_clear();
}

void username_cmd(int argc, char *argv[])
{
	if (argc < 1) {
		print("Usage: username <new_name>\n");
		return;
	}

	int offset = 0;
	for (int i = 0; i < argc && offset < USERNAME_MAX_LENGTH - 1; i++) {
		if (i > 0) {
			user_name[offset++] = ' ';
		}

		int j = 0;
		while (argv[i][j] != '\0' && offset < USERNAME_MAX_LENGTH - 1) {
			user_name[offset++] = argv[i][j++];
		}
	}
	user_name[offset] = '\0';

	print("Username updated to: ");
	print(user_name);
	putchar('\n');
}

void mute_cmd(int argc, char *argv[]) {
	sys_speaker_stop();
	print("Speaker stopped\n");
}

// Funciones del historial
static void add_to_history(const char *cmd)
{
	// No agregar comandos vacíos o duplicados del último
	if (cmd[0] == '\0') {
		return;
	}
	
	// Verificar si es igual al último comando
	if (history_count > 0) {
		int last_idx = (history_write_idx - 1 + HISTORY_SIZE) % HISTORY_SIZE;
		if (strcmp(history[last_idx], (char *)cmd) == 0) {
			return;
		}
	}
	
	// Agregar comando al historial
	strcpy(history[history_write_idx], cmd);
	history_write_idx = (history_write_idx + 1) % HISTORY_SIZE;
	
	if (history_count < HISTORY_SIZE) {
		history_count++;
	}
	
	// Resetear navegación
	history_nav_idx = -1;
}

static const char *get_history_up(void)
{
	if (history_count == 0) {
		return NULL;
	}
	
	// Primera vez que presionan UP: empezar desde el más reciente
	if (history_nav_idx == -1) {
		history_nav_idx = (history_write_idx - 1 + HISTORY_SIZE) % HISTORY_SIZE;
		return history[history_nav_idx];
	}
	
	// Calcular índice del comando más antiguo disponible
	int oldest_idx = (history_write_idx - history_count + HISTORY_SIZE) % HISTORY_SIZE;
	
	// Si ya estamos en el más antiguo, no hacer nada
	if (history_nav_idx == oldest_idx) {
		return history[history_nav_idx];
	}
	
	// Retroceder en el historial
	history_nav_idx = (history_nav_idx - 1 + HISTORY_SIZE) % HISTORY_SIZE;
	return history[history_nav_idx];
}

static const char *get_history_down(void)
{
	if (history_count == 0 || history_nav_idx == -1) {
		return NULL;
	}
	
	// Calcular el índice del comando más reciente
	int newest_idx = (history_write_idx - 1 + HISTORY_SIZE) % HISTORY_SIZE;
	
	// Si ya estamos en el más reciente, volver a línea vacía
	if (history_nav_idx == newest_idx) {
		history_nav_idx = -1;
		return NULL;
	}
	
	// Avanzar en el historial
	history_nav_idx = (history_nav_idx + 1) % HISTORY_SIZE;
	return history[history_nav_idx];
}
