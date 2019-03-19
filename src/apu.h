#pragma once
#include "util.h"

const double AUDIO_SAMPLE_RATE = 44100;
const double STEPS_PER_SAMPLE = CPU_FREQUENCY / AUDIO_SAMPLE_RATE;

typedef struct apu_memory_t {
    long cycle;
} apu_memory;

apu_memory get_apu_mem();

byte read_apu_register(apu_memory* apu_mem, byte register_num);
void write_apu_register(apu_memory* apu_mem, int register_num, byte value);
void apu_step(apu_memory* apu_mem);
void apu_init(apu_memory* apu_mem);