// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <processes.h>
#include <memory_manager.h>
#include "lib.h"
#include "scheduler.h"
#include "interrupts.h"
#include "pipes.h"
#include <stddef.h>

extern void  *setup_initial_stack(void *caller, int pid, void *stack_pointer, void *rcx);
static char **duplicate_argv(const char **argv, int argc);
static void   process_caller(int pid);
static int  init_pcb_argv(pcb_t *p, int argc, const char **argv);
static void init_pcb_file_descriptors(pcb_t *p, uint8_t read_fd, uint8_t write_fd);
static void free_pcb_argv(pcb_t *p);
static void free_pcb_stack(pcb_t *p);



int init_pcb_stack(pcb_t *p)
{
	p->stack_base = mm_alloc(PROCESS_STACK_SIZE);
	if (p->stack_base == NULL) {
		return -1;
	}
	p->stack_pointer = setup_initial_stack(
	        &process_caller, p->pid, (char *)p->stack_base + PROCESS_STACK_SIZE, 0);
	return 0;
}



static int init_pcb_argv(pcb_t *p, int argc, const char **argv)
{
	p->argc = argc;
	if (argc > 0 && argv != NULL) {
		p->argv = duplicate_argv(argv, argc);
		if (p->argv == NULL) {
			return -1;
		}
	} else {
		p->argv = NULL;
	}
	return 0;
}

static void init_pcb_file_descriptors(pcb_t *p, uint8_t read_fd, uint8_t write_fd)
{
	
	p->fd_table[STDIN] = read_fd;
	if (read_fd >= FIRST_FREE_FD) {
		if(open_fd(read_fd) < 0) {
			p->fd_table[STDIN] = STDIN;
		}
	}
	p->fd_table[STDOUT] = write_fd;
	if (write_fd >= FIRST_FREE_FD) {
		if (open_fd(write_fd) < 0) {
			p->fd_table[STDOUT] = STDOUT;
		}
	}
	

	// builtin fds
	for (int i = STDERR; i < FIRST_FREE_FD; i++) {
		p->fd_table[i] = i;
	}

	// the rest are closed
	for (int i = FIRST_FREE_FD; i < MAX_FDS; i++) {
		p->fd_table[i] = NO_FD;
	}
}

int init_pcb(pcb_t * p, process_entry_t entry, int argc, const char ** argv, const char * name,
		int parent_pid, uint8_t priority , uint8_t read_fd, uint8_t write_fd, uint8_t killable) {

	p->parent_pid = parent_pid;
	strncpy(p->name, name, MAX_PROCESS_NAME_LENGTH - 1);
	p->name[MAX_PROCESS_NAME_LENGTH - 1] = '\0';
	p->entry = entry;
	p->waiting_on = NO_PID;
	p->killable = killable;
	p->unblockable = 0;
	p->priority = priority;

	if (init_pcb_argv(p, argc, argv) == ERROR) {
		if (p->stack_base != NULL) {
			mm_free(p->stack_base);
		}
		mm_free(p);
		return -1;
	}

	init_pcb_file_descriptors(p, read_fd, write_fd);

	return 0;
}


int create_process(process_entry_t entry, int argc, const char **argv, 
						const char * name, process_attrs_t * attrs) {
	if (!entry || !name || argc < 0) {
		return -1;
	}

	pcb_t *p = mm_alloc(sizeof(pcb_t));
	if (!p) {
		return -1;
	}


	int parent_pid = sch_get_current_pid();
	uint8_t read_fd = STDIN;
	uint8_t write_fd = STDOUT;
	uint8_t priority = DEFAULT_PRIORITY;

	if (attrs != NULL) {
		read_fd = attrs->read_fd;
		write_fd = attrs->write_fd;
		priority = attrs->priority;
	} 

	if (init_pcb(p, entry, argc, argv, name, parent_pid, priority, read_fd, write_fd, 1) < 0) {
		return -1;
	}

	uint8_t foreground = attrs != NULL ? attrs->foreground : 0;

	int new_pid = sch_add_process(p, foreground);
	if (new_pid < 0) {
		free_process_resources(p);
	}
	return new_pid;
	
}

static void free_pcb_argv(pcb_t *p)
{
	if (p->argv != NULL) {
		for (int i = 0; i < p->argc; i++) {
			if (p->argv[i] != NULL) {
				mm_free(p->argv[i]);
			}
		}
		mm_free(p->argv);
		p->argv = NULL;
	}
}

static void free_pcb_stack(pcb_t *p)
{
	if (p->stack_base != NULL) {
		mm_free(p->stack_base);
		p->stack_base    = NULL;
		p->stack_pointer = NULL;
	}
}

void free_process_resources(pcb_t *p)
{
	if (p == NULL) {
		return;
	}



	free_pcb_argv(p);
	free_pcb_stack(p);

	// Liberar pcb_t
	mm_free(p);
}

static char **duplicate_argv(const char **argv, int argc)
{
	// Caso argv vacío o NULL: crear argv minimal con solo NULL
	if (argv == NULL || argc == 0 || (argc > 0 && argv[0] == NULL)) {
		char **new_argv = mm_alloc(sizeof(char *));
		if (new_argv == NULL) {
			return NULL;
		}
		new_argv[0] = NULL;
		return new_argv;
	}

	// Alocar array de punteros (argc + 1 para el NULL terminador)
	char **new_argv = mm_alloc((argc + 1) * sizeof(char *));
	if (new_argv == NULL) {
		return NULL;
	}

	// Copiar cada string
	for (int i = 0; i < argc; i++) {
		if (argv[i] == NULL) {
			new_argv[i] = NULL;
			continue;
		}

		size_t len  = strlen(argv[i]) + 1;
		new_argv[i] = mm_alloc(len);

		if (new_argv[i] == NULL) {
			// Error: liberar todo lo alocado hasta ahora (ROLLBACK)
			for (int j = 0; j < i; j++) {
				if (new_argv[j] != NULL) {
					mm_free(new_argv[j]);
				}
			}
			mm_free(new_argv);
			return NULL;
		}

		memcpy(new_argv[i], argv[i], len);
	}

	new_argv[argc] = NULL;
	return new_argv;
}

// check
static void process_caller(int pid)
{
	pcb_t *p = sch_get_pcb(pid);
	if (p == NULL || p->entry == NULL) {
		sch_exit_process(-1);
		return;
	}

	// Llamar a la función de entrada del proceso
	int res = p->entry(p->argc, p->argv);

	// Cuando retorna, terminar el proceso
	sch_exit_process(res);
}
