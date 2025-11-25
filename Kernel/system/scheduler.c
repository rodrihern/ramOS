// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "scheduler.h"
#include "processes.h"
#include "lib.h"
#include "queue.h"
#include "pipes.h"
#include "interrupts.h"
#include "video.h"
#include "../include/time.h"
#include <stddef.h>
#include "semaphores.h"

extern void timer_tick();

#define SHELL_ADDRESS ((void *)0x400000)

static pcb_t    *processes[MAX_PROCESSES];
static queue_t ready_queue[PRIORITY_COUNT] = {0};

static pid_t    current_pid            = NO_PID;
static uint8_t  process_count          = 0;
static uint64_t ticks        = 0;
static uint8_t     force_reschedule       = false;
static uint8_t     scheduler_initialized  = false;
static pid_t    foreground_process_pid = NO_PID;

static pcb_t        *pick_next_process(void);
static void        reparent_children_to_init(pid_t pid);
static int         init(int argc, char **argv);
static int         scheduler_add_init();
static inline uint8_t pid_is_valid(pid_t pid);
static void        cleanup_all_processes(void);
static int         create_shell();
static void        close_open_fds(pcb_t *p);
static void        apply_aging(void);
static int         terminate_process(pcb_t *p, int64_t ret_value);
static void        dequeue_if_ready(pcb_t *p);

static inline uint8_t pid_is_valid(pid_t pid)
{
	return pid >= 0 && pid <= MAX_PID;
}

static int get_next_free_pid(void)
{
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processes[i] == NULL) {
			return i;
		}
	}
	return NO_PID;
}

static void close_open_fds(pcb_t *p)
{
	for (int i = 0; i < MAX_FDS; i++) {
		int fd = p->fd_table[i];
		if (fd >= FIRST_FREE_FD) {
			close_fd(fd);
		}
	}
}

// Proceso init: arranca la shell y se queda haciendo halt para no consumir CPU (actúa como proceso
// idle). Se lo elige siempre que no haya otro proceso para correr!!!!
static int init(int argc, char **argv)
{
	if (create_shell() != 0) {
		return -1;
	}
	sch_set_foreground_process(SHELL_PID);
	while (1) {
		_hlt();
	}
	return -1;
}

pid_t sch_get_foreground_pid(void)
{
	if (!scheduler_initialized) {
		return NO_PID;
	}
	return foreground_process_pid;
}

int sch_set_foreground_process(pid_t pid)
{
	if (!scheduler_initialized) {
		return -1;
	}

	// Permitimos limpiar el foreground pasando NO_PID
	if (pid != NO_PID && !pid_is_valid(pid)) {
		return -1;
	}

	foreground_process_pid = pid;
	return 0;
}

static int create_shell()
{
	process_attrs_t attrs = {
		.read_fd    = STDIN,
		.write_fd   = STDOUT,
		.priority   = MAX_PRIORITY,
		.foreground = 1,
	};

	int pid = create_process((process_entry_t)SHELL_ADDRESS, 0, NULL, "shell", &attrs);
	if (pid != SHELL_PID) {
		return -1;
	}

	pcb_t *pcb_shell = sch_get_pcb(pid);
	if (pcb_shell != NULL) {
		pcb_shell->killable = 0;
	}
	return 0;
}

static int scheduler_add_init()
{
	if (process_count != 0) { // si no es el primer proceso en ser creado está mal
		return -1;
	}

	process_attrs_t attrs = {
		.read_fd    = STDIN,
		.write_fd   = STDOUT,
		.priority   = MIN_PRIORITY,
		.foreground = 0,
	};

	int pid = create_process((process_entry_t)init, 0, NULL, "init", &attrs);
	if (pid != INIT_PID) {
		return -1;
	}

	pcb_t *pcb_init = sch_get_pcb(pid);
	if (pcb_init != NULL) {
		pcb_init->killable = 0;
	}
	return 0;
}

