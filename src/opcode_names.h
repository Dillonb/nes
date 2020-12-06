#ifndef _OPCODE_NAMES_H
#define _OPCODE_NAMES_H

#include "util.h"
#include "cpu.h"

#define ADC_Immediate  0x69
#define ADC_Zeropage   0x65
#define ADC_Zeropage_X 0x75
#define ADC_Absolute   0x6D
#define ADC_Absolute_X 0x7D
#define ADC_Absolute_Y 0x79
#define ADC_Indirect_X 0x61
#define ADC_Indirect_Y 0x71

#define AND_Immediate  0x29
#define AND_Zeropage   0x25
#define AND_Zeropage_X 0x35
#define AND_Absolute   0x2D
#define AND_Absolute_X 0x3D
#define AND_Absolute_Y 0x39
#define AND_Indirect_X 0x21
#define AND_Indirect_Y 0x31

#define ASL_Accumulator 0x0A
#define ASL_Zeropage    0x06
#define ASL_Zeropage_X  0x16
#define ASL_Absolute    0x0E
#define ASL_Absolute_X  0x1E

#define BCC 0x90
#define BCS 0xB0
#define BEQ 0xF0

#define BIT_Zeropage 0x24
#define BIT_Absolute 0x2C

#define BMI 0x30
#define BNE 0xD0
#define BPL 0x10

#define BRK 0x0

#define BVC 0x50
#define BVS 0x70

#define CLC 0x18
#define CLD 0xD8
#define CLI 0x58
#define CLV 0xB8

#define CMP_Immediate  0xC9
#define CMP_Zeropage   0xC5
#define CMP_Zeropage_X 0xD5
#define CMP_Absolute   0xCD
#define CMP_Absolute_X 0xDD
#define CMP_Absolute_Y 0xD9
#define CMP_Indirect_X 0xC1
#define CMP_Indirect_Y 0xD1

#define CPX_Immediate 0xE0
#define CPX_Zeropage  0xE4
#define CPX_Absolute  0xEC

#define CPY_Immediate 0xC0
#define CPY_Zeropage  0xC4
#define CPY_Absolute  0xCC

#define DEC_Zeropage   0xC6
#define DEC_Zeropage_X 0xD6
#define DEC_Absolute   0xCE
#define DEC_Absolute_X 0xDE

#define DEX 0xCA
#define DEY 0x88

#define EOR_Immediate  0x49
#define EOR_Zeropage   0x45
#define EOR_Zeropage_X 0x55
#define EOR_Absolute   0x4D
#define EOR_Absolute_X 0x5D
#define EOR_Absolute_Y 0x59
#define EOR_Indirect_X 0x41
#define EOR_Indirect_Y 0x51

#define INC_Zeropage   0xE6
#define INC_Zeropage_X 0xF6
#define INC_Absolute   0xEE
#define INC_Absolute_X 0xFE

#define INX 0xE8
#define INY 0xC8

#define JMP_Absolute 0x4C
#define JMP_Indirect 0x6C

#define JSR 0x20

#define LDA_Immediate  0xA9
#define LDA_Zeropage   0xA5
#define LDA_Zeropage_X 0xB5
#define LDA_Absolute   0xAD
#define LDA_Absolute_X 0xBD
#define LDA_Absolute_Y 0xB9
#define LDA_Indirect_X 0xA1
#define LDA_Indirect_Y 0xB1

#define LDX_Immediate  0xA2
#define LDX_Zeropage   0xA6
#define LDX_Zeropage_Y 0xB6
#define LDX_Absolute   0xAE
#define LDX_Absolute_Y 0xBE

#define LDY_Immediate  0xA0
#define LDY_Zeropage   0xA4
#define LDY_Zeropage_X 0xB4
#define LDY_Absolute   0xAC
#define LDY_Absolute_X 0xBC

#define LSR_Accumulator 0x4A
#define LSR_Zeropage    0x46
#define LSR_Zeropage_X  0x56
#define LSR_Absolute    0x4E
#define LSR_Absolute_X  0x5E

#define NOP 0xEA

#define ORA_Immediate  0x09
#define ORA_Zeropage   0x05
#define ORA_Zeropage_X 0x15
#define ORA_Absolute   0x0D
#define ORA_Absolute_X 0x1D
#define ORA_Absolute_Y 0x19
#define ORA_Indirect_X 0x01
#define ORA_Indirect_Y 0x11

#define PHA 0x48
#define PHP 0x08

#define PLA 0x68
#define PLP 0x28

#define ROL_Accumulator 0x2A
#define ROL_Zeropage    0x26
#define ROL_Zeropage_X  0x36
#define ROL_Absolute    0x2E
#define ROL_Absolute_X  0x3E

#define ROR_Accumulator 0x6A
#define ROR_Zeropage    0x66
#define ROR_Zeropage_X  0x76
#define ROR_Absolute    0x6E
#define ROR_Absolute_X  0x7E

#define RTI 0x40
#define RTS 0x60

#define SBC_Immediate  0xE9
#define SBC_Zeropage   0xE5
#define SBC_Zeropage_X 0xF5
#define SBC_Absolute   0xED
#define SBC_Absolute_X 0xFD
#define SBC_Absolute_Y 0xF9
#define SBC_Indirect_X 0xE1
#define SBC_Indirect_Y 0xF1

#define SEC 0x38
#define SED 0xF8
#define SEI 0x78

#define STA_Zeropage   0x85
#define STA_Zeropage_X 0x95
#define STA_Absolute   0x8D
#define STA_Absolute_X 0x9D
#define STA_Absolute_Y 0x99
#define STA_Indirect_X 0x81
#define STA_Indirect_Y 0x91


#define STX_Zeropage   0x86
#define STX_Zeropage_Y 0x96
#define STX_Absolute   0x8E

#define STY_Zeropage   0x84
#define STY_Zeropage_X 0x94
#define STY_Absolute   0x8C

#define TAX 0xAA
#define TAY 0xA8

#define TSX 0xBA
#define TXA 0x8A
#define TXS 0x9A
#define TYA 0x98

extern byte opcode_sizes[256];
extern byte opcode_cycles[256];
extern addressing_mode opcode_addressing_modes[256];

const char* opcode_to_name_full(byte opcode);
const char* opcode_to_name_short(byte opcode);

#endif