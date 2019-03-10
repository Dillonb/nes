#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <err.h>

#include "ppu.h"
#include "cpu.h"
#include "debugger.h"
#include "render.h"
#include "palette.h"
#include "mapper/mapper.h"

#define VBLANK_LINE 241
#define MAX_SPRITES_PER_LINE 8

ppu_memory get_ppu_mem(rom* r) {
    ppu_memory ppu_mem;

    ppu_mem.r = r;

    ppu_mem.control     = 0b00000000;
    ppu_mem.mask        = 0b00000000;
    ppu_mem.status      = 0b10100000;
    ppu_mem.oam_address = 0b00000000;
    ppu_mem.v           = 0b0000000000000000;
    ppu_mem.t           = 0b0000000000000000;
    ppu_mem.w           = HIGH;
    ppu_mem.data        = 0b00000000;

    ppu_mem.tile.attribute_table  = 0;
    ppu_mem.tile.nametable        = 0;
    ppu_mem.tile.tile_bitmap_high = 0;
    ppu_mem.tile.tile_bitmap_low  = 0;

    ppu_mem.frame = 0;
    ppu_mem.scan_line = 0;
    ppu_mem.cycle = 0;

    ppu_mem.num_sprites = 0;

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
        }
        ppu_mem->v = (uint16_t) ((ppu_mem->v & ~0x03E0) | (y << 5));
    }
}
uint16_t get_coarse_x(ppu_memory* ppu_mem) {
    return ppu_mem->v & (uint16_t)0b11111;
}

uint16_t get_coarse_y(ppu_memory* ppu_mem) {
    return (ppu_mem->v >> 5) & (uint16_t)0b11111;
}

byte get_fine_x(ppu_memory* ppu_mem) {
    return ppu_mem->x;
}

byte get_fine_y(ppu_memory* ppu_mem) {
    return (byte) (ppu_mem->v >> 12 & (uint16_t)0b111);
}


uint16_t get_nametable_address(ppu_memory* ppu_mem) {
    return (uint16_t) (0x2000 | (ppu_mem->v & 0x0FFF));
}

uint16_t get_attribute_address(ppu_memory* ppu_mem) {
    return (uint16_t) (0x23C0 | (ppu_mem->v & 0x0C00) | ((ppu_mem->v >> 4) & 0x38) | ((ppu_mem->v >> 2) & 0x07));
}

uint16_t mirror_nametable_address(uint16_t addr, ppu_memory* ppu_mem) {
    uint16_t nametable_addr = addr - (uint16_t)0x2000;
    nametable_mirroring mirror_mode = ppu_mem->r->nametable_mirroring_mode;

    if (mirror_mode == HORIZONTAL) {
        if (nametable_addr < 0x400) {
            // Nothing!
        }
        else if (nametable_addr < 0x800) {
            // 2nd nametable is a mirror of the first
            nametable_addr -= 0x400;
        }
        else if (nametable_addr < 0xC00) {
            // third nametable is the 2nd in memory
            nametable_addr -= 0x400;
        }
        else {
            // fourth nametable is a mirror of the third
            nametable_addr %= 0x800;
        }
    }
    else if (mirror_mode == VERTICAL) {
        if (nametable_addr >= 0x800) {
            // First and second nametables are 0x000-0x800
            // Third and fourth are mirrors of first and second, respectively
            nametable_addr %= 0x800;
        }
    }
    else if (mirror_mode == SINGLE_LOWER) {
        nametable_addr %= 0x400;
    }
    else if (mirror_mode == SINGLE_UPPER) {
        nametable_addr %= 0x400;
        nametable_addr += 0x400;
    }
    else {
        errx(EXIT_FAILURE, "Need to implement some kind of mirroring! Rev up those debuggers!");
    }

    if (nametable_addr >= 0x800) {
        errx(EXIT_FAILURE, "Got a value >= 0x800 after mirroring! Rev up those debuggers!");
    }

    return nametable_addr;
}

uint16_t mirror_palette_address(uint16_t address) {
    address %= 32;

    if (address >= 0x10 && address % 4 == 0) {
        return address - (uint16_t)0x10;
    }

    return address;
}

