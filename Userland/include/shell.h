#ifndef SHELL_H
#define SHELL_H
#include "usrlib.h"
#include "syscalls.h"

typedef void (*command_handler_t)(int argc, char **argv);

typedef struct {
	char             *name;
	char             *description;
	command_handler_t handler;

} builtin_command_t;

typedef struct {
	char           *name;
	char           *description;
	process_entry_t entry;
} external_program_t;

#define INPUT_MAX 128
#define PROMPT "> "
#define ERROR_MSG "Use command \'help\' to see available commands\n"
#define HELP_MESSAGE "--Write help to see available commands--\n"
#define USERNAME_MAX_LENGTH 16

#define WELCOME_MESSAGE "Welcome to ramOS"
#define WELCOME_MESSAGE_LENGTH 16

#define PRESS_KEY_MESSAGE "Press any key to continue..."
#define PRESS_KEY_MESSAGE_LENGTH 28

#define INITIAL_SCREEN_TEXT_COLOR 0xff8000

// funciones de la shell
void read_line(char *buf, uint64_t max);
void process_line(char *line);
void set_username(const char *new_name);

// comandos builtin
void cls_cmd(int argc, char *argv[]);
void help_cmd(int argc, char *argv[]);
void username_cmd(int argc, char *argv[]);
void mute_cmd(int argc, char *argv[]);

#endif