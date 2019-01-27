#include <stdio.h>
#include <stdbool.h>

#include "ppu.h"
#include "cpu.h"

#define VBLANK_LINE 241


ppu_memory get_ppu_mem() {
    ppu_memory ppu_mem;
    ppu_mem.control    = 0b00000000;
    ppu_mem.mask       = 0b00000000;
    ppu_mem.status     = 0b10100000;
    ppu_mem.oamAddress = 0b00000000;
    ppu_mem.oamData    = 0b00000000;
    ppu_mem.scroll     = 0b00000000;
    ppu_mem.address    = 0b00000000;
    ppu_mem.data       = 0b00000000;
    ppu_mem.dma        = 0b00000000;

    ppu_mem.frame = 0;
    ppu_mem.scan_line = 0;
    ppu_mem.cycle = 0;

    return ppu_mem;
}

bool get_control_flag(ppu_memory* mem, int index) {
    return (mem->control & mask_flag(index)) > 0;
}

bool vblank_nmi(ppu_memory* mem) {
    return get_control_flag(mem, 7);
}


// 0 - 261
// 0: Pre-render
// 1-240: Visible
// 241: Post-render
// 242-261: VBLANK
#define NUM_LINES 262
// 0 - 340
#define CYCLES_PER_LINE 341

bool is_visible(ppu_memory* ppu_mem) {
    // Pre-render scanline
    if (ppu_mem->scan_line == 0) {
        return false;
    }
    // Post-render and VBLANK
    if (ppu_mem->scan_line > 240) {
        return false;
    }

    return true;
}

void set_vblank(ppu_memory* ppu_mem) {
    if (vblank_nmi(ppu_mem)) {
        trigger_nmi();
    }
}

void clear_vblank(ppu_memory* ppu_mem) {
}

void ppu_step(ppu_memory* ppu_mem) {
    ppu_mem->cycle++;
    if (ppu_mem->cycle >= CYCLES_PER_LINE) {
        ppu_mem->cycle = 0;
        ppu_mem->scan_line++;
        if (ppu_mem->scan_line >= NUM_LINES) {
            ppu_mem->frame++;
            ppu_mem->scan_line = 0;
        }
    }

    if (ppu_mem->cycle == 0) {
        // Idle cycle
    }
    else if (ppu_mem->cycle == 1) {
        if (ppu_mem->scan_line == 0) {
            clear_vblank(ppu_mem);
        }
        else if (ppu_mem->scan_line == VBLANK_LINE) {
            set_vblank(ppu_mem);
        }
    }
}

byte read_ppu_register(ppu_memory* ppu_mem, byte register_num) {
    switch (register_num) {
        case 0:
            return ppu_mem->control;
        case 1:
            return ppu_mem->mask;
        case 2:
            return ppu_mem->status;
        case 3:
            return ppu_mem->oamAddress;
        case 4:
            return ppu_mem->oamData;
        case 5:
            return ppu_mem->scroll;
        case 6:
            return ppu_mem->address;
        case 7:
            return ppu_mem->data;
            /* $4014
               case something:
               return ppu_mem->dma;
            */
        default:
            printf("WARNING: tried to read invalid PPU register %x, returning 0x00\n", register_num);
            return 0x00;
    }
}

void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value) {
    switch (register_num) {
        case 0:
            ppu_mem->control = value;
            printf("NMI on VBlank is now: %d\n", vblank_nmi(ppu_mem));
            return;
        case 1:
            ppu_mem->mask = value;
            return;
        case 2:
            return;
        case 3:
            ppu_mem->oamAddress = value;
            return;
        case 4:
            ppu_mem->oamData = value;
            return;
        case 5:
            ppu_mem->scroll = value;
            return;
        case 6:
            ppu_mem->address = value;
            return;
        case 7:
            ppu_mem->data = value;
            return;
            /* $4014
               case 7:
               ppu_mem->dma = value;
               return;
            */
        default:
            printf("WARNING: tried to write invalid PPU register %x, doing nothing\n", register_num);
            return;
    }
}
