// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


#include "syscall_handler.h"


#define MIN_CHAR 0
#define MAX_CHAR 256

void *syscalls[] = {
	// syscalls de arqui
	&sys_read,               // 0
	&sys_write,              // 1
	&sys_regs,               // 2
	&sys_time_info,          // 3
	&sys_increase_fontsize,  // 4
	&sys_decrease_fontsize,  // 5
	&sys_beep,               // 6
	&sys_screensize,         // 7
	&sys_circle,             // 8
	&sys_rectangle,          // 9
	&sys_draw_line,          // 10
	&sys_draw_string,        // 11
	&sys_clear,              // 12
	&sys_speaker_start,      // 13
	&sys_speaker_stop,       // 14
	&sys_textmode,           // 15
	&sys_videomode,          // 16
	&sys_put_pixel,          // 17
	&sys_key_status,         // 18
	&sys_sleep,              // 19
	&sys_clear_input_buffer, // 20
	&sys_ticks,              // 21

	// syscalls de memoria
	&sys_malloc,   // 22
	&sys_free,     // 23
	&sys_mem_info, // 24

	// syscalls de procesos
	&sys_create_process, // 25
	&sys_exit,           // 26
	&sys_getpid,         // 27
	&sys_kill,           // 28
	&sys_block,          // 29
	&sys_unblock,        // 30
	&sys_wait,           // 31
	&sys_nice,           // 32
	&sys_yield,          // 33
	&sys_processes_info, // 34

	// syscalls de semaforos
	&sys_sem_open,  // 35
	&sys_sem_close, // 36
	&sys_sem_wait,  // 37
	&sys_sem_post,  // 38

	// syscalls de pipes
	&sys_create_pipe,  // 49
	&sys_destroy_pipe, // 40

	&sys_set_foreground_process, // 41
	&sys_adopt_init_as_parent,   // 42
	&sys_get_foreground_process, // 43

	&sys_open_named_pipe, // 44
	&sys_close_fd,        // 45
	&sys_pipes_info,      // 46
};

uint64_t syscall_count = sizeof(syscalls) / sizeof(syscalls[0]);

static uint64_t sys_regs(char *buffer)
{
	return copy_registers(buffer);
}

// devuelve cuantos chars escribió
static int sys_write(int fd, const char *buffer, uint64_t count)
{
	if (fd < 0 || fd >= MAX_FDS) {
		return -1;
	}
	int  pid = scheduler_get_current_pid();
	pcb_t *p   = scheduler_get_pcb(pid);

	fd = p->fd_table[fd];

	if (fd <= STDIN) {
		return -1;
	}

	if (fd < FIRST_FREE_FD) {
		uint32_t color = fd_colors[fd];
		for (int i = 0; i < count; i++) {
			vd_put_char(buffer[i], color);
		}

		return count;
	}

	// es un pipe
	return write_pipe(fd, buffer, count);
}

// leo hasta count
static int sys_read(int fd, char *buffer, uint64_t count)
{
	if (fd < 0 || fd >= MAX_FDS) {
		return -1;
	}

	int pid = scheduler_get_current_pid();
	pcb_t *p = scheduler_get_pcb(pid);

	fd = p->fd_table[fd];

	if (fd < 0 || (STDOUT <= fd && fd < FIRST_FREE_FD)) {
		return -1;
	}

	if (fd == STDIN) { // quiere leer de teclado
		int foreground_pid = scheduler_get_foreground_pid();
		if (pid != foreground_pid) {
			return -1; // solo el proceso de foreground puede leer del teclado
		}
		return read_keyboard_buffer(buffer, count);
	}

	// es un pipe
	return read_pipe(fd, buffer, count);
}


static void sys_time_info(time_info_t *buffer)
{
	get_time(buffer);
}

// limpia la shell y pone el cursor en el principio
static void sys_clear()
{
	vd_clear();
}

static void sys_increase_fontsize()
{
	vd_increase_text_size();
}

static void sys_decrease_fontsize()
{
	vd_decrease_text_size();
}

// Ruido para el juego
static void sys_beep(uint32_t freq_hz, uint64_t duration)
{
	beep(freq_hz, duration);
}

// devuelve la info del tamaño de la pantalla
static void sys_screensize(uint32_t *width, uint32_t *height)
{
	*width  = get_screen_width();
	*height = get_screen_height();
}

// info: [x_center, y_center, radius]
static void sys_circle(uint64_t fill, uint64_t *info, uint32_t color)
{
	if (fill) {
		fill_circle(info[0], info[1], info[2], color);
	} else {
		draw_circle(info[0], info[1], info[2], color);
	}
}

// info: [x0, y0, x1, y1]
static void sys_rectangle(uint64_t fill, uint64_t *info, uint32_t color)
{
	if (fill) {
		fill_rectangle(info[0], info[1], info[2], info[3], color);
	} else {
		draw_rectangle(info[0], info[1], info[2], info[3], color);
	}
}

// info: [x0, y0, x1, y1]
static void sys_draw_line(uint64_t *info, uint32_t color)
{
	vd_draw_line(info[0], info[1], info[2], info[3], color);
}

// info: [x0, y0, size]
static void sys_draw_string(const char *buf, uint64_t *info, uint32_t color)
{
	vd_draw_string(buf, info[0], info[1], color, info[2]);
}

static void sys_speaker_start(uint32_t freq_hz)
{
	speaker_start_tone(freq_hz);
}

