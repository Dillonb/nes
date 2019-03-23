#include "system.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

int system_step(memory* mem) {
    int cpu_steps = cpu_step(mem);

    for (int i = 0; i < cpu_steps; i++) {
        // This is a bit of a hack, but this is here to avoid a circular dependency.
        // The APU cannot know about the CPU's memory space.
        dmc_oscillator* dmc = &mem->apu_mem.dmc;
        if (dmc->enable && dmc->sample_length > 0 && dmc->sample_bit == 0) {
            dmc->output_buffer = read_byte(mem, dmc->sample_address);

            dmc->sample_bit = 8;
            if (++dmc->sample_address == 0) {
                dmc->sample_address = 0x8000;
            }
            if (--dmc->sample_length == 0 && dmc->loop) {
                dmc->sample_length = dmc->sample_length_register;
                dmc->sample_address = dmc->sample_address_register;
            }
            cpu_steps += 4; // Simulate time spent reading from memory
        }
        apu_step(&mem->apu_mem);
    }

    int ppu_steps = cpu_steps * 3; // 3 PPU steps for every CPU step
    for (int i = 0; i < ppu_steps; i++) {
        ppu_step(&mem->ppu_mem);
    }

    return cpu_steps;
}
