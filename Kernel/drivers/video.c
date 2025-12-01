// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "lib.h"
#include <font.h>
#include <video.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define HISTORY_LEN 4096

#define WIDTH (VBE_mode_info->width)
#define HEIGHT (VBE_mode_info->height)
#define BPP (VBE_mode_info->bpp)
#define PITCH (VBE_mode_info->pitch)
#define FB ((uintptr_t)VBE_mode_info->framebuffer)

typedef struct cell {
	char c;
	uint32_t color;
} cell_t;


static VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;


static cell_t tty_buffer[HISTORY_LEN];
static int buffer_begin = 0;
static int buffer_end = 0;

static int tty_show = 0;

static uint16_t cursor_x;
static uint16_t cursor_y;
static uint8_t text_size = 1;
static uint32_t bg_color = 0x000000;


// video

void get_video_info(video_info_t * buffer) {
	buffer->width = WIDTH;
	buffer->height = HEIGHT;
	buffer->pitch = PITCH;
	buffer->bpp = BPP;
	
}

void vd_present(void * framebuffer) {
	tty_show = 0;
	memcpy((void *) FB, framebuffer, HEIGHT * PITCH);
}

void vd_present_region(void * framebuffer, region_t * region) {
	tty_show = 0;
	uint8_t *src = (uint8_t *)framebuffer;
	uint8_t *dst = (uint8_t *)FB;
	uint64_t bytes_per_pixel = BPP / 8;
	uint64_t line_width_bytes = region->w * bytes_per_pixel;

	// Copy line by line
	for (uint16_t y = 0; y < region->h; y++) {
		uint64_t src_offset = ((region->y + y) * PITCH) + (region->x * bytes_per_pixel);
		uint64_t dst_offset = ((region->y + y) * PITCH) + (region->x * bytes_per_pixel);
		
		memcpy(dst + dst_offset, src + src_offset, line_width_bytes);
	}
}


// tty

static void vd_new_line(); // Forward declaration

static int is_within_screen_bounds(uint16_t x, uint16_t y) {
	return x < VBE_mode_info->width && y < VBE_mode_info->height;
}


