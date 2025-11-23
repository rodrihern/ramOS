global  sys_write, sys_read, sys_time_info, sys_regs, sys_clear
global  sys_increase_fontsize, sys_decrease_fontsize
global  sys_screensize, sys_circle, sys_rectangle, sys_draw_line, sys_draw_string
global  sys_textmode, sys_videomode, sys_put_pixel, sys_is_pressed
global  sys_sleep, sys_clear_input_buffer, sys_ms_elapsed
global  sys_speaker_start, sys_speaker_stop
global  sys_malloc, sys_free, sys_mem_info
global  sys_create_process, sys_exit_current, sys_getpid, sys_kill, sys_block, sys_unblock, sys_wait, sys_nice, sys_processes_info, sys_yield
global sys_sem_open,sys_sem_close,sys_sem_wait,sys_sem_post
global sys_create_pipe, sys_destroy_pipe, sys_open_named_pipe, sys_close_fd, sys_pipes_info
global sys_set_foreground_process, sys_adopt_init_as_parent, sys_get_foreground_process

; macro para syscalls
%macro SYSCALL 1
	mov     rax, %1
	int     0x80
	ret
%endmacro

section .text 

sys_read:
	SYSCALL 0
		
sys_write:
	SYSCALL 1

sys_regs:
	SYSCALL 2

sys_time_info:
	SYSCALL 3

sys_increase_fontsize:
	SYSCALL 4

sys_decrease_fontsize:
	SYSCALL 5

sys_screensize:
	SYSCALL 6

sys_circle:
	SYSCALL 7

sys_rectangle:
	SYSCALL 8

sys_draw_line:
	SYSCALL 9

sys_draw_string:
	SYSCALL 10

sys_clear:
	SYSCALL 11

sys_speaker_start:
	SYSCALL 12

sys_speaker_stop:
	SYSCALL 13

sys_textmode:
	SYSCALL 14

sys_videomode:
	SYSCALL 15

sys_put_pixel:
	SYSCALL 16

sys_is_pressed:
	SYSCALL 17

sys_sleep:
	SYSCALL 18

sys_clear_input_buffer:
	SYSCALL 19

sys_ms_elapsed:
	SYSCALL 20

sys_malloc:
    SYSCALL 21

sys_free:
    SYSCALL 22

sys_mem_info:
    SYSCALL 23

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

sys_processes_info:
    SYSCALL 33

sys_sem_open:
    SYSCALL 34

sys_sem_close:
    SYSCALL 35

sys_sem_wait:
    SYSCALL 36

sys_sem_post:
    SYSCALL 37

sys_create_pipe:
    SYSCALL 38

sys_destroy_pipe:
    SYSCALL 39

sys_set_foreground_process:
    SYSCALL 40

sys_adopt_init_as_parent:
    SYSCALL 41

sys_get_foreground_process:
    SYSCALL 42

sys_open_named_pipe: 
    SYSCALL 43

sys_close_fd:
    SYSCALL 44

sys_pipes_info:
    SYSCALL 45
