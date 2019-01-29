#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#include "ppu.h"
#include "cpu.h"
#include "debugger.h"

#define VBLANK_LINE 241

ppu_memory get_ppu_mem() {
    ppu_memory ppu_mem;
    ppu_mem.control     = 0b00000000;
    ppu_mem.mask        = 0b00000000;
    ppu_mem.status      = 0b10100000;
    ppu_mem.oam_address = 0b00000000;
    ppu_mem.scroll      = 0b00000000;
    ppu_mem.v           = 0b0000000000000000;
    ppu_mem.t           = 0b0000000000000000;
    ppu_mem.w           = HIGH;
    ppu_mem.data        = 0b00000000;
    ppu_mem.dma         = 0b00000000;

    ppu_mem.frame = 0;
    ppu_mem.scan_line = 0;
    ppu_mem.cycle = 0;

    return ppu_mem;
}

void vram_write(ppu_memory* ppu_mem, uint16_t address, byte value) {
    dprintf("Writing 0x%02X to PPU VRAM address 0x%04X\n", value, address);

    // Nametables
    if (address < 0x3F00 && address >= 0x2000) {
        uint16_t index = (address - 2000) % 0x1000;
        ppu_mem->name_tables[index] = value;
    }
    // Palette RAM indexes
    else if (address < 0x4000) {
        uint16_t index = (address - 0x3F00) % 32;
        ppu_mem->palette_ram[index] = value;
    }
    else {
        errx(EXIT_FAILURE, "Attempted to write 0x%02X to UNKNOWN PPU VRAM ADDRESS 0x%04X", value, address);
    }
}

byte vram_read(ppu_memory* ppu_mem, uint16_t address) {
    byte result;

    if (address < 0x3F00 && address >= 0x2000) {
        uint16_t index = (address - 2000) % 0x1000;
        result = ppu_mem->name_tables[index];
    }
    // Palette RAM indexes
    else if (address < 0x4000) {
        uint16_t index = (address - 0x3F00) % 32;
        result = ppu_mem->palette_ram[index];
    }
    else {
        errx(EXIT_FAILURE, "Read from UNKNOWN PPU VRAM ADDRESS 0x%04X", address);
    }

    dprintf("Read 0x%02X from PPU VRAM address 0x%04X\n", result, address);
    debugger_wait();

    return result;
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

bool rendering_enabled(ppu_memory* ppu_mem) {
    return (ppu_mem->mask & 0b00011000) > 0; // Enable sprites OR enable background flags enabled
}

bool is_line_visible(ppu_memory* ppu_mem) {
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

bool is_cycle_visible(ppu_memory* ppu_mem) {
    if (ppu_mem->cycle == 0) {
        return false;
    }

    if (ppu_mem->cycle > 256) {
        return false;
    }

    return true;
}

bool is_visible(ppu_memory* ppu_mem) {
    return is_line_visible(ppu_mem) && is_cycle_visible(ppu_mem);
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

void render_pixel(ppu_memory* ppu_mem) {

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

    if (rendering_enabled(ppu_mem)) {
        if (is_visible(ppu_mem)) {
            render_pixel(ppu_mem);
        }

        if (ppu_mem->cycle == 0) {
            // Idle cycle
        }
        // fetch
        else if (ppu_mem->cycle <= 256) {
            // Fetch tile data for
        }
        else if (ppu_mem->cycle <= 320) {
        }

        int which_fetch = ppu_mem->cycle % 8;

        switch (which_fetch) {
            case 1:
                break;
            case 3:
                break;
            case 5:
                break;
            case 7:
                break;
            case 0:
                break;
                
        }

        // TODO fetches and stuff that only happen when rendering enabled
    }

    if (ppu_mem->cycle == 1) {
        if (ppu_mem->scan_line == 0) {
            clear_vblank(ppu_mem);
        }
        else if (ppu_mem->scan_line == VBLANK_LINE) {
            set_vblank(ppu_mem);
        }
    }
}

byte read_status_sideeffects(ppu_memory* ppu_mem) {
    dprintf("WARNING: returning status register with sideeffects\n");
    byte oldval = ppu_mem->status;
    clear_vblank(ppu_mem);
    ppu_mem->w = HIGH;
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

void write_oam_byte(ppu_memory* ppu_mem, byte value) {
    ppu_mem->oam_data[ppu_mem->oam_address++] = value;
}

void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value) {
    // Update last 5 bits of status register _every_ write to _any_ ppu register
    byte last5 = value & 0b00011111;
    ppu_mem->status = (ppu_mem->status & 0b11100000) | last5;

    switch (register_num) {
        case 0:
            ppu_mem->control = value;
            dprintf("NMI on VBlank is now: %d\n", vblank_nmi(ppu_mem));
            return;
        case 1:
            ppu_mem->mask = value;
            return;
        case 3:
            ppu_mem->oam_address = value;
            return;
        case 4:
            write_oam_byte(ppu_mem, value);
            return;
        case 5:
            if (value == 0x00) {
                printf("IGNORING WRITE TO SCROLL!\n");
            }
            else {
                errx(EXIT_FAILURE, "Nonzero write to scroll! Time to implement this thing\n");
            }
            return;

        case 6: {
            if (ppu_mem->w == HIGH) {
                // Mask out old high byte
                ppu_mem->t &= 0x00FF;
                ppu_mem->t |= ((uint16_t)value) << 8;
                ppu_mem->w = LOW;
            }
            else if (ppu_mem->w == LOW) {
                // Mask out old low byte
                ppu_mem->t &= 0xFF00;
                ppu_mem->t |= value;
                ppu_mem->v = ppu_mem->t;
                ppu_mem->w = HIGH;
            }
            if (ppu_mem->t > 0x3FFF) {
                errx(EXIT_FAILURE, "Wrote an address larger than 0x3FFF to PPUADDR but mirroring not implemented yet!");
            }
            return;
        }
        case 7:
            vram_write(ppu_mem, ppu_mem->v, value);
            ppu_mem->v += get_addr_increment(ppu_mem);
            return;
        default:
            errx(EXIT_FAILURE, "Tried to write 0x%02X to invalid or unimplemented PPU register %x", value, register_num);
    }
}
