#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#include "ppu.h"
#include "cpu.h"
#include "debugger.h"
#include "render.h"
#include "palette.h"

#define VBLANK_LINE 241

ppu_memory get_ppu_mem(rom* r) {
    ppu_memory ppu_mem;

    ppu_mem.r = r;

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

    ppu_mem.tile.attribute_table  = 0;
    ppu_mem.tile.nametable        = 0;
    ppu_mem.tile.tile_bitmap_high = 0;
    ppu_mem.tile.tile_bitmap_low  = 0;

    ppu_mem.frame = 0;
    ppu_mem.scan_line = 0;
    ppu_mem.cycle = 0;

    return ppu_mem;
}

void increment_x(ppu_memory* ppu_mem) {
    if ((ppu_mem->v & 0x001F) == 31) {
        ppu_mem->v &= ~0x001F;
        ppu_mem->v ^= 0x0400;
    }
    else {
        ppu_mem->v += 1;
    }
}

void increment_y(ppu_memory* ppu_mem) {
    if ((ppu_mem->v & 0x7000) != 0x7000) {
        ppu_mem->v += 0x1000;
    }
    else {
        ppu_mem->v &= ~0x7000;
        int y = (ppu_mem->v & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            ppu_mem->v ^= 0x0800;
        }
        else if (y == 31) {
            y = 0;
        }
        else {
            y += 1;
            ppu_mem->v = (ppu_mem->v & ~0x03E0) | (y << 5);
        }
    }
}

uint16_t get_nametable_address(ppu_memory* ppu_mem) {
    return 0x2000 | (ppu_mem->v & 0x0FFF);
}

uint16_t get_attribute_address(ppu_memory* ppu_mem) {
    return 0x23C0 | (ppu_mem->v & 0x0C00) | ((ppu_mem->v >> 4) & 0x38) | ((ppu_mem->v >> 2) & 0x07);
}

