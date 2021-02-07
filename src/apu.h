#pragma once
#include "util.h"

#define AUDIO_SAMPLE_RATE 44100
#define APU_RING_BUFFER_SIZE 10000
#define APU_STEPS_PER_SAMPLE (CPU_FREQUENCY / AUDIO_SAMPLE_RATE)
#define APU_STEPS_PER_FRAME_COUNTER_STEP (CPU_FREQUENCY / 240.0)

#define FC_4STEP 0
#define FC_5STEP 1

typedef struct pulse_oscillator_t {
    bool enable;

    uint16_t timer_register; // 11 bit value. Written through register. Selects frequency.
    uint16_t timer_step; // Progress through timer period. Updated internally.
    byte duty_value; // 3 bit value. Written through register. Selects waveform.
    byte duty_step; // Current position in the duty byte. Updated internally when timer hits 0.
    byte length_counter; // Set by register, from lookup table. Decremented internally.
    bool length_counter_halt;
    bool constant_volume;

    byte vol_and_env_period;

    bool envelope_start;
    byte envelope_volume;
    byte envelope_period_value;

    bool sweep_enabled;
    byte sweep_period;
    bool sweep_negate;
    byte sweep_shift_count;
    bool sweep_reload;
    byte sweep_counter;
} pulse_oscillator;

typedef struct triangle_oscillator_t {
    bool enable;

    uint16_t timer_register; // 11 bit value. Written through register. Selects frequency.
    uint16_t timer_step; // Progress through timer period. Updated internally.
    bool length_counter_halt;
    byte length_counter; // Set by register, from lookup table. Decremented internally.

    byte duty_step;

    bool linear_counter_reload;
    byte linear_counter_load;
    byte linear_counter;
} triangle_oscillator;

typedef struct noise_oscillator_t {
    bool enable;

    bool length_counter_halt;
    bool cv_or_env;
    byte vol_and_env_period;

    bool mode;

    uint16_t timer_register;
    uint16_t timer_step;

    byte length_counter;

    uint16_t lfsr;
    bool envelope_start;
    byte envelope_period_value;
    byte envelope_volume;
} noise_oscillator;

typedef struct dmc_oscillator_t {
    bool enable;
    uint16_t sample_address_register;
    uint16_t sample_address;

    uint16_t sample_length_register;
    uint16_t sample_length;

    bool irq_enabled;
    bool loop;
    byte rate;
    byte level;
    int sample_bit;
    byte output_buffer;
    byte tick;
} dmc_oscillator;

typedef struct apu_memory_t {
    long cycle;
    float buffer[APU_RING_BUFFER_SIZE];
    volatile long buffer_write_index;
    volatile long buffer_read_index;

    pulse_oscillator pulse1;
    pulse_oscillator pulse2;

    triangle_oscillator triangle;

    noise_oscillator noise;

    int frame_counter_mode;
    bool interrupt_inhibit;
    byte frame_counter;
    dmc_oscillator dmc;
} apu_memory;

apu_memory get_apu_mem();

byte read_apu_status(apu_memory *apu_mem);
void write_apu_register(apu_memory* apu_mem, int register_num, byte value);
void apu_step(apu_memory* apu_mem);
void apu_init(apu_memory* apu_mem);
void set_apu_tracker_enabled(bool enabled);