int init_scheduler(void)
{
	if (scheduler_initialized) { // para no crearlo más de una vez
		return 0;
	}

	// Inicializar array de procesos
	for (int i = 0; i < MAX_PROCESSES; i++) {
		processes[i] = NULL;
	}

	// inicializamos las queues
	for (int i = MAX_PRIORITY; i <= MIN_PRIORITY; i++) {
		ready_queue[i] = q_init();
		if (ready_queue[i] == NULL) {
			return -1;
		}
	}

	process_count    = 0;
	ticks  = 0;
	force_reschedule = false;
	current_pid      = NO_PID; // NO le pongo INIT_PID porque el que lo tiene que elegir es el
	                           // schedule la primera vez que se llama.
	// Sino, la primera llamada a scheudule va a tratar a init como current y va a pisar su
	// stack_pointer con prev_rsp

	if (scheduler_add_init() != 0) {
		for (int i = MAX_PRIORITY; i <= MIN_PRIORITY; i++) {
			q_destroy(ready_queue[i]);
		}

		return -1;
	}

	scheduler_initialized = true;
	return 0;
}

void *schedule(void *prev_rsp)
{
	if (!scheduler_initialized) {
		return prev_rsp;
	}

	pcb_t *current = (pid_is_valid(current_pid)) ? processes[current_pid] : NULL;

	if (current) {
		current->stack_pointer = prev_rsp; // actualiza el rsp del proceso que estuvo
		                                   // corriendo hasta ahora en su pcb

		ticks++;

		// Cuando un proceso deja de correr (por cualquier razón), vuelve a su prioridad
		// base (pierde cualquier promoción temporal por aging)
		current->effective_priority = current->priority;

		// Con quantum de 1 tick, siempre forzamos reschedule después de cada tick
		// (a menos que el proceso ya no esté RUNNING por otra razón)

		if (current->status == PS_RUNNING) {
			// Si el status es RUNNING, cambiar a READY
			// Si fue bloqueado, terminado o matado, el status ya se cambió en otras
			// funciones
			current->status = PS_READY;
		}

		if (current->status == PS_READY && current->pid != INIT_PID) {
			// Agregar a la cola correspondiente a su prioridad efectiva (que ya fue
			// reseteada a priority)
			queue_t target_queue = ready_queue[current->effective_priority];
			if (!q_add(target_queue, current->pid)) {
				current->status  = PS_RUNNING;
				force_reschedule = false;
				return prev_rsp;
			}
		}
	}

	// Aplicar aging cada N ticks
	if (ticks % AGING_CHECK_INTERVAL == 0) {
		apply_aging();
	}

	// Si el proceso actual tiene que cambiar:
	pcb_t *next = pick_next_process();

	// Si no hay otro proceso listo, usar el proceso init como fallback
	if (!next) {
		next = processes[INIT_PID];
	}

	// Cuando un proceso va a correr:
	// Actualizar su last_tick para el control de aging
	next->last_tick = ticks;

	current_pid      = next->pid;
	next->status     = PS_RUNNING;
	force_reschedule = false;
	return next->stack_pointer;
}

// Devuelve null si no hay proceso listo para correr en ninguna cola
// Recorre las colas de mayor prioridad (0) a menor (PRIORITY_COUNT-1)
static pcb_t *pick_next_process(void)
{
	if (!scheduler_initialized || process_count == 0) {
		return NULL;
	}

	// Recorrer las colas por orden de prioridad
	for (int priority = MAX_PRIORITY; priority <= MIN_PRIORITY; priority++) {
		if (q_is_empty(ready_queue[priority])) {
			continue;
		}

		// Buscar un proceso válido en esta cola de prioridad
		while (!q_is_empty(ready_queue[priority])) {
			pid_t next_pid = (pid_t)q_poll(ready_queue[priority]);
			if (!pid_is_valid(next_pid)) {
				continue;
			}
			pcb_t *candidate = processes[next_pid];
			if (candidate != NULL && candidate->status == PS_READY) {
				return candidate;
			}
			// Si el proceso ya no está listo, seguimos buscando en esta cola
		}
	}

	return NULL;
}

