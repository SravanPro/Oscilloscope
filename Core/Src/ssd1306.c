//ssd1306.h
#include "ssd1306.h"
#include "main.h"
#include "spi.h"
#include <string.h>

static uint8_t fb[SSD1306_W * SSD1306_H / 8];

static void cs_low  (void) { HAL_GPIO_WritePin(OLED_CS_GPIO_Port,  OLED_CS_Pin,  GPIO_PIN_RESET); }
static void cs_high (void) { HAL_GPIO_WritePin(OLED_CS_GPIO_Port,  OLED_CS_Pin,  GPIO_PIN_SET);   }
static void dc_cmd  (void) { HAL_GPIO_WritePin(OLED_DC_GPIO_Port,  OLED_DC_Pin,  GPIO_PIN_RESET); }
static void dc_data (void) { HAL_GPIO_WritePin(OLED_DC_GPIO_Port,  OLED_DC_Pin,  GPIO_PIN_SET);   }
static void rst_low (void) { HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_RESET); }
static void rst_high(void) { HAL_GPIO_WritePin(OLED_RST_GPIO_Port, OLED_RST_Pin, GPIO_PIN_SET);   }

static void send_cmd(uint8_t cmd) {
    dc_cmd(); cs_low();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, 10);
    cs_high();
}

static void send_data(uint8_t *buf, uint16_t len) {
    dc_data(); cs_low();
    HAL_SPI_Transmit(&hspi2, buf, len, 100);
    cs_high();
}

void ssd1306_init(void) {
    rst_low(); HAL_Delay(10);
    rst_high(); HAL_Delay(10);

    const uint8_t init_seq[] = {
        0xAE,
        0xD5, 0x80,
        0xA8, 0x3F,
        0xD3, 0x00,
        0x40,
        0x8D, 0x14,
        0x20, 0x00,
        0xA1,
        0xC8,
        0xDA, 0x12,
        0x81, 0xCF,
        0xD9, 0xF1,
        0xDB, 0x40,
        0xA4,
        0xA6,
        0xAF
    };
    for (uint8_t i = 0; i < sizeof(init_seq); i++) send_cmd(init_seq[i]);
}

void ssd1306_clear(void) {
    memset(fb, 0, sizeof(fb));
}

void ssd1306_draw_pixel(int x, int y) {
    if (x < 0 || x >= SSD1306_W || y < 0 || y >= SSD1306_H) return;
    fb[x + (y / 8) * SSD1306_W] |= (1 << (y % 8));
}

void ssd1306_draw_vline(int x, int y0, int y1) {
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    for (int y = y0; y <= y1; y++) ssd1306_draw_pixel(x, y);
}

void ssd1306_flush(void) {
    send_cmd(0x21); send_cmd(0); send_cmd(127);
    send_cmd(0x22); send_cmd(0); send_cmd(7);
    send_data(fb, sizeof(fb));
}
