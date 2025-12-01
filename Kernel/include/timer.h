#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

typedef struct time_info {
    uint8_t hour;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} time_info_t;


void init_timer(void);
uint64_t get_timer_ms(void);
uint64_t get_timer_ticks();
uint64_t timer_handler(uint64_t rsp);
void get_time(time_info_t * buf);

// Import from interrupts.h
extern void _hlt(void);

#endif
