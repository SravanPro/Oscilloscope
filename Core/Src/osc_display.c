//osc_display.c
#include "osc_display.h"
#include "osc_buffer.h"
#include "ssd1306.h"

static int sample_to_y(uint16_t s) {
    int y = 63 - (int)(s * 63 / 4095);
    if (y < 0)  y = 0;
    if (y > 63) y = 63;
    return y;
}

void osc_display_frame(float window_sec) {
    uint32_t window_samples = (uint32_t)(window_sec * 20000.0f);
    if (window_samples < 1)            window_samples = 1;
    if (window_samples > HISTORY_SIZE) window_samples = HISTORY_SIZE;

    uint32_t start_idx;
    if (!osc_find_trigger(window_samples, &start_idx)) {
        uint32_t wr = history_write;
        start_idx = (wr - window_samples + HISTORY_SIZE) % HISTORY_SIZE;
    }

    ssd1306_clear();

    for (int col = 0; col < 128; col++) {
        uint32_t s_start = (uint32_t)((float)col       * window_samples / 128.0f);
        uint32_t s_end   = (uint32_t)((float)(col + 1) * window_samples / 128.0f);
        if (s_end >= window_samples) s_end = window_samples - 1;

        uint16_t vmin = 4095, vmax = 0;
        for (uint32_t s = s_start; s <= s_end; s++) {
            uint32_t idx = (start_idx + s) % HISTORY_SIZE;
            uint16_t val = history[idx];
            if (val < vmin) vmin = val;
            if (val > vmax) vmax = val;
        }

        int y0 = sample_to_y(vmax);
        int y1 = sample_to_y(vmin);
        ssd1306_draw_vline(col, y0, y1);
    }

    ssd1306_flush();
}
