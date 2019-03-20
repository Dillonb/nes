#pragma once
#include "util.h"

#define SAMPLE_RATE_DEF 44100

static const double AUDIO_SAMPLE_RATE = SAMPLE_RATE_DEF;
#define APU_RING_BUFFER_SIZE 10000
static const double APU_STEPS_PER_SAMPLE = CPU_FREQUENCY / AUDIO_SAMPLE_RATE;
static const double APU_STEPS_PER_FRAME_COUNTER_STEP = CPU_FREQUENCY / 240.0;

#define FC_4STEP 0
#define FC_5STEP 1

typedef struct pulse_oscillator_t {
    uint16_t timer_register; // 11 bit value. Written through register. Selects frequency.
    uint16_t timer_step; // Progress through timer period. Updated internally.
    byte duty_value; // 3 bit value. Written through register. Selects waveform.
    byte duty_step; // Current position in the duty byte. Updated internally when timer hits 0.
    byte length_counter; // Set by register, from lookup table. Decremented internally.
    bool enable;
    bool length_counter_halt;
    bool constant_volume;
} pulse_oscillator;

typedef struct apu_memory_t {
    long cycle;
    float buffer[APU_RING_BUFFER_SIZE];
    volatile long buffer_write_index;
    volatile long buffer_read_index;

    pulse_oscillator pulse1;
    pulse_oscillator pulse2;

    int frame_counter_mode;
    bool interrupt_inhibit;
    byte frame_counter;
} apu_memory;

apu_memory get_apu_mem();

byte read_apu_register(apu_memory* apu_mem, byte register_num);
void write_apu_register(apu_memory* apu_mem, int register_num, byte value);
void apu_step(apu_memory* apu_mem);
void apu_init(apu_memory* apu_mem);