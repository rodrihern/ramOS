// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "timer.h"
#include "scheduler.h"
#include "video_driver.h"

#define BCD_TO_UINT(x) ((x >> 4) * 10 + (x & 0x0F))

extern uint8_t get_hour();
extern uint8_t get_minutes();
extern uint8_t get_seconds();
extern uint8_t get_day();
extern uint8_t get_month();
extern uint8_t get_year();



static uint64_t ticks = 0;

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