static void sys_speaker_stop()
{
	speaker_off();
}

static void sys_textmode()
{
	enable_text_mode();
}

static void sys_videomode()
{
	disable_text_mode();
}

static void sys_put_pixel(uint32_t hex_color, uint64_t x, uint64_t y)
{
	put_pixel(hex_color, x, y);
}

static uint64_t sys_key_status(char c)
{
	return is_pressed_key(c);
}

static void sys_sleep(uint64_t miliseconds)
{
	sleep(miliseconds);
}

static void sys_clear_input_buffer()
{
	clear_buffer();
}

static uint64_t sys_ticks()
{
	return ticks_elapsed();
}

// Memory management syscalls
static void *sys_malloc(size_t size)
{
	return alloc_memory(get_kernel_memory_manager(), size);
}

static void sys_free(void *ptr)
{
	free_memory(get_kernel_memory_manager(), ptr);
}

static mem_info_t sys_mem_info(void)
{
	return get_mem_status(get_kernel_memory_manager());
}

// ===================== Processes syscalls =====================

// Crea un proceso: reserva un PID libre y delega en el scheduler
static int64_t
sys_create_process(void *entry, int argc, const char **argv, const char *name, int fds[2])
{
	if (entry == NULL || name == NULL) {
		return -1;
	}

	int new_pid = scheduler_add_process((process_entry_t)entry, argc, argv, name, fds);
	return new_pid;
}

// Termina el proceso actual con un status, POR AHORA NO USAMOS
static void sys_exit(int status)
{
	scheduler_exit_process(status);
}

// Devuelve el PID del proceso en ejecución
static int64_t sys_getpid(void)
{
	return (int64_t)scheduler_get_current_pid();
}

// Mata un proceso por PID
static int64_t sys_kill(int pid)
{
	return scheduler_kill_process(pid);
}

// Bloquea un proceso por PID
static int64_t sys_block(int pid)
{	
	if (scheduler_block_process(pid) < 0) {
		return -1;
	}
	pcb_t * p = scheduler_get_pcb(pid);
	p->unblockable = 1;
	return 0;
}

// Desbloquea un proceso por PID
static int64_t sys_unblock(int pid)
{
	pcb_t * p = scheduler_get_pcb(pid);
	if (p == NULL || !p->unblockable) {
		return -1;
	}
	return (int64_t)scheduler_unblock_process(pid);
}

// Espera a que el proceso hijo 'pid' termine. Devuelve su código de salida si estaba terminado o 0
// si bloqueó.
static int64_t sys_wait(int pid)
{
	return (int64_t)scheduler_waitpid(pid);
}

static int64_t sys_nice(int pid, int new_prio)
{
	return scheduler_set_priority(pid, new_prio);
}

static void sys_yield()
{
	// _hlt();
	scheduler_force_reschedule();
}

static int sys_processes_info(process_info_t *buf, int max_count)
{
	return scheduler_get_processes(buf, max_count);
}

// SEMÁFOROS (API basada en nombre)
static int64_t sys_sem_open(const char *name, int value)
{
	return (int64_t)sem_open((char *)name, value);
}
static void sys_sem_close(const char *name)
{
	sem_close((char *)name);
}
static void sys_sem_wait(const char *name)
{
	sem_wait((char *)name);
}
static void sys_sem_post(const char *name)
{
	sem_post((char *)name);
}

static int sys_create_pipe(int fds[2])
{ // tengo que agregar
	int pipe_id = create_pipe(fds);
	if (pipe_id < 0) {
		return pipe_id;
	}

	pid_t pid = scheduler_get_current_pid();
	pcb_t *p   = scheduler_get_pcb(pid);

	p->fd_table[fds[0]] = fds[0];
	p->fd_table[fds[1]] = fds[1];

	return pipe_id;
}

static void sys_destroy_pipe(int id)
{
	destroy_pipe(id);
}

static int sys_set_foreground_process(int pid)
{
	// scheduler_set_foreground_process devuelve 0/-1 según éxito
	return scheduler_set_foreground_process((pid_t)pid);
}

static int sys_get_foreground_process(void)
{
	// scheduler_get_foreground_pid devuelve el PID del proceso en foreground o NO_PID
	return scheduler_get_foreground_pid();
}

static int sys_adopt_init_as_parent(int pid)
{
	return adopt_init_as_parent((pid_t)pid);
}

static int sys_open_named_pipe(char *name, int fds[2])
{
	int pipe_id = open_pipe(name, fds);
	if (pipe_id < 0) {
		return pipe_id;
	}

	// Agregar ambos FDs a la lista de open_fds del proceso
	pid_t pid = scheduler_get_current_pid();
	pcb_t  *p   = scheduler_get_pcb(pid);

	p->fd_table[fds[0]] = fds[0];
	p->fd_table[fds[1]] = fds[1];

	return pipe_id;
}

static int sys_close_fd(int fd)
{
	if (fd < 0 || fd >= MAX_FDS) {
		return -1;
	}
	
	pid_t pid = scheduler_get_current_pid();
	pcb_t  *p   = scheduler_get_pcb(pid);
	fd = p->fd_table[fd];
	if (fd < FIRST_FREE_FD) {
		return -1;
	}

	return close_fd(fd);
}

static int sys_pipes_info(pipe_info_t *buf, int max_count)
{
	return pipes_info(buf, max_count);
}
