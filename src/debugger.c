#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

#include "debugger.h"
#include "mem.h"
#include "set.h"
#include "opcode_names.h"

bool debug = false;
int cpu_steps = 0;
address_tree* breakpoints = NULL;

typedef enum debugger_state_value_t {
    RUNNING,
    STOPPED,
    STEPPING
} debugger_state_value;

void set_breakpoint(uint16_t address) {
    if (breakpoints == NULL) {
        breakpoints = new_address_tree();
    }
    insert_to_address_tree(breakpoints, address);
}

void set_breakpoints_for_rom(char* filename) {
    // Read breakpoints if file exists
    char bpfilename[strlen(filename) + 13];
    strncpy(bpfilename, filename, strlen(filename));
    strncat(bpfilename, ".breakpoints", 12);

    FILE* bpfp = fopen(bpfilename, "r");

    if (bpfp != NULL) {
        printf("Breakpoints file %s found, loading\n", bpfilename);
        char buf[10];
        while (fgets(buf, 10, bpfp) != NULL) {
            uint16_t address = strtol(buf, NULL, 16);
            printf("Setting breakpoint at 0x%04X\n", address);
            set_breakpoint(address);
        }
    }
}

bool is_breakpoint(uint16_t address) {
    if (breakpoints == NULL) {
        breakpoints = new_address_tree();
    }
    bool result = address_tree_contains(breakpoints, address);

    return result;
}

bool breakpoint_on_interrupt = false;
void set_breakpoint_on_interrupt() {
    breakpoint_on_interrupt = true;
}

void set_debug() {
    // Super hacky and non-portable way to not wait for enter after getchar().
    system("stty -icanon");
    debug = true;
}

void print_byte_binary(byte value) {
    for (int i = 0; i < 8; i++) {
        printf("%d", value & 1);
        value >>= 1;
    }
}

void dump_byte(byte b) {
    for(int i = 7; i >= 0; i--) {
        printf("%d", (b & mask_flag(i)) > 0);
    }

    printf(" -- 0x%02X\n", b);
}

void print_status(memory* mem) {
    ppu_memory* ppu_mem = &mem->ppu_mem;

    printf("pc  : 0x%04X\n", mem->pc);
    printf("a   : 0x%02X\n", mem->a);
    printf("x   : 0x%02X\n", mem->x);
    printf("y   : 0x%02X\n", mem->y);
    printf("sp  : 0x%02X\n", mem->sp);
    printf("p   : NVBDIZC\n"); 
    printf("    : %d%d%d", get_p_negative(mem), get_p_overflow(mem), get_p_break(mem));
    printf("%d%d%d%d", get_p_decimal(mem), get_p_interrupt(mem), get_p_zero(mem), get_p_carry(mem));
    printf(" -- 0x%02X\n\n", mem->p);
    printf("PPUCTRL  : VPHBSINN\n");
    printf("           ");
    dump_byte(ppu_mem->control);

    printf("PPUSTATUS: VSO.....\n");
    printf("           ");
    dump_byte(ppu_mem->status);


    printf("\nStack:\n");
    for (byte i = mem->sp; i < 0xFF; i++) {
        uint16_t addr = (uint16_t)(i + 1) | 0x100;
        printf("0x%02X: 0x%02X\n", (i + 1), read_byte(mem, addr));

    }
}

void dump_address_device(uint16_t addr) {
    if (addr < 0x2000) {
        printf("RAM");
    }
    else if (addr < 0x4000) {
        printf("PPU");
    }
    else if (addr < 0x4018) {
        printf("APU & I/O");
    }
    else if (addr < 0x4020) {
        printf("APU I/O normally disabled");
    }
    else if (addr <= 0xFFFF) {
        printf("Cartridge ROM");
    }
}

void debugger_read_byte(memory* mem) {
    char buf[10] = {0,0,0,0,0,0,0,0,0,0};
    while (true) {
        uint16_t address;
        printf("\nEnter an address (0x0000 - 0xFFFF). Enter to continue program\n0x");
        fgets(buf, 10, stdin);
        if (buf[0] == '\n') {
            break;
        }
        address = strtol(buf, NULL, 16);

        printf("0x%04X (", address);
        dump_address_device(address);
        printf("): 0x%02X", read_byte(mem, address));
    }
}

debugger_state_value debugger_state = RUNNING;
void process_debugger_command(memory* mem, char command) {
    switch (command) {
        case 's':
            debugger_state = STEPPING;
            return;
        case 'c':
            debugger_state = RUNNING;
            return;
        case 'r':
            debugger_read_byte(mem);
            debugger_state = STEPPING;
            return;
        case 'q':
            errx(EXIT_FAILURE, "User requested quit.");
        default:
            break;
    }
}

void debugger_wait(memory* mem) {
    debugger_state = STOPPED;
    printf("s: step, c: continue, q: quit, r: read byte >");
    while (debugger_state == STOPPED) {
        process_debugger_command(mem, getchar());
    }
    printf("\n");
}

void print_disassembly(memory* mem, uint16_t addr) {
    byte opcode = read_byte(mem, mem->pc);
    printf("%s", opcode_to_name_full(opcode));
    byte size = opcode_sizes[opcode];

    for (int offset = 1; offset < size; offset++) {
        printf(" %02X", read_byte(mem, mem->pc + offset));
    }
}

void debug_hook(debug_hook_type type, memory* mem) {
    if (debug_mode()) {
        print_status(mem);
    }
    if (type == INTERRUPT && breakpoint_on_interrupt) {
        debugger_wait(mem);
    }
    else if (type == STEP) {
        if (debug_mode()) {
            printf("\n\n%05d $%04x: Executing instruction ", cpu_steps++, mem->pc);
            print_disassembly(mem, mem->pc);
            printf("\n");
        }
        if (is_breakpoint(mem->pc) || debugger_state == STEPPING || debugger_state == STOPPED) {
            debugger_wait(mem);
        }
    }
}

bool debug_mode() {
    return debug;
}
