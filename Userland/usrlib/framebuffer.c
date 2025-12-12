#include "framebuffer.h"
#include "usrlib.h"

#define abs(x) ((x) < 0 ? -(x) : (x))

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

static void draw_bitmap(framebuffer_t fb, uint8_t bitmap[FONT_HEIGHT], uint16_t x, uint16_t y, uint32_t color, uint64_t size) {
	for (int i = 0; i < FONT_HEIGHT; i++) {
		char line = bitmap[i];
		for (int j = 0; j < FONT_WIDTH; j++) {
		
			// dibuja un cuadrado de size × size pixels por cada bit
			for (uint64_t dy = 0; dy < size; dy++) {
				for (uint64_t dx = 0; dx < size; dx++) {
                    if ((line << j) & 0x80)
					    fb_putpixel(fb, color, x + j * size + dx, y + i * size + dy);
				}
			}
				
			
		}
	}
}

void fb_draw_char(framebuffer_t fb, char c, uint8_t font[][FONT_HEIGHT], uint16_t x, uint16_t y, uint32_t color, uint64_t size) {
    draw_bitmap(fb, font[(uint8_t)c], x, y, color, size);
}

void fb_draw_string(framebuffer_t fb, const char * str, uint8_t font[][FONT_HEIGHT], uint64_t x, uint64_t y, uint32_t color, uint64_t size) {
    for (int i = 0; str[i] != 0; i++) {
        fb_draw_char(fb, str[i], font, x + (FONT_WIDTH*size)*i, y, color, size);
    }
}       

void fb_draw_line(framebuffer_t fb, uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // algoritmo de Bresenham
    int64_t dx = abs((int64_t)x1 - (int64_t)x0);
    int64_t dy = abs((int64_t)y1 - (int64_t)y0);

    int64_t step_x = (x0 < x1) ? 1 : -1;
    int64_t step_y = (y0 < y1) ? 1 : -1;

    int64_t error = dx - dy;
	int done = 0;

    while (!done) {
        fb_putpixel(fb, color, x0, y0);  

		if ((x0 == x1 && y0 == y1) || !is_in_bounds(fb, x0, y0)) { 
			done = 1;
		} else {
			int64_t error2 = 2 * error;
	
			if (error2 > -dy) {
				error -= dy;
				x0 += step_x;
			}
	
			if (error2 < dx) {
				error += dx;
				y0 += step_y;
			}
		}
    }

}

void fb_draw_rectangle(framebuffer_t fb,uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // checkeo que (x0, y0) sean esquina superior izquierda
	int dx = x1-x0;
	int dy = y1-y0;
	if (dx < 0 || dy < 0) {
		return;
	}
	
	fb_draw_line(fb, x0, y0, x1, y0, color);
	fb_draw_line(fb, x0, y0, x0, y1, color);
	fb_draw_line(fb, x1 ,y1, x0, y1, color);
	fb_draw_line(fb, x1, y1, x1, y0, color);
}

void fb_fill_rectangle(framebuffer_t fb, uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color) {
    // checkeo que (x0, y0) sean esquina superior izquierda
	int dx = x1-x0;
	int dy = y1-y0;
	if (dx < 0 || dy < 0) {
		return;
	}
	
	for (int i = 0; i < dx; i++) {
		for (int j = 0; j < dy; j++) {
			fb_putpixel(fb, color, x0+i, y0+j);
		}
	}
}

void fb_draw_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color) {
    int64_t x = radius;
    int64_t y = 0;
    int64_t err = 0;

    while (x >= y) {
        fb_putpixel(fb, color, x_center + x, y_center + y);
        fb_putpixel(fb, color, x_center + y, y_center + x);
        fb_putpixel(fb, color, x_center - y, y_center + x);
        fb_putpixel(fb, color, x_center - x, y_center + y);
        fb_putpixel(fb, color, x_center - x, y_center - y);
        fb_putpixel(fb, color, x_center - y, y_center - x);
        fb_putpixel(fb, color, x_center + y, y_center - x);
        fb_putpixel(fb, color, x_center + x, y_center - y);

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void fb_fill_circle(framebuffer_t fb,uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color) {
    uint64_t x0 = (x_center >= radius) ? x_center - radius : 0;
    uint64_t y0 = (y_center >= radius) ? y_center - radius : 0;
    uint64_t x1 = x_center + radius;
    uint64_t y1 = y_center + radius;

    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            int dx = x - x_center;
            int dy = y - y_center;
            if (dx*dx + dy*dy <= radius*radius) { // si esta adentro del circulo
                fb_putpixel(fb, color, x, y);
            }
        }
    }
}