void vram_write(ppu_memory* ppu_mem, uint16_t address, byte value) {
    dprintf("Writing 0x%02X to PPU VRAM address 0x%04X\n", value, address);

    // Pattern tables
    if (address < 0x2000) {
        errx(EXIT_FAILURE, "Pattern tables written to! NROM can't do this (CHR ROM), does this ROM use a different mapper?");
    }
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

    // Pattern tables
    if (address < 0x2000) {
        // NROM
        if (ppu_mem->r->mapper == 0) {
            result = ppu_mem->r->chr_rom[address];
        }
        else {
            errx(EXIT_FAILURE, "Mapper 0x%02X not implemented in PPU!", ppu_mem->r->mapper);
        }
    }
    // Nametables
    else if (address < 0x3F00) {
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

uint16_t get_background_table_base_address(ppu_memory* mem) {
    return get_control_flag(mem, 4) * 0x1000;
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

byte get_color(int x, int y, tiledata tile) {
    byte colorbyte = tile.attribute_table;
    // Colors in the PPU are 4 bits. This 4 bit number is then used as an index into the pallette to get the _real_ color.
    // The two most significant bits come from the attribute table byte. Where in this byte they come from depends on which "metatile" in the background they come from. These "metatiles" are 32x32 pixels, or 4x4 tiles.

    // TODO probably don't have to do this on every pixel, only on a new tile.
    bool bottom  = y % 32 < 16;
    bool right = x % 32 < 16;
    
    byte bitmap_bit = 7 - (x % 8); // TODO fine scrolling

    // bottom right, bottom left, top right, top left
    // BRBLTRTL
    colorbyte >>= bottom * 4; // If we're in the bottom quadrants, need to shift over by 4, otherwise we're good.
    colorbyte >>= right * 2; // If we're in the right quadrant, need to shift over by 2, otherwise we're good
    colorbyte = (colorbyte & 0b00000011) << 2; // Make space for the LSB from the tile data

    byte high = (tile.tile_bitmap_high & (0b1 << bitmap_bit)) >> (bitmap_bit - 1); // TODO fine scrolling
    byte low  = (tile.tile_bitmap_low  & (0b1 << bitmap_bit)) >> bitmap_bit;  // TODO fine scrolling
    colorbyte |= high | low;

    return colorbyte;
}

color get_real_color(ppu_memory* ppu_mem, byte colorbyte) {
    byte palette_entry = vram_read(ppu_mem, (uint16_t) colorbyte + 0x3F00);
    if (palette_entry >= 64) {
        errx(EXIT_FAILURE, "Error: palette entry out of range. Maybe time to mod 64 it? or there's something else going on");
    }
    return rgb_palette[palette_entry];
}

void render_pixel(ppu_memory* ppu_mem) {
    int x = ppu_mem->cycle - 1; // Cycle 0 isn't visible.
    int y = ppu_mem->scan_line - 1; // Line 1 isn't visible, either.

    //printf("Rendering pixel at %dx%d\n", x,y);

    byte colorbyte = get_color(x, y, ppu_mem->tile);
    color real_color = get_real_color(ppu_mem, colorbyte);

    ppu_mem->screen[x][y] = real_color;
}

void fetch_step(ppu_memory* ppu_mem) {
    // Each of these fetches takes 2 cycles on a real CPU, and we need to do 4 of them. All 4 will have been completed on the 8th cycle.
    // TODO: do I need to load them in real time or is it ok to grab them all at once every 8 cycles?
    // Just in case I need to do this, to make it easier, they're loaded in the same order they would be in real time, below.
    if (ppu_mem->cycle % 8 == 0) {
        // Nametable byte
        ppu_mem->tile.nametable = vram_read(ppu_mem, get_nametable_address(ppu_mem));
        // Attribute table byte
        ppu_mem->tile.attribute_table = vram_read(ppu_mem, get_attribute_address(ppu_mem));
        dprintf("Read 0x%02X for ATTRIBUTE TABLE\n", ppu_mem->tile.attribute_table);
        // Tile bitmap
        // TODO fine scrolling
        uint16_t tile_bitmap_address = get_background_table_base_address(ppu_mem) + ppu_mem->tile.nametable * 16;
        // Low byte
        ppu_mem->tile.tile_bitmap_low = vram_read(ppu_mem, tile_bitmap_address);
        // High byte
        // The high byte is not stored next to the low byte.
        // The entire tile's 8 low bytes are stored first, then 8 high bytes. So, offset by 8 bytes to get the high byte.
        ppu_mem->tile.tile_bitmap_high = vram_read(ppu_mem, tile_bitmap_address + 8);

        dprintf("Fetched 0x%02X for nametable byte\nFetched 0x%02X for attribute table byte\n", ppu_mem->tile.nametable, ppu_mem->tile.attribute_table);

        // Once done, move to the next tile if we're in a visible line
        if (is_line_visible(ppu_mem)) {
            increment_x(ppu_mem);
        }
    }
}

void ppu_step(ppu_memory* ppu_mem) {
    ppu_mem->cycle++;
    if (ppu_mem->cycle >= CYCLES_PER_LINE) {
        ppu_mem->cycle = 0;
        ppu_mem->scan_line++;
        if (ppu_mem->scan_line >= NUM_LINES) {
            ppu_mem->frame++;
            ppu_mem->scan_line = 0;
            printf("Rendering frame %llu\n", ppu_mem->frame);
            render_screen(ppu_mem->screen);
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
            fetch_step(ppu_mem);
            if (ppu_mem->cycle == 256 && is_line_visible(ppu_mem)) {
                increment_y(ppu_mem);
            }
        }
        else if (is_line_visible(ppu_mem) && ppu_mem->cycle == 257) {
            // Copy X stuff from t to v
            uint16_t masked = ppu_mem->t & 0b0000010000011111;
            ppu_mem->v &= 0b1111101111100000;
            ppu_mem->v |= masked;
        }
        else if (ppu_mem->cycle < 321) {
        }
        else if (ppu_mem->cycle < 337) {
            fetch_step(ppu_mem);
        }
    }

    // Pre render
    if (ppu_mem->scan_line == 261 && ppu_mem->cycle >= 280 && ppu_mem->cycle <= 304) {
            // Copy Y stuff from t to v
            uint16_t masked = ppu_mem->t & 0b111101111100000;
            ppu_mem->v &= 0b000010000011111;
            ppu_mem->v |= masked;
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
            ppu_mem->t &= 0b0111001111111111; // Mask out two bits to copy data into
            ppu_mem->t |= (uint16_t)(value & 0b00000011) << 8;
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
                byte highvalue = value & 0b00111111; // Highest value allowed = 0x3F in high byte
                ppu_mem->t &= 0x00FF;
                ppu_mem->t |= ((uint16_t)highvalue) << 8;
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
                errx(EXIT_FAILURE, "Somehow managed to write an address higher than 0x3FFF to PPUADDR? WTF?");
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
