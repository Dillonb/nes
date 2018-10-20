cmake_minimum_required (VERSION 3.10)
project (NES)
set(CMAKE_BUILD_TYPE Debug)
add_executable (nes main.c
    cpu.c
    cpu.h

    mem.c
    mem.h

    rom.c
    rom.h

    lib.c
    lib.h
    )
