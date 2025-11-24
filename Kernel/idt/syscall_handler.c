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
	&sys_video_info,         // 6
	&sys_circle,             // 7
	&sys_rectangle,          // 8
	&sys_draw_line,          // 9
	&sys_draw_string,        // 10
	&sys_clear,              // 11
	&sys_speaker_start,      // 12
	&sys_speaker_stop,       // 13
	&sys_textmode,           // 14
	&sys_videomode,          // 15
	&sys_put_pixel,          // 16
	&sys_is_pressed,         // 17
	&sys_sleep,              // 18
	&sys_clear_input_buffer, // 19
	&sys_ms_elapsed,         // 20

	// syscalls de memoria
	&sys_malloc,   // 21
	&sys_free,     // 22
	&sys_mem_info, // 23

	// syscalls de procesos
	&sys_create_process, // 24
	&sys_exit,           // 25
	&sys_getpid,         // 26
	&sys_kill,           // 27
	&sys_block,          // 28
	&sys_unblock,        // 29
	&sys_wait,           // 30
	&sys_nice,           // 31
	&sys_yield,          // 32
	&sys_processes_info, // 33

	// syscalls de semaforos
	&sys_sem_open,  // 34
	&sys_sem_close, // 35
	&sys_sem_wait,  // 36
	&sys_sem_post,  // 37

	// syscalls de pipes
	&sys_create_pipe,  // 38
	&sys_destroy_pipe, // 39

	&sys_set_foreground_process, // 40
	&sys_adopt_init_as_parent,   // 41
	&sys_get_foreground_process, // 42

	&sys_open_named_pipe, // 43
	&sys_close_fd,        // 44
	&sys_pipes_info,      // 45
};

uint64_t syscall_count = sizeof(syscalls) / sizeof(syscalls[0]);

static int sys_regs(register_info_t * buffer)
{
	if (buffer == NULL) {
		return -1;
	}

	return kb_get_snapshot(buffer);
}

// devuelve cuantos chars escribió
static int sys_write(int fd, const char *buffer, uint64_t count)
{
	if (fd < 0 || fd >= MAX_FDS) {
		return -1;
	}
	int  pid = sch_get_current_pid();
	pcb_t *p   = sch_get_pcb(pid);

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

	int pid = sch_get_current_pid();
	pcb_t *p = sch_get_pcb(pid);

	fd = p->fd_table[fd];

	if (fd < 0 || (STDOUT <= fd && fd < FIRST_FREE_FD)) {
		return -1;
	}

	if (fd == STDIN) { // quiere leer de teclado
		int foreground_pid = sch_get_foreground_pid();
		if (pid != foreground_pid) {
			return -1; // solo el proceso de foreground puede leer del teclado
		}
		return kb_read_buffer(buffer, count);
	}

	// es un pipe
	return read_pipe(fd, buffer, count);
}


static int sys_time_info(time_info_t *buffer) {
	if (buffer == NULL) {
		return -1;
	}
	get_time(buffer);
	return 0;
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

// devuelve la info del tamaño de la pantalla
static int sys_video_info(video_info_t *buffer)
{
	if (buffer == NULL) {
		return -1;
	}
	get_video_info(buffer);
	return 0;
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
	vd_enable_textmode();
}

static void sys_videomode()
{
	vd_disable_text_mode();
}

static void sys_put_pixel(uint32_t hex_color, uint64_t x, uint64_t y)
{
	put_pixel(hex_color, x, y);
}

static uint64_t sys_is_pressed(uint8_t scancode)
{
	return kb_is_pressed(scancode);
}

static void sys_sleep(uint64_t miliseconds) {
	uint64_t start_ms = get_timer_ms();
	while (get_timer_ms() - start_ms < miliseconds) {
		sch_force_reschedule();
	}
}

static void sys_clear_input_buffer()
{
	kb_flush_buffer();
}

static uint64_t sys_ms_elapsed()
{
	return get_timer_ms();
}

// Memory management syscalls
static void *sys_malloc(uint64_t size)
{
	return mm_alloc(get_kernel_memory_manager(), size);
}

static void sys_free(void *ptr)
{
	mm_free(get_kernel_memory_manager(), ptr);
}

static int sys_mem_info(mem_info_t *buffer)
{
	if (buffer == NULL) {
		return -1;
	}
	get_memory_info(get_kernel_memory_manager(), buffer);
	return 0;
}


// ===================== Processes syscalls =====================

// Crea un proceso: reserva un PID libre y delega en el scheduler
static int64_t
sys_create_process(void *entry, int argc, const char **argv, const char *name, process_attrs_t * attrs)
{
	if (entry == NULL || name == NULL) {
		return -1;
	}

	int current_pid = sch_get_current_pid();
	if (attrs != NULL && current_pid != sch_get_foreground_pid()) {
		attrs->foreground = 0;
	}

	return create_process(entry, argc, argv, name, attrs);
}

// Termina el proceso actual con un status, POR AHORA NO USAMOS
static void sys_exit(int status)
{
	sch_exit_process(status);
}

// Devuelve el PID del proceso en ejecución
static int64_t sys_getpid(void)
{
	return (int64_t)sch_get_current_pid();
}

// Mata un proceso por PID
static int64_t sys_kill(int pid)
{
	return sch_kill_process(pid);
}

// Bloquea un proceso por PID
static int64_t sys_block(int pid)
{	
	if (sch_block_process(pid) < 0) {
		return -1;
	}
	pcb_t * p = sch_get_pcb(pid);
	p->unblockable = 1;
	return 0;
}

// Desbloquea un proceso por PID
static int64_t sys_unblock(int pid)
{
	pcb_t * p = sch_get_pcb(pid);
	if (p == NULL || !p->unblockable) {
		return -1;
	}
	return (int64_t)sch_ublock_process(pid);
}

// Espera a que el proceso hijo 'pid' termine. Devuelve su código de salida si estaba terminado o 0
// si bloqueó.
static int64_t sys_wait(int pid)
{
	return (int64_t)sch_waitpid(pid);
}

static int64_t sys_nice(int pid, int new_prio)
{
	return sch_set_priority(pid, new_prio);
}

static void sys_yield()
{
	// _hlt();
	sch_force_reschedule();
}

static int sys_processes_info(process_info_t *buffer, int max_count)
{
	if (buffer == NULL) {
		return -1;
	}
	return get_processes_info(buffer, max_count);
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

	pid_t pid = sch_get_current_pid();
	pcb_t *p   = sch_get_pcb(pid);

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
	// sch_set_foreground_process devuelve 0/-1 según éxito
	return sch_set_foreground_process((pid_t)pid);
}

static int sys_get_foreground_process(void)
{
	// sch_get_foreground_pid devuelve el PID del proceso en foreground o NO_PID
	return sch_get_foreground_pid();
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
	pid_t pid = sch_get_current_pid();
	pcb_t  *p   = sch_get_pcb(pid);

	p->fd_table[fds[0]] = fds[0];
	p->fd_table[fds[1]] = fds[1];

	return pipe_id;
}

static int sys_close_fd(int fd)
{
	if (fd < 0 || fd >= MAX_FDS) {
		return -1;
	}
	
	pid_t pid = sch_get_current_pid();
	pcb_t  *p   = sch_get_pcb(pid);
	fd = p->fd_table[fd];
	if (fd < FIRST_FREE_FD) {
		return -1;
	}

	return close_fd(fd);
}

static int sys_pipes_info(pipe_info_t *buffer, int max_count)
{
	if (buffer == NULL) {
		return -1;
	}
	return get_pipes_info(buffer, max_count);
}
