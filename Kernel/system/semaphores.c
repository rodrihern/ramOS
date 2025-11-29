// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "semaphores.h"
#include "scheduler.h"
#include "memory_manager.h"
#include "lib.h"
#include "processes.h"
#include "video.h"
#include "queue.h"
#include <stddef.h>

typedef struct {
	int               value; // Contador del semáforo
	uint32_t          owner_pids[MAX_PROCESSES];
	int               ref_count; // Cantidad de procesos usando este semáforo
	char              name[MAX_SEM_NAME_LENGTH];
	queue_t           queue;
	int               lock; // Spinlock simple para proteger acceso concurrente
} sem_t;

static sem_t *semaphores[MAX_SEMAPHORES] = {NULL};
static int          semaphore_count            = 0;
static int          semaphores_initialized     = 0;

static int64_t      get_free_id(void);
static sem_t *get_sem_by_name(const char *name);
static int          get_idx_by_name(const char *name);
static int          remove_process_from_queue(sem_t *sem, uint32_t pid);
static int64_t      sem_close_by_pid(char *name, uint32_t pid);
static void
init_semaphore_struct(sem_t *sem, const char *name, int initial_value, uint32_t owner_pid);

static int pid_present_in_semaphore(sem_t *sem, uint32_t pid)
{
	return sem->owner_pids[pid];
}

void init_semaphores(void)
{
	if (semaphores_initialized) {
		return;
	}

	for (int i = 0; i < MAX_SEMAPHORES; i++) {
		semaphores[i] = NULL;
	}

	semaphore_count        = 0;
	semaphores_initialized = 1;
}

int64_t sem_open(char *name, int initial_value)
{
	if (!semaphores_initialized || name == NULL) {
		return -1;
	}

	// primero verificar si ya existe
	sem_t *sem = get_sem_by_name(name);
	if (sem != NULL) {
		// Ya existe, solo incrementar contador si no es un proceso que ya lo creo
		if (pid_present_in_semaphore(sem, sch_get_current_pid())) {
			return ERROR; // El proceso ya posee este semáforo
		}
		acquire_lock(&sem->lock);
		sem->owner_pids[sch_get_current_pid()] = 1;
		sem->ref_count++;
		release_lock(&sem->lock);
		return OK;
	}

	// Buscar slot libre
	int64_t id = get_free_id();
	if (id == -1) {
		return ERROR;
	}

	// Crear nuevo semáforo
	sem = mm_alloc(sizeof(sem_t));
	if (sem == NULL) {
		return ERROR;
	}

	sem->queue = q_init();
	if (sem->queue == NULL) {
		mm_free(sem);
		return ERROR;
	}

	init_semaphore_struct(sem, name, initial_value, sch_get_current_pid());

	semaphores[id] = sem;
	semaphore_count++;

	return OK;
}

int64_t sem_close(char *name)
{
	if (!semaphores_initialized || name == NULL) {
		return ERROR;
	}

	int idx = get_idx_by_name(name);
	if (idx == ERROR) {
		return ERROR;
	}

	sem_t *sem = semaphores[idx];

	acquire_lock(&sem->lock);

	if (sem->ref_count > 1) {
		sem->ref_count--;
		sem->owner_pids[sch_get_current_pid()] = FREE;
		release_lock(&sem->lock);
		return OK;
	}

	release_lock(&sem->lock);

	// Último proceso usando el semáforo, destruirlo
	q_destroy(sem->queue);
	mm_free(sem);
	semaphores[idx] = NULL;
	semaphore_count--;

	return OK;
}

int64_t sem_wait(char *name)
{
	if (!semaphores_initialized || name == NULL) {
		return ERROR;
	}

	sem_t *sem = get_sem_by_name(name);
	if (sem == NULL) {
		return ERROR;
	}

	acquire_lock(&sem->lock);

	if (sem->value > 0) {
		sem->value--;
		release_lock(&sem->lock);
		return OK;
	}

	// No hay recursos disponibles, bloquear proceso
	int pid = sch_get_current_pid();

	if (!q_add(sem->queue, pid)) {
		release_lock(&sem->lock);
		return ERROR;
	}

	// _cli();

	release_lock(&sem->lock);

	sch_block_process(pid);

	// _sti();

	return 0;
}

