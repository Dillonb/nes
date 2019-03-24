#include <err.h>
#include <stdint.h>
#include <stdlib.h>

#include "../util.h"
#include "rom.h"
#include "mapper.h"
#include "mapper0.h"
#include "mapper1.h"
#include "mapper2.h"
#include "mapper4.h"
#include "mapper7.h"

void mapper_init(rom* r) {
    switch (r->mapper) {
        case 0:
            mapper0_init(&r->mapperdata);
            break;
        case 1:
            mapper1_init(&r->mapperdata);
            break;
        case 2:
            mapper2_init(&r->mapperdata);
            break;
        case 4:
            mapper4_init(r);
            break;
        case 7:
            break;
        default:
            errx(EXIT_FAILURE, "init: Unknown mapper %d!", r->mapper);
    }
}

byte mapper_prg_read(rom* r, uint16_t address) {
    switch (r->mapper) {
        case 0:
            return mapper0_prg_read(r, address);
        case 1:
            return mapper1_prg_read(r, address);
        case 2:
            return mapper2_prg_read(r, address);
        case 4:
            return mapper4_prg_read(r, address);
        case 7:
            return mapper7_prg_read(r, address);
        default:
            errx(EXIT_FAILURE, "prg read: Unknown mapper %d!", r->mapper);
    }
}

void mapper_prg_write(rom* r, uint16_t address, byte value) {
    switch (r->mapper) {
        case 0:
            mapper0_prg_write(r, address, value);
            break;
        case 1:
            mapper1_prg_write(r, address, value);
            break;
        case 2:
            mapper2_prg_write(r, address, value);
            break;
        case 4:
            mapper4_prg_write(r, address, value);
            break;
        case 7:
            mapper7_prg_write(r, address, value);
            break;
        default:
            errx(EXIT_FAILURE, "prg write: Unknown mapper %d!", r->mapper);
    }
}

byte mapper_chr_read(rom* r, uint16_t address) {
    switch (r->mapper) {
        case 0:
            return mapper0_chr_read(r, address);
        case 1:
            return mapper1_chr_read(r, address);
        case 2:
            return mapper2_chr_read(r, address);
        case 4:
            return mapper4_chr_read(r, address);
        case 7:
            return mapper7_chr_read(r, address);
        default:
            errx(EXIT_FAILURE, "chr read: Unknown mapper %d!", r->mapper);
    }
}

void mapper_chr_write(rom* r, uint16_t address, byte value) {
    switch (r->mapper) {
        case 0:
            mapper0_chr_write(r, address, value);
            break;
        case 1:
            mapper1_chr_write(r, address, value);
            break;
        case 2:
            mapper2_chr_write(r, address, value);
            break;
        case 4:
            mapper4_chr_write(r, address, value);
            break;
        case 7:
            mapper7_chr_write(r, address, value);
            break;
        default:
            errx(EXIT_FAILURE, "chr write: Unknown mapper %d!", r->mapper);
    }
}

void mapper_ppu_step(rom *r, int cycle, int scan_line, bool rendering_enabled) {
    switch (r->mapper) {
        case 4:
            mapper4_ppu_step(r, cycle, scan_line, rendering_enabled);
            break;
        default:
            break;
    }
}