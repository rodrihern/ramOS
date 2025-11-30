// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "lib.h"
#include <font.h>
#include <video.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define ENABLED 1
#define DISABLED 0
#define BKG_COLOR 0x000000
#define MAX_SIZE 4
#define WIDTH (VBE_mode_info->width)
#define HEIGHT (VBE_mode_info->height)
#define BPP (VBE_mode_info->bpp)
#define PITCH (VBE_mode_info->pitch)


static VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;

static int tty_show = DISABLED;

static uint64_t cursor_x;
static uint64_t cursor_y;
static uint8_t text_size = 1;

static int is_within_screen_bounds(uint64_t x, uint64_t y) {
	return x < VBE_mode_info->width && y < VBE_mode_info->height;
}



static void put_pixel(uint32_t hex_color, uint64_t x, uint64_t y) {
	if (!is_within_screen_bounds(x, y)) {
		return;
	}
	uint8_t *framebuffer = (uint8_t *)(uintptr_t)VBE_mode_info->framebuffer;
	uint64_t offset = (x * (BPP / 8)) + (y * PITCH);
	
	// if (BPP == 32) {
	// 	uint32_t *pixel = (uint32_t *)(framebuffer + offset);
	// 	*pixel = hex_color;
	// } else {
	// 	framebuffer[offset] = (hex_color) & 0xFF;
	// 	framebuffer[offset + 1] = (hex_color >> 8) & 0xFF;
	// 	framebuffer[offset + 2] = (hex_color >> 16) & 0xFF;
	// }

	uint32_t *pixel = (uint32_t *)(framebuffer + offset);
	*pixel = hex_color;
}

void get_video_info(video_info_t * buffer) {
	buffer->width = WIDTH;
	buffer->heght = HEIGHT;
	buffer->pitch = PITCH;
	buffer->bpp = BPP;
	
}

void vd_show_tty() {
	if (tty_show) {
		return;
	}
	tty_show = ENABLED;
	vd_clear_tty();
}


void vd_set_text_size(int size) {
	if (size < MIN_TEXT_SIZE) {
		text_size = MIN_TEXT_SIZE;
	} else if (size > MAX_TEXT_SIZE) {
		text_size = MAX_TEXT_SIZE;
	} else {
		text_size = size; 
	}
}

int vd_get_text_size() { return text_size; }

static void scroll_up() {
	uint64_t line_height = text_size * FONT_HEIGHT;
	uint8_t *framebuffer = (uint8_t *)(uintptr_t)VBE_mode_info->framebuffer;

	// Usar memcpy para copiar cada línea completa
	// desde la segunda línea de texto hasta el final hacia arriba
	for (uint64_t src_y = line_height; src_y < HEIGHT; src_y++) {
		uint64_t dst_y = src_y - line_height;

		// Calcular offset de la línea fuente y destino
		uint64_t src_offset = src_y * PITCH;
		uint64_t dst_offset = dst_y * PITCH;

		memcpy(framebuffer + dst_offset, framebuffer + src_offset,
			PITCH);
	}

	// Limpiar la última línea usando memset
	uint64_t last_line_start = HEIGHT - line_height;
	uint64_t last_line_offset = last_line_start * PITCH;
	uint64_t bytes_to_clear = line_height * PITCH;
	uint64_t color = BKG_COLOR | ((uint64_t)BKG_COLOR << 32);
	memset64(framebuffer + last_line_offset, color, bytes_to_clear);
}


static void update_cursor() {
	if (!is_within_screen_bounds(cursor_x + FONT_WIDTH * text_size,
								cursor_y + FONT_HEIGHT * text_size)) {
		vd_new_line();
	}
}

static void move_cursor_right() {
	uint64_t step_x = FONT_WIDTH * text_size;
	if (cursor_x + 2 * step_x <= WIDTH) {
		cursor_x += step_x;
	} else {
		vd_new_line();
	}
}

static void move_cursor_left() {
	uint64_t step_x = FONT_WIDTH * text_size;
	if (cursor_x >= step_x) {
		cursor_x -= step_x;
	} else {
		uint64_t step_y = FONT_HEIGHT * text_size;
		if (cursor_y >= step_y) {
		cursor_y -= step_y;
		cursor_x = (WIDTH / step_x - 1) * step_x;
		}
	}
}

static void draw_bitmap(uint8_t bitmap[FONT_HEIGHT], uint64_t x, uint64_t y, uint32_t color, uint64_t size) {
	for (int i = 0; i < FONT_HEIGHT; i++) {
		char line = bitmap[i];
		for (int j = 0; j < FONT_WIDTH; j++) {
			uint32_t pixel_color = (line << j) & 0x80 ? color : BKG_COLOR;
		
			// dibuja un cuadrado de size × size pixels por cada bit
			for (uint64_t dy = 0; dy < size; dy++) {
				for (uint64_t dx = 0; dx < size; dx++) {
					put_pixel(pixel_color, x + j * size + dx, y + i * size + dy);
				}
			}
				
			
		}
	}
}

static void draw_cursor() {
	draw_bitmap(cursor, cursor_x, cursor_y, 0xFFFFFF, text_size);
}

static void delete_cursor() {
	draw_bitmap(font[0], cursor_x, cursor_y, 0x000000, text_size);
}

void vd_new_line() {
	cursor_x = 0;
	uint64_t step_y = text_size * FONT_HEIGHT;

	// Verificar si hay espacio para una línea más sin hacer scroll
	if (cursor_y + step_y < HEIGHT) {
		cursor_y += step_y;
		// fill_rectangle(cursor_x, cursor_y, WIDTH,
		// 			cursor_y + FONT_HEIGHT * text_size, BKG_COLOR);
	} else {
		scroll_up();
		cursor_y = HEIGHT - step_y; // Posicionar cursor en la última línea
	}
	draw_cursor();
}


static void delete_char() {
	move_cursor_left();
	for (int y = 0; y < FONT_HEIGHT * text_size; y++) {
		for (int x = 0; x < FONT_WIDTH * text_size; x++) {
			put_pixel(BKG_COLOR, cursor_x + x, cursor_y + y);
		}
	}
	draw_cursor();
}

void vd_putchar(uint8_t ch, uint32_t color) {
	if (!tty_show || ch >= 128) {
		return;
	}
	switch (ch) {
		case '\b':
			delete_cursor();
			delete_char();
			break;
		case '\n':
			delete_cursor();
			vd_new_line();
			break;
		default:
			draw_bitmap(font[ch], cursor_x, cursor_y, color, text_size);
			move_cursor_right();
			draw_cursor();
			break;
	}
}

void vd_print(const char *str, uint32_t color) {
	if (!tty_show) {
		return;
	}
	for (int i = 0; str[i] != 0; i++) {
		vd_putchar(str[i], color);
	}

}

void vd_increase_text_size() {
	if (text_size < MAX_SIZE && tty_show) {
		text_size++;
	}
	update_cursor();
}

void vd_decrease_text_size() {
	if (text_size > 1 && tty_show) {
		text_size--;
	}
	update_cursor();
}

void vd_clear_tty() {
	if (!tty_show) {
		return;
	}
	uint64_t length = HEIGHT * WIDTH * BPP / 8;
	uint64_t color = BKG_COLOR | ((uint64_t)BKG_COLOR << 32);
	memset64((void *)(uintptr_t)VBE_mode_info->framebuffer, color, length); // hardcodeado que el background color es negro
	cursor_x = 0;
	cursor_y = 0;
}


