#include <portaudio.h>
#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include "apu.h"

byte read_apu_register(apu_memory* apu_mem, byte register_num) {

}

void write_apu_register(apu_memory* apu_mem, int register_num, byte value) {
    if (register_num < 0x4) {
        // Pulse 1
    } else if (register_num < 0x8) {
        // Pulse 2
    }
    else if (register_num < 0xC) {
        // Triangle
    }
    else if (register_num < 0x10) {
        // Noise
    }
    else if (register_num < 0x14) {
        // DMC
    }
    else if (register_num == 0x15) {
        // Channel enable and length counter status
    }
    else if (register_num == 0x17) {
        // Frame counter
    }
    else {
        printf("WARNING: Unhandled APU register write to 0x%04X\n", (register_num + 0x4000));
    }
}

apu_memory get_apu_mem() {
    apu_memory apu_mem;
    apu_mem.cycle = 0;

    return apu_mem;
}

void apu_step(apu_memory* apu_mem) {
    double last_cycle = apu_mem->cycle++;
    double this_cycle = apu_mem->cycle;

    if ((int)(last_cycle / STEPS_PER_SAMPLE) != (int)(this_cycle / STEPS_PER_SAMPLE)) {
        // TODO Generate a sample, place in ring buffer
    }

}

// TODO: Put PortAudio stuff into its own file
static int paCallback(const void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      const PaStreamCallbackTimeInfo *timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void *userData) {
    apu_memory* apu_mem = (apu_memory*)userData;
    float* out = (float*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */

    for(uint32_t i = 0; i < framesPerBuffer; i++) {
        // Read sample from ring buffer. While the ring buffer is empty, push zeroes.
        *out++ = 0;
    }
    return 0;
}

PaStream* stream;

void apu_init(apu_memory* apu_mem) {
    // TODO: Put PortAudio stuff into its own file
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        errx(EXIT_FAILURE, "Unable to initialize PortAudio: %s", Pa_GetErrorText(err));
    }

    err = Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, AUDIO_SAMPLE_RATE, 256, paCallback, apu_mem);


    if (err != paNoError) {
        errx(EXIT_FAILURE, "Unable to open PortAudio stream: %s", Pa_GetErrorText(err));
    }

    err = Pa_StartStream(stream);

    if (err != paNoError) {
        errx(EXIT_FAILURE, "Unable to start PortAudio stream: %s", Pa_GetErrorText(err));
    }

}