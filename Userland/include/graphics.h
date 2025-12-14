#ifndef GOLF_GRAPHICS_H
#define GOLF_GRAPHICS_H

#include "golf.h"


#define MENU_BKG    0x041803  
#define MENU_CONTENT 0xf4d35e 
#define SCOREBOARD_HEIGHT 32

void init_graphics(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp);

uint16_t get_width();

uint16_t get_height();





void draw_background(uint32_t color);

void draw_player(circle_t *p);

void draw_circle(circle_t *c);

void draw_scoreboard(uint8_t two_players, uint16_t score1, uint16_t score2, uint64_t touches);


int draw_menu_screen(void); // TODO: cambiar por un menu con flechitas

void draw_level_end_screen(uint8_t winner);

void draw_countdown_screen(uint64_t size);

void draw_final_score_screen(uint8_t two_players, uint16_t score_p1, uint16_t score_p2, uint64_t touches);


void show_frame();

#define WAIT_SPACE()   while(getchar()!=' ') 

#endif