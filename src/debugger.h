#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "mem.h"

typedef enum debug_hook_type_t {
    STEP,
    INTERRUPT
} debug_hook_type;

bool debug_mode();
void debug_hook(debug_hook_type type, memory* mem);
void set_debug();
void set_breakpoint(uint16_t address);
void set_breakpoints_for_rom(char* filename);
void set_breakpoint_on_interrupt();
void debugger_wait();
