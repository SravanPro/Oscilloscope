//osc_buffer.h
#pragma once
#include <stdint.h>

#define HISTORY_SIZE   20000
#define ADC_DMA_SIZE   256
#define TRIGGER_THRESH 2048

extern uint16_t history[HISTORY_SIZE];
extern volatile uint32_t history_write;
extern uint16_t adc_dma_buf[ADC_DMA_SIZE];

void osc_buffer_init(void);
void osc_buffer_push(uint16_t *src, uint32_t count);
int  osc_find_trigger(uint32_t window_samples, uint32_t *trigger_idx);
