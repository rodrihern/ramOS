#include "framebuffer.h"
#include "usrlib.h"

typedef struct framebuffer_cdt {
    uint16_t width;
    uint16_t height;
    uint16_t pitch;
    uint8_t bpp;
    void * framebuffer;
} framebuffer_cdt;

framebuffer_t fb_init(uint16_t width, uint16_t height, uint16_t pitch, uint8_t bpp) {
    framebuffer_t fb = sys_malloc(sizeof(framebuffer_cdt));
    fb->width = width;
    fb->height = height;
    fb->pitch = pitch;
    fb->bpp = bpp;
    fb->framebuffer = sys_malloc(height * pitch);

    return fb;
}

void fb_destroy(framebuffer_t fb) {
    sys_free(fb->framebuffer);
    sys_free(fb);
}

void fb_present(framebuffer_t fb) {
    sys_present(fb->framebuffer);
}
void fb_present_region(framebuffer_t fb, region_t * region) {
    sys_present_region(fb->framebuffer, region);
}

void fb_present_nregions(framebuffer_t fb, region_t * regions[], int n) {
    sys_present_nregions(fb->framebuffer, regions, n);
}

static uint8_t is_in_bounds(framebuffer_t fb, uint16_t x, uint16_t y) {
    return x < fb->width && y < fb->height;
}

void fb_putpixel(framebuffer_t fb, uint32_t color, uint16_t x, uint16_t y) {
    if (!is_in_bounds(fb, x, y)) {
		return;
	}
	uint8_t *framebuffer = (uint8_t *)fb->framebuffer;
	uint64_t offset = (x * (fb->bpp / 8)) + (y * fb->pitch);
	
	// if (BPP == 32) {
	// 	uint32_t *pixel = (uint32_t *)(framebuffer + offset);
	// 	*pixel = hex_color;
	// } else {
	// 	framebuffer[offset] = (hex_color) & 0xFF;
	// 	framebuffer[offset + 1] = (hex_color >> 8) & 0xFF;
	// 	framebuffer[offset + 2] = (hex_color >> 16) & 0xFF;
	// }

	uint32_t *pixel = (uint32_t *)(framebuffer + offset);
	*pixel = color;
}

void fb_fill(framebuffer_t fb, uint32_t color) {
    uint64_t to_fill = color | ((uint64_t) color << 32);
    int fb_size = fb->height * fb->pitch;
    memset64(fb->framebuffer, to_fill, fb_size);
}

void fb_draw_char(framebuffer_t fb, uint8_t ch, uint8_t font[][FONT_HEIGHT], uint16_t x, uint16_t y, uint32_t color, uint64_t size) {
    // TODO: implement
}

void fb_draw_string(framebuffer_t fb, const char * str, uint8_t font[][FONT_HEIGHT], uint64_t x, uint64_t y, uint32_t color, uint64_t size) {
    // TODO: implement
}

void fb_drawLine(framebuffer_t fb, uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // TODO: implement
}

void fb_draw_rectangle(framebuffer_t fb,uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // TODO: implement
}

void fb_fill_rectangle(framebuffer_t fb,uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // TODO: implement
}

void fb_draw_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color) {
    // TODO: implement
}

void fb_fill_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color) {
    // TODO: implement
}