#include "apu.h"

byte read_apu_register(apu_memory* apu_mem, byte register_num) {

}

void write_apu_register(apu_memory* apu_mem, int register_num, byte value) {
    if (register_num < 0x4) {
        // Pulse 1
    } else if (register_num < 0x8) {
        // Pulse 2
    }
    else if (register_num < 0xC) {
        // Triangle
    }
    else if (register_num < 0x10) {
        // Noise
    }
    else if (register_num < 0x14) {
        // DMC
    }
    else if (register_num == 0x15) {
        // Channel enable and length counter status
    }
    else if (register_num == 0x17) {
        // Frame counter
    }
    else {
        printf("WARNING: Unhandled APU register write to 0x%04X\n", (register_num + 0x4000));
    }
}

void apu_step(apu_memory* apu_mem) {

}
