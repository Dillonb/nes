# nes

[![Build Status](https://travis-ci.org/Dillonb/nes.svg?branch=master)](https://travis-ci.org/Dillonb/nes)

A toy NES emulator. A little buggy, but it mostly works. Tested on Linux and MacOS.

## Mappers supported

0, 1, 2, 4, 7, 31

## Building

* Install cmake, PortAudio, and SDL2 (>=2.0.5)
* Run the following commands:

```bash
cd build
cmake ..
make
```

## Running

    ./nes <rom.nes>

To run in debug, do:

    ./nes <rom.nes> debug

To create breakpoints, place a rom.nes.breakpoints file next to rom.nes. Each line of this file should contain a memory address to break on.

## Controls

* WASD/Arrow Keys: D-Pad
* J/Z: A
* K/X: B
* Right shift: Select
* Enter: Start
