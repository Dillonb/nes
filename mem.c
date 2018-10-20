#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "util.h"
#include "mem.h"

byte read_cartridge_space_address(rom* r, unsigned short address) {
  // http://wiki.nesdev.com/w/index.php/Mapper

  // NROM
  if (r->mapper == 0) {
    if (address >= 0x8000) { // Can't be more than 0xFFFF
      unsigned short prg_rom_address = (address - 0x8000) % get_prg_rom_bytes(r); // TODO optimize
      byte result = r->prg_rom[prg_rom_address];
      printf("Reading cartridge space address 0x%x from PRG ROM at 0x%x: %02x\n", address, prg_rom_address, result);

      return result;
    }
  }
  errx(EXIT_FAILURE, "attempted to read_cartridge_space_address() at 0x%x, but this is not implemented (yet?)", address);
}

// http://wiki.nesdev.com/w/index.php/CPU_memory_map
byte read_byte(memory* mem, unsigned short address) {
    if (address < 0x2000) {
        return mem->ram[address % 0x800];
    }
    if (address >= 0x4020) { // 0x4020 -> USHRT_MAX is cartridge space
        return read_cartridge_space_address(mem->r, address);
    }

    errx(EXIT_FAILURE, "Access attempted for invalid address: %x", address);
}

void write_byte(memory* mem, unsigned short address, byte value) {
  printf("Tried to write %02x to %04x\n", value, address);
}

memory get_blank_memory(rom* r) {
    // http://wiki.nesdev.com/w/index.php/CPU_power_up_state
    memory mem;

    mem.a = 0x00;
    mem.x = 0x00;
    mem.y = 0x00;
    mem.sp = 0xFD;
    mem.p = 0x34;
    mem.r = r;

    // Read initial value of program counter from the reset vector
    mem.pc = (read_cartridge_space_address(r, 0xFFFD) << 8) | read_cartridge_space_address(r, 0xFFFC);

    printf("Set program counter to 0x%x\n", mem.pc);

    return mem;
}