static void put_pixel(uint32_t hex_color, uint16_t x, uint16_t y) {
	if (!is_within_screen_bounds(x, y)) {
		return;
	}
	uint8_t *framebuffer = (uint8_t *)FB;
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
	uint64_t color = bg_color | ((uint64_t)bg_color << 32);
	memset64(framebuffer + last_line_offset, color, bytes_to_clear);
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

static void draw_bitmap(uint8_t bitmap[FONT_HEIGHT], uint16_t x, uint16_t y, uint32_t color, uint64_t size) {
	for (int i = 0; i < FONT_HEIGHT; i++) {
		char line = bitmap[i];
		for (int j = 0; j < FONT_WIDTH; j++) {
			uint32_t pixel_color = (line << j) & 0x80 ? color : bg_color;
		
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

static void vd_new_line() {
	cursor_x = 0;
	uint64_t step_y = text_size * FONT_HEIGHT;

	// Verificar si hay espacio para una línea más sin hacer scroll
	if (cursor_y + step_y < HEIGHT) {
		cursor_y += step_y;
		// fill_rectangle(cursor_x, cursor_y, WIDTH,
		// 			cursor_y + FONT_HEIGHT * text_size, bg_color);
	} else {
		scroll_up();
		cursor_y = HEIGHT - step_y; // Posicionar cursor en la última línea
	}
	draw_cursor();
}

static void fill_screen(uint32_t color) {
	uint64_t length = HEIGHT * PITCH;
	uint64_t to_fill = color | ((uint64_t)color << 32);
	memset64((void *) FB, to_fill, length);
}

static int history_size() {
	if (buffer_end >= buffer_begin) {
		return buffer_end - buffer_begin;
	}
	return HISTORY_LEN - buffer_begin + buffer_end;
}

static int history_is_full() {
	return history_size() == HISTORY_LEN - 1;
}

static int history_is_empty() {
	return buffer_begin == buffer_end;
}

static void add_to_histroy(char c, uint32_t color) {
	if (history_is_full()) {
		// Remove oldest character
		buffer_begin = (buffer_begin + 1) % HISTORY_LEN;
	}
	
	tty_buffer[buffer_end].c = c;
	tty_buffer[buffer_end].color = color;
	buffer_end = (buffer_end + 1) % HISTORY_LEN;
}

static void delete_from_history() {
	if (history_is_empty()) {
		return;
	}
	
	// Move end pointer back
	buffer_end = (buffer_end - 1 + HISTORY_LEN) % HISTORY_LEN;
}

static void redraw_history() {
	fill_screen(bg_color);
	cursor_x = 0;
	cursor_y = 0;
	
	if (history_is_empty()) {
		draw_cursor();
		return;
	}
	
	// Calculate how many lines the history would take
	uint16_t temp_x = 0;
	uint16_t temp_y = 0;
	uint16_t step_x = FONT_WIDTH * text_size;
	uint16_t step_y = FONT_HEIGHT * text_size;
	
	int idx = buffer_begin;
	while (idx != buffer_end) {
		char c = tty_buffer[idx].c;
		
		if (c == '\n') {
			temp_x = 0;
			temp_y += step_y;
		} else {
			temp_x += step_x;
			if (temp_x + step_x > WIDTH) {
				temp_x = 0;
				temp_y += step_y;
			}
		}
		
		idx = (idx + 1) % HISTORY_LEN;
	}
	
	// If content would go off-screen, skip lines from the beginning
	int lines_to_skip = 0;
	if (temp_y >= HEIGHT) {
		lines_to_skip = (temp_y - HEIGHT) / step_y + 1;
	}
	
	// Now redraw, skipping the first lines_to_skip lines
	int current_line = 0;
	temp_x = 0;
	temp_y = 0;
	
	idx = buffer_begin;
	while (idx != buffer_end) {
		char c = tty_buffer[idx].c;
		uint32_t color = tty_buffer[idx].color;
		
		if (c == '\n') {
			current_line++;
			temp_x = 0;
			if (current_line > lines_to_skip) {
				temp_y += step_y;
			}
		} else {
			// Handle line wrapping
			if (temp_x + step_x > WIDTH) {
				current_line++;
				temp_x = 0;
				if (current_line > lines_to_skip) {
					temp_y += step_y;
				}
			}
			
			// Only draw if we're past the skipped lines
			if (current_line >= lines_to_skip) {
				draw_bitmap(font[(uint8_t)c], temp_x, temp_y, color, text_size);
			}
			
			temp_x += step_x;
		}
		
		idx = (idx + 1) % HISTORY_LEN;
	}
	
	cursor_x = temp_x;
	cursor_y = temp_y;
	draw_cursor();
}




static void delete_char() {
	if (history_is_empty()) {
		return;
	}
	
	// Check what character we're deleting
	int last_idx = (buffer_end - 1 + HISTORY_LEN) % HISTORY_LEN;
	char deleted_char = tty_buffer[last_idx].c;
	
	delete_from_history();
	
	if (!tty_show) {
		return;
	}
	
	delete_cursor();
	
	// If we're deleting a newline, we need to redraw since content structure changed
	if (deleted_char == '\n') {
		redraw_history();
	} else {
		// Normal character deletion - just move cursor back and clear
		move_cursor_left();
		for (int y = 0; y < FONT_HEIGHT * text_size; y++) {
			for (int x = 0; x < FONT_WIDTH * text_size; x++) {
				put_pixel(bg_color, cursor_x + x, cursor_y + y);
			}
		}
		draw_cursor();
	}
}

void vd_putchar(char c, uint32_t color) {
	if (c < 0 || c >= 128) {
		return;
	}

	switch (c) {
		case '\b':
			if (tty_show) {
				delete_cursor();
			}
			delete_char();
			break;
		case '\n':
			add_to_histroy(c, color);
			if (tty_show) {
				delete_cursor();
				vd_new_line();
			}
			break;
		default:
			add_to_histroy(c, color);
			if (tty_show) {
				draw_bitmap(font[(uint8_t)c], cursor_x, cursor_y, color, text_size);
				move_cursor_right();
				draw_cursor();
			}
			break;
	}
}

void vd_print(const char *str, uint32_t color) {
	for (int i = 0; str[i] != 0; i++) {
		vd_putchar(str[i], color);
	}
}


void vd_show_tty() {
	if (tty_show) {
		return;
	}
	tty_show = 1;
	redraw_history();
}

void vd_clear_tty() {
	buffer_begin = 0;
	buffer_end = 0;

	if (tty_show) {
		cursor_x = 0;
		cursor_y = 0;
		fill_screen(bg_color);
	}
	
	
}

void vd_set_bg_color(uint32_t color) {
	bg_color = color;
	redraw_history();
}

void vd_set_text_size(int size) {
	if (size < MIN_TEXT_SIZE) {
		text_size = MIN_TEXT_SIZE;
	} else if (size > MAX_TEXT_SIZE) {
		text_size = MAX_TEXT_SIZE;
	} else {
		text_size = size; 
		redraw_history();
	}
}


int vd_get_text_size() { return text_size; }