// Aplica aging: promueve procesos que llevan mucho tiempo sin correr
static void apply_aging(void)
{
	// Recorrer desde MIN_PRIORITY hasta MAX_PRIORITY+1 (no promovemos desde MAX_PRIORITY)
	for (int i = MIN_PRIORITY; i > MAX_PRIORITY; i--) {
		if (q_is_empty(ready_queue[i])) {
			continue;
		}

		q_to_begin(ready_queue[i]);
		while (q_has_next(ready_queue[i])) {
			pid_t pid = q_next(ready_queue[i]);
			pcb_t  *p   = processes[pid];

			// Si el proceso no existe o no cumple el threshold, continuar
			if (p == NULL) {
				continue;
			}

			// Verificar si hace mucho que no corre (comparar con ticks)
			if (ticks - p->last_tick >= AGING_THRESHOLD) {
				// Promover: remover de esta cola y agregar a la de mayor prioridad
				q_remove_current(ready_queue[i]);
				p->effective_priority = i - 1;  // Subir un nivel de prioridad
				p->last_tick = ticks; // Actualizar last_tick para evitar
				                                // promociones repetidas
				q_add(ready_queue[i - 1], pid);
			}
		}
	}
}



// Saca el proceso de la cola READY si estaba listo/corriendo
static void dequeue_if_ready(pcb_t *p)
{
	if (p == NULL) {
		return;
	}
	if (p->status == PS_READY || p->status == PS_RUNNING) {
		q_remove(ready_queue[p->effective_priority], p->pid);
	}
}

int sch_add_process(pcb_t *p, uint8_t foreground)
{
	if (p == NULL || p->priority >= PRIORITY_COUNT) {
		return -1;
	}

	if (process_count >= MAX_PROCESSES ||
	    (!scheduler_initialized && ready_queue[DEFAULT_PRIORITY] == NULL)) {
		return -1;
	}

	p->pid = get_next_free_pid();

	if (!pid_is_valid(p->pid)) {
		return -1;
	}
	
	if (p->priority > MIN_PRIORITY) {
		p->priority = DEFAULT_PRIORITY;
	}

	p->effective_priority = p->priority;
	p->status             = PS_READY;
	p->last_tick          = ticks;

	if (init_pcb_stack(p) == ERROR) {
		return -1;
	}

	queue_t target_queue = ready_queue[p->effective_priority];
	if (target_queue == NULL) {
		return -1;
	}

	if (!q_add(target_queue, p->pid)) {
		return -1;
	}

	processes[p->pid] = p;
	process_count++;

	if (foreground) {
		sch_set_foreground_process(p->pid);
	}
	
	return p->pid;
}

int sch_remove_process(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return -1;
	}

	pcb_t *process = processes[pid];
	if (!process) {
		return -1;
	}

	// Remover de la cola de procesos listos para correr
	dequeue_if_ready(process);

	// Remover del array
	processes[pid] = NULL;
	process_count--;

	// Si estábamos ejecutando este proceso, forzar reschedule
	if (current_pid == pid) {
		current_pid = -1;
		sch_force_reschedule();
	}

	// Liberar recursos del proceso
	free_process_resources(process);

	return 0;
}

int sch_set_priority(pid_t pid, uint8_t new_priority)
{
	if (!scheduler_initialized || !pid_is_valid(pid) || processes[pid] == NULL ||
	    new_priority < MAX_PRIORITY ||
	    new_priority > MIN_PRIORITY) { // mas chico el numero, mayor la prioridad
		return -1;
	}

	pcb_t    *process                = processes[pid];
	uint8_t old_priority           = process->priority;
	uint8_t old_effective_priority = process->effective_priority;

	// Si la prioridad no cambia, no hacer nada
	if (old_priority == new_priority) {
		return 0;
	}

	// Si el proceso está READY, hay que moverlo de una cola a otra
	if (process->status == PS_READY) {
		// Remover de la cola actual (usa effective_priority porque ahí está realmente)
		q_remove(ready_queue[old_effective_priority], pid);

		// Cambiar ambas prioridades
		process->priority           = new_priority;
		process->effective_priority = new_priority;

		// Actualizar last_tick: el cambio manual de prioridad cuenta como "atención" del
		// scheduler
		process->last_tick = ticks;

		// Agregar a la nueva cola
		if (!q_add(ready_queue[new_priority], pid)) {
			// Si falla, volver a poner en la cola antigua
			process->priority           = old_priority;
			process->effective_priority = old_effective_priority;
			q_add(ready_queue[old_effective_priority], pid);
			return -1;
		}
	} else {
		// Si está RUNNING, BLOCKED o TERMINATED, solo cambiar la prioridad base
		// effective_priority se reseteará cuando vuelva a correr
		process->priority           = new_priority;
		process->effective_priority = new_priority;
	}

	return 0;
}

