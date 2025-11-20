#ifndef _VIDEO_DRIVER_H_
#define _VIDEO_DRIVER_H_

#include <stdint.h>


struct vbe_mode_info_structure {
  uint16_t
      attributes; // deprecated, only bit 7 should be of interest to you, and it
                  // indicates the mode supports a linear frame buffer.
  uint8_t window_a;     // deprecated
  uint8_t window_b;     // deprecated
  uint16_t granularity; // deprecated; used while calculating bank numbers
  uint16_t window_size;
  uint16_t segment_a;
  uint16_t segment_b;
  uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode
                         // without returning to real mode

  uint16_t pitch;  // number of bytes per horizontal line
  uint16_t width;  // width in pixels
  uint16_t height; // height in pixels

  uint8_t w_char; // unused...
  uint8_t y_char; // ...
  uint8_t planes;

  uint8_t bpp; // bits per pixel in this mode

  uint8_t banks; // deprecated; total number of banks in this mode
  uint8_t memory_model;
  uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may
                     // be 16 KB...
  uint8_t image_pages;
  uint8_t reserved0;

  uint8_t red_mask;
  uint8_t red_position;
  uint8_t green_mask;
  uint8_t green_position;
  uint8_t blue_mask;
  uint8_t blue_position;
  uint8_t reserved_mask;
  uint8_t reserved_position;
  uint8_t direct_color_attributes;

  uint32_t framebuffer; // physical address of the linear frame buffer; write
                        // here to draw to the screen

  uint32_t off_screen_mem_off;
  uint16_t off_screen_mem_size; // size of memory in the framebuffer but not
                                // being displayed on the screen
  uint8_t reserved1[206];
} __attribute__((packed));

typedef struct vbe_mode_info_structure *VBEInfoPtr;

// dibuja un pixel en pantalla
void     put_pixel(uint32_t hex_color, uint64_t x, uint64_t y);
uint16_t get_screen_height();
uint16_t get_screen_width();
uint8_t get_screen_bpp();

/*  FUNCIONES DE MODO TEXTO  */
void    enable_text_mode();
void    disable_text_mode();
void    vd_set_text_size(uint8_t size);
uint8_t vd_get_text_size();
void    vd_increase_text_size();
void    vd_decrease_text_size();
void    vd_put_char(uint8_t ch, uint32_t color);
void    vd_print(const char *str, uint32_t color);
void    vd_clear();
void    newline();

/*  FUNCIONES DE DIBUJO */

// x,y son esquina superior izquierda
void vd_draw_char(uint8_t ch, uint64_t x, uint64_t y, uint32_t color, uint64_t size);
void vd_draw_string(const char *str, uint64_t x, uint64_t y, uint32_t color, uint64_t size);
// dibuja una linea desde x0,y0 hasta x1,y1
void vd_draw_line(uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);
// x0,y0 esquina superior izquierda. x1,y1 esquina inferior derecha
void draw_rectangle(uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);
void fill_rectangle(uint64_t x0, uint64_t y0, uint64_t x1, uint64_t y1, uint32_t color);
void draw_circle(uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color);
void fill_circle(uint64_t x_center, uint64_t y_center, uint64_t radius, uint32_t color);

#endif 