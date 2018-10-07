cmake_minimum_required (VERSION 3.12)
project (NES)
add_executable (nes main.c
    cpu.c
    cpu.h

    mem.c
    mem.h

    rom.c
    rom.h

    system.c
    system.h

    lib.c
    lib.h
    )
