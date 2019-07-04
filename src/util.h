#pragma once
#include <stdio.h>

#define CPU_FREQUENCY_DEF 1789773

const double CPU_FREQUENCY = 1789773;

typedef unsigned char byte;

void wait_interactive();
byte mask_flag(int index);
