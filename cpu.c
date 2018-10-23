#include <stdlib.h>
#include <err.h>

#include "cpu.h"
#include "mem.h"

#include "util.h"

byte read_byte_and_inc_pc(memory* mem) {
  byte data = read_byte(mem, mem->pc);
  mem->pc++;
  return data;
}

uint16_t read_address_and_inc_pc(memory* mem) {
  byte lower = read_byte_and_inc_pc(mem);
  byte upper = read_byte_and_inc_pc(mem);
  return (upper << 8) | lower;
}

void cpu_step(memory* mem) {
  byte opcode = read_byte_and_inc_pc(mem);

  int cycles = 0;

  switch (opcode) {
    case BRK: // BRK
      // Set interrupt flag on status register
      mem->p |= 0b0000100;
      mem->pc++; // Skip next byte
      cycles = 7;
      break;

    case SEI: // SEI
      mem->p |= 0b0000100;
      cycles = 2;
      break;

    case STA_Absolute: // STA Absolute
      write_byte(mem, read_address_and_inc_pc(mem), mem->a);
      cycles = 4;
      break;

    case TXS: // TXS
      mem->sp = mem->x;
      cycles = 2;
      break;

    case LDX_Immediate: // LDX Immediate
      mem->x = read_byte_and_inc_pc(mem);
      cycles = 2;
      break;

    case LDA_Immediate: // LDA Immediate
      mem->a = read_byte_and_inc_pc(mem);
      cycles = 2;
      break;

    case LDA_Absolute: // LDA absolute
      mem->a = read_byte(mem, read_address_and_inc_pc(mem));
      cycles = 4;
      break;

    case CLD: // CLD
      // Clear decimal mode flag on status register
      mem->p &= 0b1111101;
      cycles = 2;
      break;


    default:
      errx(EXIT_FAILURE, "Invalid opcode: 0x%x\n", opcode);
  }

  //printf("Step took %d cycles\n", cycles);
}
