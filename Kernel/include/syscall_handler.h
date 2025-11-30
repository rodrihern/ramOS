#ifndef _SYSCALL_HANDLER_H_
#define _SYSCALL_HANDLER_H_

#include <stdint.h>
#include <stddef.h>
#include "video.h"
#include "timer.h"
#include "keyboard.h"
#include "sound.h"
#include "memory_manager.h"
#include "scheduler.h"
#include "semaphores.h"
#include "interrupts.h"
#include "processes.h"
#include "pipes.h"

// registers
static int sys_register_snapshot(register_info_t *buffer);

// io
static int sys_read(int fd, char *buf, uint64_t count);
static int sys_write(int fd, const char *buf, uint64_t count);
static int sys_flush(int fd);

// tty
static void sys_tty_clear();
static void sys_tty_show();
static void sys_tty_set_bgcolor(uint32_t color);
static void sys_tty_set_textcolor(uint32_t color);
static int sys_tty_get_fontsize();
static void sys_tty_set_fontsize(int size);

// video
static int sys_video_info(video_info_t *buffer);
static void sys_present(void * framebuffer);
static void sys_present_region(void * framebuffer, region_t * region);
static void sys_present_nregions(void * framebuffer, region_t * regions[], uint64_t n);

// keyboard
static uint8_t sys_is_pressed(uint8_t scancode);

// time
static void sys_sleep(uint64_t miliseconds);
static uint64_t sys_ms_elapsed();
static uint64_t sys_ticks_elapsed();
static int sys_time_info(time_info_t *buffer);

// sound
static void sys_speaker_start(uint32_t freq_hz);
static void sys_speaker_stop();

// memory
static void *sys_malloc(uint64_t size);
static void sys_free(void *ptr);
static int sys_mem_info(mem_info_t *buffer);

// processes
static int sys_create_process(void *entry, int argc, const char **argv, const char *name, process_attrs_t * attrs);
static void sys_exit(int status);
static int sys_getpid(void);
static int sys_kill(int pid);
static int sys_block(int pid);
static int sys_unblock(int pid);
static int sys_wait(int pid);
static int sys_nice(int pid, int new_prio);
static void sys_yield();
static int sys_set_foreground_process(int pid);
static int sys_adopt_init_as_parent(int pid);
static int sys_get_foreground_process(void);
static int sys_processes_info(process_info_t *buf, int max_count);

// semaphores
static int sys_sem_open(const char *name, int value);
static void sys_sem_close(const char *name);
static void sys_sem_wait(const char *name);
static void sys_sem_post(const char *name);

// pipes
static int sys_create_pipe(int fds[2]);
static void sys_destroy_pipe(int id);
static int sys_open_named_pipe(char *name, int fds[2]);
static int sys_close_fd(int fd);
static int  sys_pipes_info(pipe_info_t *buf, int max_count);

#endif