int sch_get_priority(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid) || processes[pid] == NULL) {
		return -1;
	}
	pcb_t *process = processes[pid];
	return process->priority;
}

void sch_force_reschedule(void)
{
	if (scheduler_initialized) {
		force_reschedule = true;
		timer_tick();
	}
}

int sch_get_current_pid(void)
{
	if (scheduler_initialized) {
		return current_pid;
	}
	return -1;
}

int sch_kill_process(pid_t pid)
{
	if (!scheduler_initialized) {
		return -1; // Scheduler not initialized
	}

	if (!pid_is_valid(pid)) {
		return -2; // Invalid PID
	}

	pcb_t *killed_process = processes[pid];
	if (!killed_process) {
		return -3; // No process found with this PID
	}

	if (!killed_process->killable) {
		return -4; // Process is protected (init or shell)
	}

	if (terminate_process(killed_process, KILLED_RET_VALUE)) {
		sch_force_reschedule();
	}
	return 0;
}

// Llamar desde proc_block() en processes.c
int sch_block_process(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return -1;
	}

	pcb_t *process = processes[pid];
	if (process == NULL) {
		return -1;
	}

	// Remover de cola READY (si está ahí)
	dequeue_if_ready(process);

	process->status = PS_BLOCKED;

	// Si es el proceso actual, forzar reschedule
	if (pid == current_pid) {
		sch_force_reschedule();
	}

	return 0;
}

int sch_ublock_process(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return -1;
	}

	pcb_t *process = processes[pid];
	if (!process || process->status != PS_BLOCKED) {
		return -1;
	}

	process->status = PS_READY;

	// Al desbloquearse, el proceso vuelve a su prioridad base
	// (por si fue promovido por aging mientras esperaba ser seleccionado, antes de bloquearse)
	process->effective_priority = process->priority;

	// Actualizar last_tick: el tiempo bloqueado no cuenta para aging
	// El proceso debe esperar en READY para acumular tiempo y ser promovido
	process->last_tick = ticks;

	// Agregar a la cola correspondiente a su prioridad efectiva
	if (!q_add(ready_queue[process->effective_priority], pid)) {
		process->status = PS_BLOCKED;
		return -1;
	}

	process->unblockable = 0;

	return 0;
}

static void cleanup_all_processes(void)
{
	if (!scheduler_initialized) {
		return;
	}

	for (int i = 0; i < MAX_PROCESSES; i++) {
		pcb_t *p = processes[i];
		if (p) {
			free_process_resources(p);
			processes[i] = NULL;
		}
	}
}

void scheduler_destroy(void)
{
	if (!scheduler_initialized) {
		return;
	}

	// Limpiar todas las colas de prioridades liberando nodos
	for (int i = MAX_PRIORITY; i <= MIN_PRIORITY; i++) {
		q_destroy(ready_queue[i]);
		ready_queue[i] = NULL;
	}

	cleanup_all_processes();

	process_count         = 0;
	ticks       = 0;
	force_reschedule      = false;
	current_pid           = NO_PID;
	scheduler_initialized = false;
}

// En scheduler.c
pcb_t *sch_get_pcb(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return NULL;
	}

	return processes[pid];
}

int get_processes_info(process_info_t *buffer, int max_count)
{
	if (!scheduler_initialized || buffer == NULL || max_count <= 0) {
		return -1;
	}

	int count = 0;
	for (int i = 0; i < MAX_PROCESSES && count < max_count; i++) {
		pcb_t *p = processes[i];
		if (p) {
			buffer[count].pid = p->pid;
			strncpy(buffer[count].name, p->name, MAX_PROCESS_NAME_LENGTH);
			buffer[count].status        = p->status;
			buffer[count].priority      = p->priority;
			buffer[count].parent_pid    = p->parent_pid;
			buffer[count].read_fd       = p->fd_table[STDIN];
			buffer[count].write_fd      = p->fd_table[STDOUT];
			buffer[count].stack_base    = (uint64_t)p->stack_base;
			buffer[count].stack_pointer = (uint64_t)p->stack_pointer;

			count++;
		}
	}

	return count;
}

