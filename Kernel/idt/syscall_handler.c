// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


#include "syscall_handler.h"


#define MIN_CHAR 0
#define MAX_CHAR 256

void *syscalls[] = {
	// registers
	&sys_register_snapshot,  // 0

	// io
	&sys_read,               // 1
	&sys_write,              // 2
	&sys_flush,              // 3

	// tty
	&sys_tty_show,           // 4
	&sys_tty_clear,          // 5
	&sys_tty_set_bgcolor,    // 6
	&sys_tty_set_textcolor,  // 7
	&sys_tty_get_fontsize,   // 8
	&sys_tty_set_fontsize,   // 9

	// video
	&sys_video_info,         // 10
	&sys_present,            // 11
	&sys_present_region,     // 12
	&sys_present_nregions,   // 13

	// keyboard
	&sys_is_pressed,         // 14

	// time
	&sys_sleep,              // 15
	&sys_ms_elapsed,         // 16
	&sys_ticks_elapsed,      // 17
	&sys_time_info,          // 18

	// sound
	&sys_speaker_start,      // 19
	&sys_speaker_stop,       // 20

	// memory
	&sys_malloc,             // 21
	&sys_free,               // 22
	&sys_mem_info,           // 23

	// processes
	&sys_create_process,     // 24
	&sys_exit,               // 25
	&sys_getpid,             // 26
	&sys_kill,               // 27
	&sys_block,              // 28
	&sys_unblock,            // 29
	&sys_wait,               // 30
	&sys_nice,               // 31
	&sys_yield,              // 32
	&sys_set_foreground_process, // 33
	&sys_adopt_init_as_parent,   // 34
	&sys_get_foreground_process, // 35
	&sys_processes_info,     // 36

	// semaphores
	&sys_sem_open,           // 37
	&sys_sem_close,          // 38
	&sys_sem_wait,           // 39
	&sys_sem_post,           // 40

	// pipes
	&sys_create_pipe,        // 41
	&sys_destroy_pipe,       // 42
	&sys_open_named_pipe,    // 43
	&sys_close_fd,           // 44
	&sys_pipes_info,         // 45
};

uint64_t syscall_count = sizeof(syscalls) / sizeof(syscalls[0]);

static uint8_t is_foreground() {
	return sch_get_current_pid() == sch_get_foreground_pid();
}

static int sys_register_snapshot(register_info_t * buffer)
{
	if (buffer == NULL) {
		return -1;
	}

	return kb_get_snapshot(buffer);
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
		if (!is_foreground()) {
			return -1; // only foreground process can read form keyboard
		}
		return kb_read_buffer(buffer, count);
	}

	// es un pipe
	return read_pipe(fd, buffer, count);
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
			vd_putchar(buffer[i], color);
		}

		return count;
	}

	// es un pipe
	return write_pipe(fd, buffer, count);
}

int sys_flush(int fd)
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
		kb_flush_buffer();
		return 0;
	}

	// es un pipe
	return flush_pipe(fd);
}

// limpia la shell y pone el cursor en el principio
static void sys_tty_show() {
	vd_show_tty();
}

static void sys_tty_clear()
{
	vd_clear_tty();
}

static void sys_tty_set_bgcolor(uint32_t color) {
	// TODO: implement
}

static void sys_tty_set_textcolor(uint32_t color) {
	// TODO: implement
}

static int sys_tty_get_fontsize() {
	return vd_get_text_size();
}

static void sys_tty_set_fontsize(int size) {
	vd_set_text_size(size);
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

static void sys_present(void * framebuffer) {
	if (!is_foreground()) {
		return;
	}

	vd_present(framebuffer);
}

static void sys_present_region(void * framebuffer, region_t * region) {
	if (!is_foreground()) {
		return;
	}

	vd_present_region(framebuffer, region);
}

static void sys_present_nregions(void * framebuffer, region_t * regions[], uint64_t n)  {
	if (!is_foreground()) {
		return;
	}

	for (uint64_t i = 0; i < n; i++) {
		vd_present_region(framebuffer, regions[i]);
	}
}

static uint8_t sys_is_pressed(uint8_t scancode)
{
	return kb_is_pressed(scancode);
}

static void sys_sleep(uint64_t miliseconds) {
	uint64_t start_ms = get_timer_ms();
	while (get_timer_ms() - start_ms < miliseconds) {
		sch_force_reschedule();
	}
}

static uint64_t sys_ms_elapsed()
{
	return get_timer_ms();
}

static uint64_t sys_ticks_elapsed() {
	// TODO: implement
	return 0;
}

static int sys_time_info(time_info_t *buffer) {
	if (buffer == NULL) {
		return -1;
	}
	get_time(buffer);
	return 0;
}



static void sys_speaker_start(uint32_t freq_hz)
{
	speaker_start_tone(freq_hz);
}

static void sys_speaker_stop()
{
	speaker_off();
}


// Memory management syscalls
static void *sys_malloc(uint64_t size)
{
	return mm_alloc(size);
}

static void sys_free(void *ptr)
{
	mm_free(ptr);
}

static int sys_mem_info(mem_info_t *buffer)
{
	if (buffer == NULL) {
		return -1;
	}
	get_memory_info(buffer);
	return 0;
}


// Crea un proceso: reserva un PID libre y delega en el scheduler
static int sys_create_process(void *entry, int argc, const char **argv, const char *name, process_attrs_t * attrs)
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
static int sys_getpid(void)
{
	return sch_get_current_pid();
}

// Mata un proceso por PID
static int sys_kill(int pid)
{
	return sch_kill_process(pid);
}

// Bloquea un proceso por PID
static int sys_block(int pid)
{	
	if (sch_block_process(pid) < 0) {
		return -1;
	}
	pcb_t * p = sch_get_pcb(pid);
	p->unblockable = 1;
	return 0;
}

// Desbloquea un proceso por PID
static int sys_unblock(int pid)
{
	pcb_t * p = sch_get_pcb(pid);
	if (p == NULL || !p->unblockable) {
		return -1;
	}
	return sch_ublock_process(pid);
}

// Espera a que el proceso hijo 'pid' termine. Devuelve su código de salida si estaba terminado o 0
// si bloqueó.
static int sys_wait(int pid)
{
	return sch_waitpid(pid);
}

static int sys_nice(int pid, int new_prio)
{
	return sch_set_priority(pid, new_prio);
}

static void sys_yield()
{
	sch_force_reschedule();
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

static int sys_processes_info(process_info_t *buffer, int max_count)
{
	if (buffer == NULL) {
		return -1;
	}
	return get_processes_info(buffer, max_count);
}


// SEMÁFOROS (API basada en nombre)
static int sys_sem_open(const char *name, int value)
{
	return sem_open((char *)name, value);
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