void vram_write(ppu_memory* ppu_mem, uint16_t address, byte value) {
    address %= 0x4000;
    dprintf("Writing 0x%02X to PPU VRAM address 0x%04X\n", value, address);

    // Pattern tables
    if (address < 0x2000) {
        mapper_chr_write(ppu_mem, address, value);
    }
    // Nametables
    else if (address < 0x3F00) {
        uint16_t index = mirror_nametable_address(address, ppu_mem);
        ppu_mem->name_tables[index] = value;
    }
    // Palette RAM indexes
    else if (address < 0x4000) {
        ppu_mem->palette_ram[mirror_palette_address(address)] = value;
    }
    else {
        errx(EXIT_FAILURE, "Attempted to write 0x%02X to UNKNOWN PPU VRAM ADDRESS 0x%04X", value, address);
    }
}

byte vram_read(ppu_memory* ppu_mem, uint16_t address) {
    address %= 0x4000;
    byte result;

    // Pattern tables
    if (address < 0x2000) {
        result = mapper_chr_read(ppu_mem, address);
    }
    // Nametables
    else if (address < 0x3F00) {
        uint16_t index = mirror_nametable_address(address, ppu_mem);
        result = ppu_mem->name_tables[index];
    }
    // Palette RAM indexes
    else if (address < 0x4000) {
        result = ppu_mem->palette_ram[mirror_palette_address(address)];
    }
    else {
        errx(EXIT_FAILURE, "Read from UNKNOWN PPU VRAM ADDRESS 0x%04X", address);
    }

    dprintf("Read 0x%02X from PPU VRAM address 0x%04X\n", result, address);

    return result;
}

bool get_control_flag(ppu_memory* mem, int index) {
    return (mem->control & mask_flag(index)) != 0;
}

bool vblank_nmi(ppu_memory* mem) {
    return get_control_flag(mem, 7);
}

bool get_sprite_size_flag(ppu_memory* ppu_mem) {
    return get_control_flag(ppu_mem, 5);
}

bool get_sprite_pattern_table_flag(ppu_memory* ppu_mem) {
    return get_control_flag(ppu_mem, 3);
}

bool get_addr_increment_flag(ppu_memory* mem) {
    return get_control_flag(mem, 2);
}

int get_addr_increment(ppu_memory* mem) {
    return get_addr_increment_flag(mem) ? 32 : 1;
}

int get_sprite_height(ppu_memory* ppu_mem) {
    return ((int)get_sprite_size_flag(ppu_mem) + 1) * 8;
}

uint16_t get_background_table_base_address(ppu_memory* ppu_mem) {
    return get_control_flag(ppu_mem, 4) * (uint16_t)0x1000;
}

