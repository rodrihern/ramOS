// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "timer.h"
#include "scheduler.h"
#include "video_driver.h"

#define CAL_TICKS 5
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

static volatile uint64_t ticks         = 0;
static volatile uint64_t tsc_frequency = 0; // TSC ticks por milisegundo
static volatile uint8_t  timer_ready   = 0; // habilita el scheduler desde el handler
static uint64_t          tsc_start     = 0; // TSC en el momento de la calibracion

void init_timer(void) {
	// Deshabilitamos scheduling mientras calibramos para que no se salte del kernel a init()
	timer_ready = 0;

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

    uint64_t delta_ms = (CAL_TICKS * 1000) / TIMER_FREQ;

	tsc_frequency = tsc_delta / delta_ms; // ciclos por ms
	tsc_start     = start_tsc;            // base para que ms arranque en ~0 al boot
	timer_ready   = 1;
}

uint64_t get_timer_ms(void) {

	if (tsc_frequency == 0) {
		// Si no se calibró, usar ticks del PIT (menos preciso)
		return ticks * (1000 / TIMER_FREQ);
	}

	return (tsc_read() - tsc_start) / tsc_frequency;
}

uint64_t timer_handler(uint64_t rsp) {
	ticks++;

	if (!timer_ready) {
		return rsp;
	}

	rsp = (uint64_t)schedule((void *)rsp);
	return rsp;
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
