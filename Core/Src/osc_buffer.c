//osc_buffer.c
#include "osc_buffer.h"
#include <string.h>

uint16_t history[HISTORY_SIZE];
volatile uint32_t history_write = 0;
uint16_t adc_dma_buf[ADC_DMA_SIZE];

void osc_buffer_init(void) {
    memset(history, 0, sizeof(history));
}

void osc_buffer_push(uint16_t *src, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        history[history_write] = src[i];
        history_write = (history_write + 1) % HISTORY_SIZE;
    }
}

int osc_find_trigger(uint32_t window_samples, uint32_t *trigger_idx) {
    /* Search only in the region that has at least window_samples after it.
       i counts backwards from wr, so idx1 = wr - i.
       Reading forward window_samples from idx1 reaches wr - i + window_samples.
       To stay within written data: i >= window_samples.
       Search up to 3x window for a trigger, capped by buffer size. */
    uint32_t search_start = window_samples;          /* must have room after trigger */
    uint32_t search_end   = window_samples * 3;
    if (search_end > HISTORY_SIZE - 2) search_end = HISTORY_SIZE - 2;

    uint32_t wr = history_write;

    for (uint32_t i = search_start; i < search_end; i++) {
        uint32_t idx1 = (wr - i     + HISTORY_SIZE) % HISTORY_SIZE;
        uint32_t idx0 = (wr - i - 1 + HISTORY_SIZE) % HISTORY_SIZE;

        if (history[idx0] < TRIGGER_THRESH && history[idx1] >= TRIGGER_THRESH) {
            *trigger_idx = idx1;
            return 1;
        }
    }
    return 0;
}