int64_t sem_post(char *name)
{
	if (!semaphores_initialized || name == NULL) {
		return ERROR;
	}

	sem_t *sem = get_sem_by_name(name);
	if (sem == NULL) {
		return ERROR;
	}

	acquire_lock(&sem->lock);

	if (!q_is_empty(sem->queue)) {
		// Hay procesos esperando, desbloquear uno
		int pid = q_poll(sem->queue);
		if (pid < 0) {
			release_lock(&sem->lock);
			return ERROR;
		}
		_cli(); // deshabilitar interrupciones
		release_lock(&sem->lock);
		sch_ublock_process((uint32_t) pid);
		_sti(); // habilitar interrupciones
	} else {
		// No hay procesos esperando, incrementar contador
		sem->value++;
		release_lock(&sem->lock);
	}

	return OK;
}

int64_t sem_reset(char *name, int new_value)
{
	if (!semaphores_initialized || name == NULL || new_value < 0) {
		return ERROR;
	}

	sem_t *sem = get_sem_by_name(name);
	if (sem == NULL) {
		return ERROR;
	}

	uint32_t to_unblock[MAX_PROCESSES];
	uint32_t unblock_count = 0;

	acquire_lock(&sem->lock);

	// Desencolar procesos hasta agotar new_value o la cola
	while (!q_is_empty(sem->queue) && new_value > 0) {
		int pid = q_poll(sem->queue);
		if (pid < 0) {
			break;
		}
		to_unblock[unblock_count++] = (uint32_t) pid;
		new_value--;
	}

	// Ajustar contador a lo que quede disponible
	sem->value = new_value;
	release_lock(&sem->lock);

	// Desbloquear procesos seleccionados
	for (uint32_t i = 0; i < unblock_count; i++) {
		_cli();
		sch_ublock_process(to_unblock[i]);
		_sti();
	}

	return OK;
}

int remove_process_from_all_semaphore_queues(uint32_t pid)
{
	if (!semaphores_initialized) {
		return ERROR;
	}

	for (int i = 0; i < MAX_SEMAPHORES; i++) {
		sem_t *sem = semaphores[i];
		if (sem == NULL) {
			continue;
		}

		if (sem->owner_pids[pid] == OCCUPIED) {
			if (sch_get_pcb(pid)->status == PS_BLOCKED) {
				acquire_lock(&sem->lock);
				remove_process_from_queue(sem, pid);
				release_lock(&sem->lock);
			}
			sem_close_by_pid(sem->name, pid);
		}
	}

	return OK;
}

static void
init_semaphore_struct(sem_t *sem, const char *name, int initial_value, uint32_t owner_pid)
{
	sem->value = initial_value;
	strncpy(sem->name, name, MAX_SEM_NAME_LENGTH - 1);
	sem->name[MAX_SEM_NAME_LENGTH - 1] = '\0';
	sem->lock                          = 1; // Spinlock desbloqueado
	sem->ref_count                     = 1;
	for (int i = 0; i < MAX_PROCESSES; i++) {
		sem->owner_pids[i] = FREE;
	}
	sem->owner_pids[owner_pid] = OCCUPIED;
}

static int64_t get_free_id(void)
{
	for (int i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphores[i] == NULL) {
			return i;
		}
	}
	return ERROR;
}

static int get_idx_by_name(const char *name)
{
	for (int i = 0; i < MAX_SEMAPHORES; i++) {
		if (semaphores[i] != NULL && strcmp(semaphores[i]->name, name) == 0) {
			return i;
		}
	}
	return ERROR;
}

static sem_t *get_sem_by_name(const char *name)
{
	int idx = get_idx_by_name(name);
	if (idx == ERROR) {
		return NULL;
	}
	return semaphores[idx];
}

static int remove_process_from_queue(sem_t *sem, uint32_t pid)
{
	if (sem == NULL || sem->queue == NULL) {
		return ERROR;
	}

	return q_remove(sem->queue, pid) ? OK : ERROR;
}

static int64_t sem_close_by_pid(char *name, uint32_t pid)
{
	if (!semaphores_initialized || name == NULL) {
		return ERROR;
	}

	int idx = get_idx_by_name(name);
	if (idx == ERROR) {
		return ERROR;
	}

	sem_t *sem = semaphores[idx];

	acquire_lock(&sem->lock);

	if (sem->ref_count > 1) {
		sem->ref_count--;
		sem->owner_pids[pid] = FREE;
		release_lock(&sem->lock);
		return OK;
	}

	release_lock(&sem->lock);

	// Ultimo proceso usando el semaforo, destruirlo
	q_destroy(sem->queue);
	mm_free(sem);
	semaphores[idx] = NULL;
	semaphore_count--;

	return OK;
}
