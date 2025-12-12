#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

typedef struct framebuffer_cdt * framebuffer_t;

#include <stdint.h>
#include "syscalls.h"

#define FONT_HEIGHT 16
#define FONT_WIDTH 8

framebuffer_t fb_init(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp);

void fb_destroy(framebuffer_t fb);

void fb_present(framebuffer_t fb);
void fb_present_region(framebuffer_t fb, region_t * region);
void fb_present_nregions(framebuffer_t fb, region_t * region[], int n);

void fb_putpixel(framebuffer_t fb, uint32_t color, uint16_t x, uint16_t y);

void fb_fill(framebuffer_t fb, uint32_t color);

void fb_draw_char(framebuffer_t fb, char c, uint8_t font[][FONT_HEIGHT], uint16_t x, uint16_t y, uint32_t color, uint64_t size);

void fb_draw_string(framebuffer_t fb, const char * str, uint8_t font[][FONT_HEIGHT], uint64_t x, uint64_t y, uint32_t color, uint64_t size);

void fb_draw_line(framebuffer_t fb, uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);

void fb_draw_rectangle(framebuffer_t fb,uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);

void fb_fill_rectangle(framebuffer_t fb,uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);

void fb_draw_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color);

void fb_fill_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color);

#endif