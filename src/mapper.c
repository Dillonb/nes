#include <err.h>
#include <stdint.h>
#include <stdlib.h>

#include "util.h"
#include "rom.h"
#include "mapper.h"
#include "mapper0.h"
#include "mapper1.h"

byte mapper_prg_read(memory* mem, uint16_t address) {
    switch (mem->r->mapper) {
        case 0:
            return mapper0_prg_read(mem, address);
        case 1:
            return mapper1_prg_read(mem, address);
        default:
            errx(EXIT_FAILURE, "Unknown mapper %d!", mem->r->mapper);
    }
}

void mapper_prg_write(memory* mem, uint16_t address, byte value) {
    switch (mem->r->mapper) {
        case 0:
            mapper0_prg_write(mem, address, value);
            break;
        case 1:
            mapper1_prg_write(mem, address, value);
            break;
        default:
            errx(EXIT_FAILURE, "Unknown mapper %d!", mem->r->mapper);
    }
}
