// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "timer.h"
#include "scheduler.h"
#include "video_driver.h"

#define CAL_TICKS 50
#define TIMER_FREQ 100
#define BASE_FREQ 1193182

#define BCD_TO_UINT(x) ((x >> 4) * 10 + (x & 0x0F))
#define DIV_FROM_FREQ(hz) ((uint16_t)((BASE_FREQ + ((hz) / 2)) / (hz)))

extern uint8_t get_hour();
extern uint8_t get_minutes();
extern uint8_t get_seconds();
extern uint8_t get_day();
extern uint8_t get_month();
extern uint8_t get_year();
extern void set_timer_freq(uint64_t divisor);
extern uint64_t tsc_read();

static uint64_t ticks = 0;
static uint64_t tsc_frequency = 0; // TSC ticks por milisegundo

void init_timer(void) {
	
	set_timer_freq(DIV_FROM_FREQ(TIMER_FREQ)); 
	
    // Alinear a un tick: esperamos que cambie ticks
    uint64_t start_ticks = ticks;
    while (ticks == start_ticks) {
        _hlt();
    }

    start_ticks = ticks;
    uint64_t start_tsc = tsc_read();
    uint64_t target_ticks = start_ticks + CAL_TICKS;

    while (ticks < target_ticks) {
        _hlt();
    }

    uint64_t end_tsc = tsc_read();
    uint64_t tsc_delta = end_tsc - start_tsc;

    uint64_t delta_ms = (CAL_TICKS * 1000) / 100; // 100 = TIMER_HZ

    tsc_frequency = tsc_delta / delta_ms; // ciclos por ms
}

uint64_t get_timer_ms(void) {
	// if (tsc_frequency == 0) {
	// 	// Si no se calibró, usar ticks del PIT (menos preciso)
	// 	return ticks * 10; // cada tick = 10ms
	// }
	
	return tsc_read() / tsc_frequency;
}

uint64_t timer_handler(uint64_t rsp) {
	ticks++;
	rsp = (uint64_t)schedule((void *)rsp);
	return rsp;
}

uint64_t ticks_elapsed() { return ticks; }

int seconds_elapsed() { return ticks / 100; }

// refactorizar
void sleep(int miliseconds) { // normaliza a 10 ms
	unsigned long start_ticks = ticks;
	unsigned long target_ticks =
		miliseconds / 10; // convertir ms a ticks (100 ticks/seg)

	while ((ticks - start_ticks) < target_ticks) {
		_hlt();
	}
}

void get_date(uint8_t *buffer) {
  buffer[0] = get_day();
  buffer[1] = get_month();
  buffer[2] = get_year();
}

void get_time(time_info_t * buf) {
	uint8_t hour_bcd = get_hour();
	uint8_t minutes_bcd = get_minutes();
	uint8_t seconds_bcd = get_seconds();
	uint8_t day_bcd = get_day();
	uint8_t month_bcd = get_month();
	uint8_t year_bcd = get_year();
	
	// Convertir de BCD a decimal
	buf->hour = BCD_TO_UINT(hour_bcd);
	buf->minutes = BCD_TO_UINT(minutes_bcd);
	buf->seconds = BCD_TO_UINT(seconds_bcd);
	buf->day = BCD_TO_UINT(day_bcd);
	buf->month = BCD_TO_UINT(month_bcd);
	buf->year = BCD_TO_UINT(year_bcd);
}