uint16_t get_sprite_pattern_table_address(ppu_memory* ppu_mem) {
    return get_sprite_pattern_table_flag(ppu_mem) * (uint16_t)0x1000;
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

bool sprites_enabled(ppu_memory* ppu_mem) {
    return (ppu_mem->mask & 0b00010000) > 0;
}

bool background_enabled(ppu_memory* ppu_mem) {
    return (ppu_mem->mask & 0b00001000) > 0;
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

void set_sprite_overflow(ppu_memory* ppu_mem) {
    ppu_mem->status |= 0b00100000; // Set sprite overflow flag on PPUSTATUS
}

void clear_sprite_overflow(ppu_memory* ppu_mem) {
    ppu_mem->status &= 0b11011111; // Clear sprite overflow flag on PPUSTATUS
}

void set_sprite_zero_hit(ppu_memory* ppu_mem) {
    ppu_mem->status |= 0b01000000; // Set sprite zero hit flag on PPUSTATUS
}

void clear_sprite_zero_hit(ppu_memory* ppu_mem) {
    ppu_mem->status &= 0b10111111; // Clear sprite zero hit flag on PPUSTATUS
}

byte get_color(byte fine_x, tiledata tile) {
    int bitmap_bit = 15 - fine_x;
    // crumb is half a nibble
    int at_crumb = 30 - (fine_x * 2);
    byte at_entry = (byte)((tile.attribute_table >> at_crumb) & 0b11) << 2;

    byte pt_entry = (byte)((tile.tile_bitmap_high >> bitmap_bit) & (byte)1) << 1
                  | (byte)((tile.tile_bitmap_low  >> bitmap_bit) & (byte)1);

    if (pt_entry == 0) {
        return pt_entry;
    }
    else {
        return at_entry | pt_entry;
    }
}

color get_real_color(ppu_memory* ppu_mem, byte colorbyte) {
    uint16_t addr = (uint16_t)(colorbyte + 0x3F00);
    byte palette_entry = vram_read(ppu_mem, addr);
    palette_entry %= 64;
    return rgb_palette[palette_entry];
}

int get_screen_x(ppu_memory* ppu_mem) {
    return ppu_mem->cycle - 1; // Cycle 0 isn't visible.
}

int get_screen_y(ppu_memory* ppu_mem) {
    return ppu_mem->scan_line;
}

void render_pixel(ppu_memory* ppu_mem) {
    int x = get_screen_x(ppu_mem);
    int y = get_screen_y(ppu_mem);

    if (x < 0 || y < 0) {
        errx(EXIT_FAILURE, "Attempted to render invalid x,y %d,%d", x, y);
    }

    // Background
    byte background_color = 0;
    color real_background_color;

    if (background_enabled(ppu_mem)) {
        background_color = get_color(get_fine_x(ppu_mem), ppu_mem->tile);
        real_background_color = get_real_color(ppu_mem, background_color);
    }

    byte sprite_color;
    color real_sprite_color;
    bool found_sprite = false;
    int found_sprite_index = -1;
    // Sprites
    if (sprites_enabled(ppu_mem)) {
        for (int i = 0; i < ppu_mem->num_sprites && !found_sprite; i++) {
            sprite s = ppu_mem->sprites[i];
            int offset = x - s.x_coord;
            // Does the sprite overlap the pixel we're currently in?
            if (offset >= 0 && offset < 8) {
                byte color = (s.pattern.palette & (byte) 0b11) << 2;
                int shift = s.pattern.reverse ? offset : 7 - offset;
                color |= ((s.pattern.high_byte >> shift) & 1) << 1;
                color |= (s.pattern.low_byte >> shift) & 1;

                // Is the pixel of the sprite we want to render non-transparent?
                if (color % 4 != 0) {
                    dprintf("Color: %02X\n", color);
                    found_sprite = true;
                    sprite_color = color | (byte) 0x10;
                    found_sprite_index = i;
                    real_sprite_color = get_real_color(ppu_mem, sprite_color);
                }
            }
        }
    }

    if (found_sprite) {
        dprintf("RENDERING SPRITE PIXEL! %02X%02X%02X%02X\n", real_sprite_color.r, real_sprite_color.g, real_sprite_color.b, real_sprite_color.a);
        if (found_sprite_index == 0 && background_color != 0 && x != 255) {
            set_sprite_zero_hit(ppu_mem);
        }
        if (background_color != 0 && ppu_mem->sprites[found_sprite_index].priority == 1) {
            ppu_mem->screen[x][y] = real_background_color;
        }
        else {
            ppu_mem->screen[x][y] = real_sprite_color;
        }
    }
    else {
        ppu_mem->screen[x][y] = real_background_color;
    }

    dprintf("Pixel %d,%d is 0x%02X0x%02X0x%02X\n", x, y, real_background_color.r, real_background_color.g, real_background_color.b);
}

void fetch_step(ppu_memory* ppu_mem) {
    // Each of these fetches takes 2 cycles on a real CPU, and we need to do 4 of them. All 4 will have been completed on the 8th cycle.
    ppu_mem->tile.tile_bitmap_low <<= 1;
    ppu_mem->tile.tile_bitmap_high <<= 1;
    ppu_mem->tile.attribute_table <<= 2;
    if (ppu_mem->cycle % 8 == 1) {
        dprintf("Fetching at screen pos %d,%d, VRAM pos %d~%d,%d~%d\n", get_screen_x(ppu_mem), get_screen_y(ppu_mem),
                get_coarse_x(ppu_mem), get_fine_x(ppu_mem), get_coarse_y(ppu_mem), get_fine_y(ppu_mem));
        // Nametable byte
        uint16_t nametable_addr = get_nametable_address(ppu_mem);
        ppu_mem->tile.nametable = vram_read(ppu_mem, nametable_addr);
    }

    if (ppu_mem->cycle % 8 == 3) {
        // Attribute table byte
        uint32_t at = vram_read(ppu_mem, get_attribute_address(ppu_mem));
        // Colors in the PPU are 4 bits. This 4 bit number is then used as an index into the palette to get the _real_ color.
        // The two most significant bits come from the attribute table byte. Where in this byte they come from depends on
        // which "metatile" in the background they come from. These "metatiles" are 32x32 pixels, or 4x4 tiles.
        // This code grabs the correct 2 bits from the attribute table value for the current tile, and repeats it to fill
        // an entire tile's worth of shifts.
        at >>= ((ppu_mem->v >> (byte)4) & (byte)4) | (ppu_mem->v & (byte)2);
        at &= 0b11;
        at |= (at << 2)
              | (at << 4)
              | (at << 6)
              | (at << 8)
              | (at << 10)
              | (at << 12)
              | (at << 14);

        ppu_mem->temp_attribute_table = at;
    }

    if (ppu_mem->cycle % 8 == 5) {
        // Tile bitmap low byte
        uint16_t nametable = ppu_mem->tile.nametable & (byte)0xFF;
        uint16_t tile_bitmap_address = get_background_table_base_address(ppu_mem) + nametable * (uint16_t)16 + get_fine_y(ppu_mem);
        ppu_mem->temp_bitmap_low = vram_read(ppu_mem, tile_bitmap_address);
    }

    if (ppu_mem->cycle % 8 == 7) {
        // Tile bitmap high byte
        // The high byte is not stored next to the low byte.
        // The entire tile's 8 low bytes are stored first, then 8 high bytes. So, offset by 8 bytes to get the high byte.
        uint16_t nametable = ppu_mem->tile.nametable & (byte)0xFF;
        uint16_t tile_bitmap_address = get_background_table_base_address(ppu_mem) + nametable * (uint16_t)16 + get_fine_y(ppu_mem);
        ppu_mem->temp_bitmap_high = vram_read(ppu_mem, tile_bitmap_address + (uint16_t)8);
    }

    dprintf("Nametable byte 0x%02X\nAttribute table 0x%04X\n", ppu_mem->tile.nametable, ppu_mem->tile.attribute_table);

    if (ppu_mem->cycle % 8 == 0) {
        ppu_mem->tile.tile_bitmap_low |= ppu_mem->temp_bitmap_low;
        ppu_mem->tile.tile_bitmap_high |= ppu_mem->temp_bitmap_high;
        ppu_mem->tile.attribute_table |= ppu_mem->temp_attribute_table;
        // Once done, move to the next tile
        increment_x(ppu_mem);
    }
}

void x_t_to_v(ppu_memory* ppu_mem) {
    // Copy X stuff from t to v
    uint16_t masked = ppu_mem->t & (uint16_t)0b0000010000011111;
    ppu_mem->v &= 0b1111101111100000;
    ppu_mem->v |= masked;
}

void y_t_to_v(ppu_memory* ppu_mem) {
    // Copy Y stuff from t to v
    uint16_t masked = ppu_mem->t & (uint16_t)0b111101111100000;
    ppu_mem->v &= 0b000010000011111;
    ppu_mem->v |= masked;
}

sprite_pattern get_sprite_pattern(ppu_memory* ppu_mem, byte tile, byte attr, int offset) {
    bool sprites_8x16 = get_sprite_size_flag(ppu_mem);
    bool flip_vertically   = (attr & 0b10000000) > 0;
    bool flip_horizontally = (attr & 0b01000000) > 0;

    uint16_t addr;

    if (sprites_8x16) {
        if (flip_vertically) {
            offset = 15 - offset;
        }
        // Use bit 0 of the OAM table's tile value for 8x16 sprites
        addr = (tile & (byte)0b00000001) * (uint16_t)0x1000;
        dprintf("Using pattern table 0x%04X for sprite\n", addr);
        tile &= 0b11111110; // and mask out that bit
        // If we need to, skip to the next byte (8x16 sprites take up two bytes, obviously)
        if (offset > 7) {
            tile++;
            offset -= 8;
        }
    }
    else {
        if (flip_vertically) {
            offset = 7 - offset;
        }
        // Use the value from PPUCTRL for 8x8 sprites
        addr = get_sprite_pattern_table_address(ppu_mem);
    }

    addr += (uint16_t)((tile * 16) + offset);

    sprite_pattern sp;

    sp.palette = attr & (byte)0b00000011;
    sp.low_byte = vram_read(ppu_mem, addr);
    sp.high_byte = vram_read(ppu_mem, addr + (uint16_t)8); // Same as background, high byte is offset by 8 bytes
    sp.reverse = flip_horizontally;

    return sp;
}

void evaluate_sprites(ppu_memory* ppu_mem) {
    int sprite_height = get_sprite_height(ppu_mem);
    byte num_sprites_found = 0;
    for (byte i = 0; i < 64; i++) {
        byte y_coord = ppu_mem->oam_data[i * 4];
        byte tile    = ppu_mem->oam_data[i * 4 + 1];
        byte attr    = ppu_mem->oam_data[i * 4 + 2];
        byte x_coord = ppu_mem->oam_data[i * 4 + 3];

        // How many pixels offset from the current scan line the sprite is
        int16_t offset = (int16_t)ppu_mem->scan_line - (int16_t)y_coord;
        if (offset >= 0 && offset < sprite_height) {
            sprite s;
            s.pattern = get_sprite_pattern(ppu_mem, tile, attr, offset);
            s.x_coord = x_coord;
            s.priority = (attr & 0b00100000) > 0;
            s.index = i;

            if (num_sprites_found < MAX_SPRITES_PER_LINE) {
                ppu_mem->sprites[num_sprites_found] = s;
            }

            num_sprites_found++;
        }
    }

    if (num_sprites_found > MAX_SPRITES_PER_LINE) {
        num_sprites_found = MAX_SPRITES_PER_LINE;
        set_sprite_overflow(ppu_mem);
    }

    ppu_mem->num_sprites = num_sprites_found;
}

void ppu_step(ppu_memory* ppu_mem) {
    ppu_mem->cycle++;
    if (ppu_mem->cycle >= CYCLES_PER_LINE) {
        ppu_mem->cycle = 0;
        ppu_mem->scan_line++;
        if (ppu_mem->scan_line >= NUM_LINES) {
            ppu_mem->frame++;
            ppu_mem->scan_line = 0;
            dprintf("Rendering frame %llu\n", ppu_mem->frame);
            render_screen(ppu_mem->screen);
        }
    }

    // Visible
    if (ppu_mem->scan_line < 240) {
        if (rendering_enabled(ppu_mem)) {
            if (is_visible(ppu_mem)) {
                render_pixel(ppu_mem);
            }

            if ((ppu_mem->cycle > 0 && ppu_mem->cycle < 257) || (ppu_mem->cycle > 320 && ppu_mem->cycle <= 336)) {
                fetch_step(ppu_mem);
                if (ppu_mem->cycle == 256) {
                    increment_y(ppu_mem);
                }
            }
            else if (ppu_mem->cycle == 257) {
                x_t_to_v(ppu_mem);
                // This isn't how it works on the actual NES. On the real nes, this sprite evaluation process is
                // happening at the same time as the rendering up to this point, to a temporary buffer. It's swapped
                // over at the very end. Since modern computers are fast, we can just do it all at once at the end.
                // Hopefully this won't cause problems. Do games swap out sprite data during a scan line?
                evaluate_sprites(ppu_mem);
            }
        }
    }
    // VBlank
    else if (ppu_mem->scan_line < 261) {
        if (ppu_mem->scan_line == VBLANK_LINE && ppu_mem->cycle == 1) {
            set_vblank(ppu_mem);
        }
    }
    // Pre-render
    else if (ppu_mem->scan_line == 261) {
        if (rendering_enabled(ppu_mem)) {
            if ((ppu_mem->cycle> 0 && ppu_mem->cycle < 257) || (ppu_mem->cycle > 320 && ppu_mem->cycle <= 336)) {
                fetch_step(ppu_mem);
                if (ppu_mem->cycle == 256) {
                    increment_y(ppu_mem);
                }
            }
            else if (ppu_mem->cycle >= 280 && ppu_mem->cycle <= 304) {
                y_t_to_v(ppu_mem);
            }
            else if (ppu_mem->cycle == 257) {
                x_t_to_v(ppu_mem);
            }
        }
    }
    else {
        errx(EXIT_FAILURE, "Somehow ended up on invalid scanline %d. WTF?", ppu_mem->scan_line);
    }

    // Reset stuff for the end of the line
    if (ppu_mem->cycle == 1 && ppu_mem->scan_line == 0) {
        clear_vblank(ppu_mem);
        clear_sprite_overflow(ppu_mem);
        clear_sprite_zero_hit(ppu_mem);
    }
}

byte read_status_sideeffects(ppu_memory* ppu_mem) {
    dprintf("WARNING: returning status register with sideeffects\n");
    byte oldval = ppu_mem->status;
    clear_vblank(ppu_mem);
    ppu_mem->w = HIGH;
    return oldval;
}

byte ppu_open_bus;

byte read_ppu_register(ppu_memory* ppu_mem, byte register_num) {
    byte result;
    switch (register_num) {
        case 2: {
            // Update last 5 bits of status register from open bus
            byte last5 = ppu_open_bus & (byte)0b00011111;
            ppu_mem->status = (ppu_mem->status & (byte)0b11100000) | last5;
            result = read_status_sideeffects(ppu_mem);
            break;
        }
        case 4:
            result = ppu_mem->oam_data[ppu_mem->oam_address];
            break;
        case 7: {
            byte value = vram_read(ppu_mem, ppu_mem->v);
            if (ppu_mem->v < 0x3F00) {
                byte return_value = ppu_mem->fake_buffer;
                ppu_mem->fake_buffer = value;
                value = return_value;
            }
            else {
                ppu_mem->fake_buffer = vram_read(ppu_mem, ppu_mem->v - (uint16_t)0x1000);
            }
            ppu_mem->v += get_addr_increment(ppu_mem);

            result = value;
            break;
        }
        default:
            printf("WARNING: reading from invalid PPU register %x - only 2, 4, and 7 are capable of being read from\n", register_num);
            return ppu_open_bus;
    }

    ppu_open_bus = result;

    return result;
}

void write_oam_byte(ppu_memory* ppu_mem, byte value) {
    ppu_mem->oam_data[ppu_mem->oam_address++] = value;
}

void write_ppu_register(ppu_memory* ppu_mem, byte register_num, byte value) {
    switch (register_num) {
        case 0:
            ppu_mem->control = value;
            ppu_mem->t &= 0b0111001111111111; // Mask out two bits to copy data into
            ppu_mem->t |= (uint16_t)(value & 0b00000011) << 10;
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
        case 5: {
            if (ppu_mem->w == HIGH) {
                ppu_mem->t &= 0b1111111111100000; // Mask out last 5 bits (coarse X)
                ppu_mem->t |= (value >> 3); // First 5 bits of the written value to go coarse X in t
                ppu_mem->x = value & (byte)0b111; // Last 3 bits go to fine X scrolling register
                ppu_mem->w = LOW;
            }
            else if (ppu_mem->w == LOW) {
                uint16_t fine_y = value & (byte)0b111;
                uint16_t coarse_y = (value & (byte)0b11111000) >> 3;

                ppu_mem->t &= 0b000110000011111; // Mask out fine and coarse Y
                ppu_mem->t |= fine_y << 12; // Insert fine y
                ppu_mem->t |= coarse_y << 5; // Insert coarse y
                ppu_mem->w = HIGH;
            }
            return;
        }
        case 6: {
            if (ppu_mem->w == HIGH) {
                // Mask out old high byte
                uint16_t highvalue = value & (byte)0b00111111; // Highest value allowed = 0x3F in high byte
                ppu_mem->t &= 0x00FF;
                ppu_mem->t |= highvalue << 8;
                ppu_mem->w = LOW;
            }
            else if (ppu_mem->w == LOW) {
                // Mask out old low byte
                ppu_mem->t &= 0x3F00;
                ppu_mem->t |= value;
                ppu_mem->v = ppu_mem->t;
                ppu_mem->w = HIGH;
            }
            if (ppu_mem->t > 0x3FFF) {
                errx(EXIT_FAILURE, "Somehow managed to write an address higher than 0x3FFF (0x%04x) to PPUADDR? WTF?", ppu_mem->t);
            }
            return;
        }
        case 7:
            vram_write(ppu_mem, ppu_mem->v, value);
            ppu_mem->v += get_addr_increment(ppu_mem);
            return;
        default:
            printf("WARNING: writing 0x%02X to read-only PPU register %x\n", value, register_num);
            ppu_open_bus = value;

    }
}
