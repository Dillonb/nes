#pragma once

#include <stdbool.h>
#include "mem.h"

typedef struct movie_t {
    FILE* handle;
    memory* mem;
} movie;

typedef struct button_states_t {
    bool states[2][8];
} button_states;

movie read_movie(char* filename, memory* mem);
button_states get_next_inputs(movie* m);

