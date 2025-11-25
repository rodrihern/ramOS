#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "processes.h"

typedef int pid_t;

#define MAX_PRIORITY 0
#define DEFAULT_PRIORITY 1
#define MIN_PRIORITY 2
#define PRIORITY_COUNT 3

// Aging constants
#define AGING_CHECK_INTERVAL 10 // Cada cuántos ticks aplicar aging
#define AGING_THRESHOLD 50      // Ticks sin correr para ser promovido

// Inicialización
int  init_scheduler(void);
void scheduler_destroy(void);

// Función principal del scheduler (llamada por timer interrupt)
void *schedule(void *prev_rsp);

// Gestión de procesos
int sch_add_process(pcb_t * pcb, uint8_t foreground);
int  sch_remove_process(pid_t pid);
int  sch_set_priority(pid_t pid, uint8_t priority);
int  sch_get_priority(pid_t pid);
int  sch_kill_process(pid_t pid);
pcb_t *sch_get_pcb(pid_t pid);
void sch_exit_process(int64_t retValue);
int  sch_waitpid(pid_t child_pid);

// Bloqueo/desbloqueo (para usar desde processes.c)
int sch_block_process(pid_t pid);
int sch_ublock_process(pid_t pid);

// Control de scheduling
void sch_force_reschedule(void);
int  sch_get_current_pid(void);

// Información de procesos
int get_processes_info(process_info_t *buffer, int max_count);

// Foreground process control (getter/setter)
pid_t sch_get_foreground_pid(void);
int   sch_set_foreground_process(pid_t pid);
int   sch_kill_foreground_process(void);

// Reparent single process to INIT
int adopt_init_as_parent(pid_t pid);

#endif 
