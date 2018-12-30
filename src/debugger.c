#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "debugger.h"
#include "mem.h"
#include "set.h"
#include "opcode_names.h"

bool debug = false;
int cpu_steps = 0;
address_tree* breakpoints = NULL;

void set_breakpoint(uint16_t address) {
    if (breakpoints == NULL) {
        breakpoints = new_address_tree();
    }
    insert_to_address_tree(breakpoints, address);
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

typedef enum debugger_state_value_t {
    RUNNING,
    STOPPED,
    STEPPING
} debugger_state_value;

debugger_state_value debugger_state = RUNNING;
void process_debugger_command(char command) {
    switch (command) {
        case 's':
            debugger_state = STEPPING;
            return;
        case 'c':
            debugger_state = RUNNING;
            return;
        case 'q':
            errx(EXIT_FAILURE, "User requested quit.");
        default:
            break;
    }
}

void debugger_wait() {
    debugger_state = STOPPED;
    printf("s: step, c: continue, q: quit  >");
    while (debugger_state == STOPPED) {
        process_debugger_command(getchar());
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
    if (type == INTERRUPT && breakpoint_on_interrupt) {
        debugger_wait();
    }
    else if (type == STEP) {
        if (debug_mode()) {
            printf("\n\n%05d $%04x: Executing instruction ", cpu_steps++, mem->pc);
            print_disassembly(mem, mem->pc);
            printf("\n");
        }
        if (is_breakpoint(mem->pc) || debugger_state == STEPPING || debugger_state == STOPPED) {
            debugger_wait();
        }
    }
}

bool debug_mode() {
    return debug;
}
