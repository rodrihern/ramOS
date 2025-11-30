; registers
global sys_register_snapshot

; io
global sys_read, sys_write, sys_flush

; tty
global sys_tty_clear, sys_tty_show, sys_tty_set_bgcolor, sys_tty_set_textcolor
global sys_tty_get_fontsize, sys_tty_set_fontsize

; video
global sys_video_info, sys_present, sys_present_region, sys_present_nregions

; keyboard
global sys_is_pressed

; time
global sys_sleep, sys_ms_elapsed, sys_ticks_elapsed, sys_time_info

; sound
global sys_speaker_start, sys_speaker_stop

; memory
global sys_malloc, sys_free, sys_mem_info

; processes
global sys_create_process, sys_exit, sys_getpid, sys_kill, sys_block, sys_unblock
global sys_wait, sys_nice, sys_yield, sys_set_foreground_process
global sys_adopt_init_as_parent, sys_get_foreground_process, sys_processes_info

; semaphores
global sys_sem_open, sys_sem_close, sys_sem_wait, sys_sem_post

; pipes
global sys_create_pipe, sys_destroy_pipe, sys_open_named_pipe, sys_close_fd, sys_pipes_info

; macro para syscalls
%macro SYSCALL 1
	mov     rax, %1
	int     0x80
	ret
%endmacro

section .text

; registers
sys_register_snapshot:
	SYSCALL 0

; io
sys_read:
	SYSCALL 1

sys_write:
	SYSCALL 2

sys_flush:
	SYSCALL 3

; tty
sys_tty_show:
	SYSCALL 4

sys_tty_clear:
	SYSCALL 5

sys_tty_set_bgcolor:
	SYSCALL 6

sys_tty_set_textcolor:
	SYSCALL 7

sys_tty_get_fontsize:
	SYSCALL 8

sys_tty_set_fontsize:
	SYSCALL 9

; video
sys_video_info:
	SYSCALL 10

sys_present:
	SYSCALL 11

sys_present_region:
	SYSCALL 12

sys_present_nregions:
	SYSCALL 13

; keyboard
sys_is_pressed:
	SYSCALL 14

; time
sys_sleep:
	SYSCALL 15

sys_ms_elapsed:
	SYSCALL 16

sys_ticks_elapsed:
	SYSCALL 17

sys_time_info:
	SYSCALL 18

; sound
sys_speaker_start:
	SYSCALL 19

sys_speaker_stop:
	SYSCALL 20

; memory
sys_malloc:
	SYSCALL 21

sys_free:
	SYSCALL 22

sys_mem_info:
	SYSCALL 23

; processes
sys_create_process:
	SYSCALL 24

sys_exit:
	SYSCALL 25

sys_getpid:
	SYSCALL 26

sys_kill:
	SYSCALL 27

sys_block:
	SYSCALL 28

sys_unblock:
	SYSCALL 29

sys_wait:
	SYSCALL 30

sys_nice:
	SYSCALL 31

sys_yield:
	SYSCALL 32

sys_set_foreground_process:
	SYSCALL 33

sys_adopt_init_as_parent:
	SYSCALL 34

sys_get_foreground_process:
	SYSCALL 35

sys_processes_info:
	SYSCALL 36

; semaphores
sys_sem_open:
	SYSCALL 37

sys_sem_close:
	SYSCALL 38

sys_sem_wait:
	SYSCALL 39

sys_sem_post:
	SYSCALL 40

; pipes
sys_create_pipe:
	SYSCALL 41

sys_destroy_pipe:
	SYSCALL 42

sys_open_named_pipe:
	SYSCALL 43

sys_close_fd:
	SYSCALL 44

sys_pipes_info:
	SYSCALL 45
