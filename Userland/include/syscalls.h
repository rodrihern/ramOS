#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include <stddef.h>

#define OK 0
#define ERROR -1
#define EOF -1
#define MAX_NAME_LENGTH 32
#define MAX_PROCESSES 64

#define MIN_PRIORITY 2
#define DEFAULT_PRIORITY 1
#define MAX_PRIORITY 0

#define MAX_PIPES 32
#define MAX_PIPE_NAME_LENGTH 32

enum { 
	STDIN = 0,
	STDOUT,
	STDERR,
	STDGREEN,
	STDBLUE,
	STDCYAN,
	STDMAGENTA,
	STDYELLOW,
	FDS_COUNT
};

typedef struct register_info {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} register_info_t;

typedef struct time_info {
    uint8_t hour;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} time_info_t;

typedef struct video_info {
	uint16_t width;
	uint16_t height;
	uint16_t pitch;
	uint8_t bpp;
} video_info_t;

typedef struct region {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
} region_t;


typedef struct mem_info {
	uint64_t total_memory;
	uint64_t used_memory;
	uint64_t free_memory;
	uint64_t allocated_blocks;
} mem_info_t;

typedef int (*process_entry_t)(int argc, char **argv);

typedef enum { PS_READY = 0, PS_RUNNING, PS_BLOCKED, PS_TERMINATED } process_status_t;

typedef struct process_info {
	int              pid;
	char             name[MAX_NAME_LENGTH];
	process_status_t status;
	uint8_t          priority;
	int              parent_pid;
	int              read_fd;
	int              write_fd;
	uint64_t         stack_base;
	uint64_t         stack_pointer;
} process_info_t;

typedef struct process_attrs {
	uint8_t read_fd;
	uint8_t write_fd;
	uint8_t priority;
	uint8_t foreground; // 1 = fg, 0 = bg
} process_attrs_t;

typedef struct pipe_info {
	int  id;
	char name[MAX_PIPE_NAME_LENGTH];
	int  read_fd;
	int  write_fd;
	int  readers;
	int  writers;
	int  buffered;
} pipe_info_t;

// registers
extern int sys_register_snapshot(register_info_t *buffer);

// io
extern int sys_read(int fd, char *buf, uint64_t count);
extern int sys_write(int fd, const char *buf, uint64_t count);
extern int sys_flush(int fd);

// tty
extern void sys_tty_show(void);
extern void sys_tty_clear(void);
extern void sys_tty_set_bgcolor(uint32_t color);
extern void sys_tty_set_textcolor(uint32_t color);
extern int sys_tty_get_fontsize(void);
extern void sys_tty_set_fontsize(int size);

// video
extern int sys_video_info(video_info_t *buffer);
extern void sys_present(void *framebuffer);
extern void sys_present_region(void *framebuffer, region_t *region);
extern void sys_present_nregions(void *framebuffer, region_t *regions[], uint64_t n);

// keyboard
extern uint8_t sys_is_pressed(uint8_t scancode);

// time
extern void sys_sleep(uint64_t miliseconds);
extern uint64_t sys_ms_elapsed(void);
extern uint64_t sys_ticks_elapsed(void);
extern int sys_time_info(time_info_t *buffer);

// sound
extern void sys_speaker_start(uint32_t freq_hz);
extern void sys_speaker_stop(void);

// memory
extern void *sys_malloc(uint64_t size);
extern void sys_free(void *ptr);
extern int sys_mem_info(mem_info_t *buffer);

// processes
extern int64_t sys_create_process(void *entry, int argc, const char **argv, const char *name, process_attrs_t *attrs);
extern void sys_exit(int status);
extern int64_t sys_getpid(void);
extern int64_t sys_kill(int pid);
extern int64_t sys_block(int pid);
extern int64_t sys_unblock(int pid);
extern int64_t sys_wait(int pid);
extern int64_t sys_nice(int pid, int new_prio);
extern void sys_yield(void);
extern int sys_set_foreground_process(int pid);
extern int sys_adopt_init_as_parent(int pid);
extern int sys_get_foreground_process(void);
extern int sys_processes_info(process_info_t *buf, int max_count);

// semaphores
extern int64_t sys_sem_open(const char *name, int value);
extern void sys_sem_close(const char *name);
extern void sys_sem_wait(const char *name);
extern void sys_sem_post(const char *name);

// pipes
extern int sys_create_pipe(int fds[2]);
extern void sys_destroy_pipe(int fd);
extern int sys_open_named_pipe(char *name, int fds[2]);
extern int sys_close_fd(int fd);
extern int sys_pipes_info(pipe_info_t *buf, int max_count);

#endif