// Lógica común de terminación (exit/kill)
static int terminate_process(pcb_t *p, int64_t ret_value)
{
	if (p == NULL) {
		return 0;
	}

	reparent_children_to_init(p->pid);

	if (p->pid == foreground_process_pid) {
		foreground_process_pid = SHELL_PID;
	}

	remove_process_from_all_semaphore_queues(p->pid);
	close_open_fds(p);

	// Si aún está en READY/RUNNING, quitarlo de la cola
	if (p->status == PS_READY || p->status == PS_RUNNING) {
		q_remove(ready_queue[p->effective_priority], p->pid);
	}

	p->status       = PS_TERMINATED;
	p->return_value = ret_value;

	// Si el padre es init, removemos el PCB; si no, lo dejamos para waitpid
	if (p->parent_pid == INIT_PID) {
		sch_remove_process(p->pid);
	} else {
		pcb_t *parent = processes[p->parent_pid];
		if (parent != NULL && parent->status == PS_BLOCKED && parent->waiting_on == p->pid) {
			sch_ublock_process(parent->pid);
		}
	}

	return (p->pid == current_pid);
}

void sch_exit_process(int64_t ret_value)
{
	if (!scheduler_initialized) {
		return;
	}

	pcb_t *current_process = processes[current_pid];
	if (current_process == NULL) {
		return;
	}

	terminate_process(current_process, ret_value);
	sch_force_reschedule();
}

// Bloquea al proceso actual hasta que el hijo indicado termine. Devuelve el status del hijo si ya
// terminó o 0.
int sch_waitpid(pid_t child_pid)
{
	if (!scheduler_initialized || !pid_is_valid(child_pid) || processes[child_pid] == NULL ||
	    processes[child_pid]->parent_pid != current_pid) {
		return -1;
	}

	// Si el proceso hijo no termino, bloqueamos el proceso actual hasta que termine
	if (processes[child_pid]->status != PS_TERMINATED) {
		processes[current_pid]->waiting_on = child_pid;
		sch_block_process(current_pid);
	}

	// Llega acá cuando el hijo terminó y lo desbloqueo o si el hijo ya había terminado

	processes[current_pid]->waiting_on = NO_PID;
	int ret_value                      = processes[child_pid]->return_value;
	sch_remove_process(child_pid);

	return ret_value;
}

int adopt_init_as_parent(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return -1;
	}
	pcb_t *init_process = processes[INIT_PID];
	if (init_process == NULL) {
		return -1;
	}

	pcb_t *orphan_process = processes[pid];
	if (orphan_process == NULL) {
		return -1;
	}

	// Si ya terminó, reapearlo al instante
	if (orphan_process->status == PS_TERMINATED) {
		return sch_remove_process(pid);
	}

	orphan_process->parent_pid = INIT_PID;
	return 0;
}

static void reparent_children_to_init(pid_t pid)
{
	if (!scheduler_initialized || !pid_is_valid(pid)) {
		return;
	}
	for (int i = 0; i < MAX_PROCESSES; i++) {
		if (processes[i] != NULL && processes[i]->parent_pid == pid) {
			// Asignar el proceso huérfano al init
			if (processes[i]->status == PS_TERMINATED) {
				// Si el hijo ya estaba terminado, lo removemos directamente
				sch_remove_process(processes[i]->pid);
			} else {
				processes[i]->parent_pid = INIT_PID;
			}
		}
	}
}

// Mata el proceso que está en foreground. Devuelve 0 en éxito, -1 en error.
int sch_kill_foreground_process(void)
{
	if (!scheduler_initialized) {
		return -1;
	}

	// Si no hay proceso en foreground, retornar error
	if (foreground_process_pid == NO_PID) {
		return -1;
	}

	int result = sch_kill_process(foreground_process_pid);
	// Después de matar el proceso en foreground, establecer la shell como proceso en foreground
	foreground_process_pid = SHELL_PID;

	return result;
}
