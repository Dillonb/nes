#include "system.h"
#include "cpu.h"
#include "ppu.h"

int system_step(memory* mem) {
    int cpu_steps = cpu_step(mem);
    int ppu_steps = cpu_steps * 3; // 3 PPU steps for every CPU step
    for (int i = 0; i < ppu_steps; i++) {
        //ppu_step(&mem->ppu_mem);
    }

    // TODO step APU here. One APU step per CPU step.

    return cpu_steps;
}
