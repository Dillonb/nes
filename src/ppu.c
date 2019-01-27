#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#include "ppu.h"
#include "cpu.h"
#include "debugger.h"

#define VBLANK_LINE 241

typedef enum high_or_low_t {
    HIGH,
    LOW
} high_or_low;

high_or_low address_byte = HIGH;

ppu_memory get_ppu_mem() {
    ppu_memory ppu_mem;
    ppu_mem.control    = 0b00000000;
    ppu_mem.mask       = 0b00000000;
    ppu_mem.status     = 0b10100000;
    ppu_mem.oamAddress = 0b00000000;
    ppu_mem.oamData    = 0b00000000;
    ppu_mem.scroll     = 0b00000000;
    ppu_mem.address    = 0x0000;
    ppu_mem.data       = 0b00000000;
    ppu_mem.dma        = 0b00000000;

    ppu_mem.frame = 0;
    ppu_mem.scan_line = 0;
    ppu_mem.cycle = 0;

    return ppu_mem;
}

// TODO implement me for real.
void vram_write(ppu_memory* ppu_mem, uint16_t address, byte value) {
    printf("Writing 0x%02X to 0x%04X WARNING, currently this has no effect.\n", value, address);
}

bool get_control_flag(ppu_memory* mem, int index) {
    return (mem->control & mask_flag(index)) > 0;
}

bool vblank_nmi(ppu_memory* mem) {
    return get_control_flag(mem, 7);
}

bool get_addr_increment_flag(ppu_memory* mem) {
    return get_control_flag(mem, 2);
}

int get_addr_increment(ppu_memory* mem) {
    return get_addr_increment_flag(mem) ? 32 : 1;
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
    ppu_mem->status |= 0b10000000; // Set VBlank flag on PPUSTATUS
    if (vblank_nmi(ppu_mem)) {
        trigger_nmi();
    }
}

void clear_vblank(ppu_memory* ppu_mem) {
    ppu_mem->status &= 0b01111111; // Clear VBlank flag on PPUSTATUS
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

byte read_status_sideeffects(ppu_memory* ppu_mem) {
    printf("WARNING: returning status register with sideeffects\n");
    //debugger_wait();
    byte oldval = ppu_mem->status;
    clear_vblank(ppu_mem);
    return oldval;
}

byte read_ppu_register(ppu_memory* ppu_mem, byte register_num) {
    switch (register_num) {
        case 2:
            return read_status_sideeffects(ppu_mem);
        case 4:
        case 7:
            errx(EXIT_FAILURE, "PPU register %x read is not yet implemented.", register_num);
        default:
            errx(EXIT_FAILURE, "Tried to read invalid PPU register %x - only 2, 4, and 7 are capable of being read from", register_num);
    }
}

void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value) {
    // Update last 5 bits of status register _every_ write to _any_ ppu register
    byte last5 = value & 0b00011111;
    ppu_mem->status = (ppu_mem->status & 0b11100000) | last5;

    switch (register_num) {
        case 0:
            ppu_mem->control = value;
            printf("NMI on VBlank is now: %d\n", vblank_nmi(ppu_mem));
            return;
        case 1:
            ppu_mem->mask = value;
            return;
        /*
        case 2:
            return;
        */
        case 3:
            ppu_mem->oamAddress = value;
            return;
            /*
        case 4:
            ppu_mem->oamData = value;
            return;
        */
        case 5:
            if (value == 0x00) {
                printf("IGNORING WRITE TO SCROLL!\n");
            }
            else {
                errx(EXIT_FAILURE, "Nonzero write to scroll! Time to implement this thing\n");
            }
            return;

        case 6: {
            uint16_t new_address = ppu_mem->address;
            if (address_byte == HIGH) {
                // Mask out old high byte
                new_address &= 0x00FF;
                new_address |= ((uint16_t)value) << 8;
                address_byte = LOW;
                printf("Writing %02X to HIGH byte of PPUADDR. Old value 0x%04X New value 0x%04X\n", value, ppu_mem->address, new_address);
            }
            else if (address_byte == LOW) {
                // Mask out old low byte
                new_address &= 0xFF00;
                new_address |= value;
                address_byte = HIGH;
                printf("Writing %02X to LOW byte of PPUADDR. Old value 0x%04X New value 0x%04X\n", value, ppu_mem->address, new_address);
            }
            if (new_address > 0x3FFF) {
                errx(EXIT_FAILURE, "Wrote an address larger than 0x3FFF to PPUADDR but mirroring not implemented yet!");
            }
            ppu_mem->address = new_address;
            return;
        }
        case 7:
            vram_write(ppu_mem, ppu_mem->address, value);
            ppu_mem->address += get_addr_increment(ppu_mem);
            return;
        /*
            */
            /* $4014
               case 7:
               ppu_mem->dma = value;
               return;
            */
        default:
            errx(EXIT_FAILURE, "Tried to write 0x%02X to invalid or unimplemented PPU register %x", value, register_num);
    }
}
