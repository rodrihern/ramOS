#ifndef FONT_H
#define FONT_H

#include <stdint.h>

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
/*
    fuente: https://github.com/hubenchang0515/font8x16/blob/master/font8x16.h
*/



extern uint8_t cursor[];
extern uint8_t font[][FONT_HEIGHT];

#endif