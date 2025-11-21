global  sys_write, sys_read, sys_date, sys_time, sys_regs, sys_clear
global  sys_increase_fontsize, sys_decrease_fontsize, sys_beep
global  sys_screen_size, sys_circle, sys_rectangle, sys_line, sys_draw_string
global  sys_enable_textmode, sys_disable_textmode, sys_put_pixel, sys_key_status
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

sys_regs:
	SYSCALL 0

sys_time:
	SYSCALL 1

sys_date:
	SYSCALL 2

sys_read:
	SYSCALL 3
		
sys_write:
	SYSCALL 4

sys_increase_fontsize:
	SYSCALL 5

sys_decrease_fontsize:
	SYSCALL 6

sys_beep:
	SYSCALL 7

sys_screen_size:
	SYSCALL 8

sys_circle:
	SYSCALL 9

sys_rectangle:
	SYSCALL 10

sys_line:
	SYSCALL 11

sys_draw_string:
	SYSCALL 12

sys_clear:
	SYSCALL 13

sys_speaker_start:
	SYSCALL 14

sys_speaker_stop:
	SYSCALL 15

sys_enable_textmode:
	SYSCALL 16

sys_disable_textmode:
	SYSCALL 17

sys_put_pixel:
	SYSCALL 18

sys_key_status:
	SYSCALL 19

sys_sleep:
	SYSCALL 20

sys_clear_input_buffer:
	SYSCALL 21

sys_ticks:
	SYSCALL 22

sys_malloc:
    SYSCALL 23

sys_free:
    SYSCALL 24

sys_mem_info:
    SYSCALL 25

sys_create_process:
    SYSCALL 26

sys_exit:
    SYSCALL 27

sys_getpid:
    SYSCALL 28

sys_kill:
    SYSCALL 29

sys_block:
    SYSCALL 30

sys_unblock:
    SYSCALL 31

sys_wait:
    SYSCALL 32

sys_nice:
    SYSCALL 33

sys_yield:
    SYSCALL 34

sys_processes_info:
    SYSCALL 35

sys_sem_open:
    SYSCALL 36

sys_sem_close:
    SYSCALL 37

sys_sem_wait:
    SYSCALL 38

sys_sem_post:
    SYSCALL 39

sys_create_pipe:
    SYSCALL 40

sys_destroy_pipe:
    SYSCALL 41

sys_set_foreground_process:
    SYSCALL 42

sys_adopt_init_as_parent:
    SYSCALL 43

sys_get_foreground_process:
    SYSCALL 44

sys_open_named_pipe: 
    SYSCALL 45

sys_close_fd:
    SYSCALL 46

sys_pipes_info:
    SYSCALL 47

