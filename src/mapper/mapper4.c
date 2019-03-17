#include <err.h>
#include <stdlib.h>
#include <stdint.h>

#include "rom.h"

int prg_offset_for_8kb_bank(rom* r, int bank) {
    if (bank < 0) {
        // Bank is negative, so this'll get the last bank for -1, 2nd to last for -2, etc
        bank = (r->header->prg_rom_blocks * 2) + bank;
    }

    if (bank >= 0x80) {
        bank -= 0x100;
    }
    bank %= (r->header->prg_rom_blocks * 2);
    return bank * (BYTES_PER_PRG_ROM_BLOCK / 2);
}

void mapper4_init(rom* r) {
    // Switchable
    r->mapperdata.prg_bank_0_offset = prg_offset_for_8kb_bank(r, 0);
    r->mapperdata.prg_bank_1_offset = prg_offset_for_8kb_bank(r, 1);
    // Fixed
    r->mapperdata.prg_bank_2_offset = prg_offset_for_8kb_bank(r, -2);
    r->mapperdata.prg_bank_3_offset = prg_offset_for_8kb_bank(r, -1);
}

byte mapper4_prg_read(rom* r, uint16_t address) {
    byte result;

    if (address < 0x6000) {
        errx(EXIT_FAILURE, "MMC3: Invalid PRG read at 0x%04X", address);
    }
    else if (address < 0x8000) {
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        result = r->prg_ram[address];
    }
    else if (address < 0xA000) {
        // 8KB switchable rom bank
        result = r->prg_rom[r->mapperdata.prg_bank_0_offset + (address % 0x2000)];
    }
    else if (address < 0xC000) {
        // 8KB switchable rom bank 2
        result = r->prg_rom[r->mapperdata.prg_bank_1_offset + (address % 0x2000)];
    }
    else if (address < 0xE000) {
        result = r->prg_rom[r->mapperdata.prg_bank_2_offset + (address % 0x2000)];
    }
    else { // <= 0xFFFF
        result = r->prg_rom[r->mapperdata.prg_bank_3_offset + (address % 0x2000)];
    }

    return result;
}

void mapper4_prg_write(rom* r, uint16_t address, byte value) {
    if (address < 0x6000) {
        errx(EXIT_FAILURE, "MMC3: Unhandled PRG write at 0x%04X", address);
    }
    else if (address < 0x8000) { // RAM
        size_t prg_ram_bytes = get_prg_ram_bytes(r);
        address -= 0x6000;
        address %= prg_ram_bytes;
        r->prg_ram[address] = value;
        printf("Wrote 0x%02X to 0x%04X\n", value, address + 0x6000);
    }
    else if (address < 0xA000 && address % 2 == 0) {
        // Bank select
        errx(EXIT_FAILURE, "Unhandled MMC3 write: Bank select 0x%02X\n", value);
    }
    else if (address < 0xA000 && address % 2 == 1) {
        // Bank data
        errx(EXIT_FAILURE, "Unhandled MMC3 write: Bank data 0x%02X\n", value);
    }
    else if (address < 0xC000 && address % 2 == 0) {
        // Mirroring
        errx(EXIT_FAILURE, "Unhandled MMC3 write: Mirroring 0x%02X\n", value);
    }
    else if (address < 0xC000 && address % 2 == 1) {
        // PRG RAM protect
        errx(EXIT_FAILURE, "Unhandled MMC3 write: PRG RAM protect 0x%02X\n", value);
    }
    else if (address < 0xE000 && address % 2 == 0) {
        // IRQ latch
        errx(EXIT_FAILURE, "Unhandled MMC3 write: IRQ latch 0x%02X\n", value);
    }
    else if (address < 0xE000 && address % 2 == 1) {
        // IRQ reload
        errx(EXIT_FAILURE, "Unhandled MMC3 write: IRQ reload 0x%02X\n", value);
    }
    else if (address <= 0xFFFF && address % 2 == 0) {
        // IRQ disable
        errx(EXIT_FAILURE, "Unhandled MMC3 write: IRQ disable 0x%02X\n", value);
    }
    else if (address <= 0xFFFF && address % 2 == 1) {
        // IRQ enable
        errx(EXIT_FAILURE, "Unhandled MMC3 write: IRQ enable 0x%02X\n", value);
    }
    else {
        errx(EXIT_FAILURE, "Tried to write 0x%02X to PRG at 0x%04X\n", value, address);
    }
}

byte mapper4_chr_read(rom* r, uint16_t address) {
    errx(EXIT_FAILURE, "MMC3: Unhandled CHR read: 0x%04X", address);
}

void mapper4_chr_write(rom* r, uint16_t address, byte value) {
    errx(EXIT_FAILURE, "MMC3: Unhandled CHR write: 0x%04X = 0x%02X", address, value);
}
