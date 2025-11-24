// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <processes.h>
#include <memory_manager.h>
#include "lib.h"
#include "scheduler.h"
#include "interrupts.h"
#include "pipes.h"

extern void  *setup_initial_stack(void *caller, int pid, void *stack_pointer, void *rcx);
static char **duplicate_argv(const char **argv, int argc, memory_manager_ADT mm);
static void   process_caller(int pid);
static int  init_pcb_stack(pcb_t *p, memory_manager_ADT mm);
static int  init_pcb_argv(pcb_t *p, int argc, const char **argv, memory_manager_ADT mm);
static void init_pcb_file_descriptors(pcb_t *p, uint8_t read_fd, uint8_t write_fd);
static void free_pcb_argv(pcb_t *p, memory_manager_ADT mm);
static void free_pcb_stack(pcb_t *p, memory_manager_ADT mm);



static int init_pcb_stack(pcb_t *p, memory_manager_ADT mm)
{
	p->stack_base = mm_alloc(mm, PROCESS_STACK_SIZE);
	if (p->stack_base == NULL) {
		return ERROR;
	}
	p->stack_pointer = setup_initial_stack(
	        &process_caller, p->pid, (char *)p->stack_base + PROCESS_STACK_SIZE, 0);
	return OK;
}

static int init_pcb_argv(pcb_t *p, int argc, const char **argv, memory_manager_ADT mm)
{
	p->argc = argc;
	if (argc > 0 && argv != NULL) {
		p->argv = duplicate_argv(argv, argc, mm);
		if (p->argv == NULL) {
			return ERROR;
		}
	} else {
		p->argv = NULL;
	}
	return OK;
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


int create_process(process_entry_t entry, int argc, const char **argv, 
						const char * name, process_attrs_t * attrs) {
	if (!entry || !name || argc < 0) {
		return -1;
	}

	pid_t pid = sch_next_pid();
	if (pid == NO_PID) {
		return -1;
	}

	memory_manager_ADT mm = get_kernel_memory_manager();

	pcb_t *p = mm_alloc(mm, sizeof(pcb_t));
	if (!p) {
		return -1;
	}

	p->pid = pid;
	p->parent_pid = sch_get_current_pid();
	strncpy(p->name, name, MAX_PROCESS_NAME_LENGTH - 1);
	p->name[MAX_PROCESS_NAME_LENGTH - 1] = '\0';
	p->entry = entry;
	p->return_value = 0;
	p->waiting_on = NO_PID;
	p->killable = 1;
	p->unblockable = 0;

	if (init_pcb_stack(p, mm) == ERROR) {
		mm_free(mm, p);
		return -1;
	}

	if (init_pcb_argv(p, argc, argv, mm) == ERROR) {
		mm_free(mm, p->stack_base);
		mm_free(mm, p);
		return -1;
	}

	if (attrs == NULL) {
		init_pcb_file_descriptors(p, STDIN, STDOUT);
		p->priority = DEFAULT_PRIORITY;
	} else {
		init_pcb_file_descriptors(p, attrs->read_fd, attrs->write_fd);
		p->priority = attrs->priority < PRIORITY_COUNT ? attrs->priority : DEFAULT_PRIORITY;
	}

	uint8_t foreground = attrs != NULL ? attrs->foreground : 0;

	int new_pid = sch_add_process(p, foreground);
	if (new_pid < 0) {
		free_process_resources(p);
	}
	return new_pid;
	
}

static void free_pcb_argv(pcb_t *p, memory_manager_ADT mm)
{
	if (p->argv != NULL) {
		for (int i = 0; i < p->argc; i++) {
			if (p->argv[i] != NULL) {
				mm_free(mm, p->argv[i]);
			}
		}
		mm_free(mm, p->argv);
		p->argv = NULL;
	}
}

static void free_pcb_stack(pcb_t *p, memory_manager_ADT mm)
{
	if (p->stack_base != NULL) {
		mm_free(mm, p->stack_base);
		p->stack_base    = NULL;
		p->stack_pointer = NULL;
	}
}

void free_process_resources(pcb_t *p)
{
	if (p == NULL) {
		return;
	}

	memory_manager_ADT mm = get_kernel_memory_manager();

	free_pcb_argv(p, mm);
	free_pcb_stack(p, mm);

	// Liberar pcb_t
	mm_free(mm, p);
}

static char **duplicate_argv(const char **argv, int argc, memory_manager_ADT mm)
{
	// Caso argv vacío o NULL: crear argv minimal con solo NULL
	if (argv == NULL || argc == 0 || (argc > 0 && argv[0] == NULL)) {
		char **new_argv = mm_alloc(mm, sizeof(char *));
		if (new_argv == NULL) {
			return NULL;
		}
		new_argv[0] = NULL;
		return new_argv;
	}

	// Alocar array de punteros (argc + 1 para el NULL terminador)
	char **new_argv = mm_alloc(mm, (argc + 1) * sizeof(char *));
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
		new_argv[i] = mm_alloc(mm, len);

		if (new_argv[i] == NULL) {
			// Error: liberar todo lo alocado hasta ahora (ROLLBACK)
			for (int j = 0; j < i; j++) {
				if (new_argv[j] != NULL) {
					mm_free(mm, new_argv[j]);
				}
			}
			mm_free(mm, new_argv);
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
