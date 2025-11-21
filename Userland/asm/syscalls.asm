global  sys_write, sys_read, sys_time, sys_regs, sys_clear
global  sys_increase_fontsize, sys_decrease_fontsize, sys_beep
global  sys_screensize, sys_circle, sys_rectangle, sys_draw_line, sys_draw_string
global  sys_textmode, sys_videomode, sys_put_pixel, sys_key_status
global  sys_sleep, sys_clear_input_buffer, sys_ticks
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

sys_time:
	SYSCALL 3

sys_increase_fontsize:
	SYSCALL 4

sys_decrease_fontsize:
	SYSCALL 5

sys_beep:
	SYSCALL 6

sys_screensize:
	SYSCALL 7

sys_circle:
	SYSCALL 8

sys_rectangle:
	SYSCALL 9

sys_draw_line:
	SYSCALL 10

sys_draw_string:
	SYSCALL 11

sys_clear:
	SYSCALL 12

sys_speaker_start:
	SYSCALL 13

sys_speaker_stop:
	SYSCALL 14

sys_textmode:
	SYSCALL 15

sys_videomode:
	SYSCALL 16

sys_put_pixel:
	SYSCALL 17

sys_key_status:
	SYSCALL 18

sys_sleep:
	SYSCALL 19

sys_clear_input_buffer:
	SYSCALL 20

sys_ticks:
	SYSCALL 21

sys_malloc:
    SYSCALL 22

sys_free:
    SYSCALL 23

sys_mem_info:
    SYSCALL 24

sys_create_process:
    SYSCALL 25

sys_exit:
    SYSCALL 26

sys_getpid:
    SYSCALL 27

sys_kill:
    SYSCALL 28

sys_block:
    SYSCALL 29

sys_unblock:
    SYSCALL 30

sys_wait:
    SYSCALL 31

sys_nice:
    SYSCALL 32

sys_yield:
    SYSCALL 33

sys_processes_info:
    SYSCALL 34

sys_sem_open:
    SYSCALL 35

sys_sem_close:
    SYSCALL 36

sys_sem_wait:
    SYSCALL 37

sys_sem_post:
    SYSCALL 38

sys_create_pipe:
    SYSCALL 39

sys_destroy_pipe:
    SYSCALL 40

sys_set_foreground_process:
    SYSCALL 41

sys_adopt_init_as_parent:
    SYSCALL 42

sys_get_foreground_process:
    SYSCALL 43

sys_open_named_pipe: 
    SYSCALL 44

sys_close_fd:
    SYSCALL 45

sys_pipes_info:
    SYSCALL 46

