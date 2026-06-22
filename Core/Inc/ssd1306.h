//ssd1306.h
#pragma once
#include <stdint.h>

#define SSD1306_W  128
#define SSD1306_H  64

void ssd1306_init(void);
void ssd1306_clear(void);
void ssd1306_draw_pixel(int x, int y);
void ssd1306_draw_vline(int x, int y0, int y1);
void ssd1306_flush(void);
