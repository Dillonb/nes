#include <portaudio.h>
#include <err.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "apu.h"

#include <SDL.h>

const char* gradient[] = {
        "\x1b[38;2;255;255;255",
        "\x1b[38;2;0;255;0m",
        "\x1b[38;2;36;255;0m",
        "\x1b[38;2;73;255;0m",
        "\x1b[38;2;109;255;0m",
        "\x1b[38;2;146;255;0m",
        "\x1b[38;2;182;255;0m",
        "\x1b[38;2;219;255;0m",
        "\x1b[38;2;255;255;0m", // midpoint
        "\x1b[38;2;255;219;0m",
        "\x1b[38;2;255;182;0m",
        "\x1b[38;2;255;146;0m",
        "\x1b[38;2;255;109;0m",
        "\x1b[38;2;255;73;0m",
        "\x1b[38;2;255;36;0m",
        "\x1b[38;2;255;0;0m"
};

void print_color_for_volume(byte volume) {
    printf("%s", gradient[volume & 0b1111]);
}

void print_color_terminator() {
    printf("\x1b[0m");
}

bool apu_tracker_enabled = false;
void set_apu_tracker_enabled(bool enabled) {
    apu_tracker_enabled = enabled;
}

void dump_apu(apu_memory* apu_mem) {
    byte pulse1volume = apu_mem->pulse1.constant_volume ? apu_mem->pulse1.vol_and_env_period : apu_mem->pulse1.envelope_volume;
    printf("▏ ");
    if (apu_mem->pulse1.length_counter > 0 && apu_mem->pulse1.timer_register >= 8 && apu_mem->pulse1.enable && pulse1volume > 0) {
        print_color_for_volume(pulse1volume);
        printf("%04X", apu_mem->pulse1.timer_register);
        print_color_terminator();
    }
    else {
        printf("    ");
    }

    printf(" ▏ ");

    byte pulse2volume = apu_mem->pulse2.constant_volume ? apu_mem->pulse2.vol_and_env_period : apu_mem->pulse2.envelope_volume;
    if (apu_mem->pulse2.length_counter > 0 && apu_mem->pulse2.timer_register >= 8 && apu_mem->pulse2.enable && pulse2volume > 0) {
        print_color_for_volume(pulse2volume);
        printf("%04X", apu_mem->pulse2.timer_register);
        print_color_terminator();
    }
    else {
        printf("    ");
    }

    printf(" ▏ ");

    if (apu_mem->triangle.enable && apu_mem->triangle.length_counter > 0 && apu_mem->triangle.linear_counter > 0) {
        print_color_for_volume(0b1111);
        printf("%04X", apu_mem->triangle.timer_register);
        print_color_terminator();
    }
    else {
        printf("    ");
    }

    printf(" ▏ ");

    byte noisevolume = apu_mem->noise.cv_or_env ? apu_mem->noise.vol_and_env_period : apu_mem->noise.envelope_volume;
    if (apu_mem->noise.enable && apu_mem->noise.length_counter > 0 && noisevolume > 0) {
        print_color_for_volume(noisevolume);
        printf("%04X", apu_mem->noise.timer_register);
        print_color_terminator();
    }
    else {
        printf("    ");
    }

    printf(" ▏ ");

    if (apu_mem->dmc.enable && apu_mem->dmc.sample_length > 0) {
        printf("%02X", apu_mem->dmc.rate);
    }
    else {
        printf("  ");
    }

    printf(" ▏ ");

    printf("\n");
}

byte read_apu_status(apu_memory *apu_mem) {
    byte dmc_interrupt = 0;
    byte frame_interrupt = 0;
    byte dmc = apu_mem->dmc.sample_length > 0 ? (byte)1 : (byte)0;
    byte noise = apu_mem->noise.length_counter > 0 ? (byte)1 : (byte)0;
    byte triangle = apu_mem->triangle.length_counter > 0 ? (byte)1 : (byte)0;
    byte pulse1 = apu_mem->pulse1.length_counter > 0 ? (byte)1 : (byte)0;
    byte pulse2 = apu_mem->pulse2.length_counter > 0 ? (byte)1 : (byte)0;

    return (dmc_interrupt << 7) |
            (frame_interrupt << 6) |
            (dmc << 4) |
            (noise << 3) |
            (triangle << 2) |
            (pulse2 << 1) |
            (pulse1);

}

byte pulse_duty[4][8] = {
        {0, 1, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 0, 0, 0, 0, 0},
        {0, 1, 1, 1, 1, 0, 0, 0},
        {1, 0, 0, 1, 1, 1, 1, 1}
};

byte triangle_duty[] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5,  4,  3,  2,  1,  0,
        0,  1,  2,  3,  4,  5,  6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

byte length_counter_table[] = {
        10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26,
        14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16,
        28, 32, 30 };

uint16_t noise_timer_table[] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };

byte dmc_rate_table[] = {
        214, 190, 170, 160, 143, 127, 113, 107, 95, 80, 71, 64, 53, 42, 36, 27 };

void write_pulse_register(pulse_oscillator *pulse, int register_num, byte value) {
    switch (register_num) {
        case 0: {
            pulse->duty_value          = (byte)((value & 0b11000000) >> 6);
            pulse->length_counter_halt = ((value & 0b00100000) >> 5) == 1;
            pulse->constant_volume     = ((value & 0b00010000) >> 4) == 1;
            pulse->vol_and_env_period  = value & (byte)0b1111;
            pulse->envelope_start      = true;
            break;
        }
        case 1: {
            pulse->sweep_enabled     = (value & 0b10000000) >> 7 == 1;
            pulse->sweep_period      = (byte)(value & 0b01110000) >> 4;
            pulse->sweep_negate      = (byte)(value & 0b00001000) >> 3 == 1;
            pulse->sweep_shift_count = (byte)(value & 0b00000111);
            pulse->sweep_reload      = true;
            break;
        }
        case 2: {
            pulse->timer_register &= 0b11100000000;
            pulse->timer_register |= value;
            break;
        }

        case 3: {
            pulse->timer_register &= 0b00011111111;
            pulse->timer_register |= (value & 0b00000111) << 8;
            pulse->length_counter = length_counter_table[(value & 0b11111000) >> 3];
            pulse->envelope_start = true;
            break;
        }
    }
}

void write_triangle_register(triangle_oscillator* triangle, int register_num, byte value) {
    switch (register_num) {
        case 0: { // 0x4008
            triangle->length_counter_halt = (value & 0b10000000) == 0b10000000;
            triangle->linear_counter_load = (byte) (value & 0b01111111);
            triangle->linear_counter_reload = true;
            break;
        }
        case 1: // 0x4009
            // Unused
            break;
        case 2: { // 0x400A
            triangle->timer_register &= 0b11100000000;
            triangle->timer_register |= value;
            break;
        }
        case 3: { // 0x400B
            triangle->timer_register &= 0b00011111111;
            triangle->timer_register |= ((uint16_t)value & 0b111) << 8;
            triangle->length_counter = length_counter_table[value >> 3];
            triangle->linear_counter_reload = true;
            break;
        }
        default:
            break;
    }
}

void write_noise_register(noise_oscillator* noise, int register_num, byte value) {
    switch (register_num) {
        case 0: { // 0x400C
            noise->length_counter_halt = (value & 0b00100000) == 0b00100000;
            noise->cv_or_env           = (value & 0b00010000) == 0b00010000;
            noise->vol_and_env_period  = (byte)(value & 0b00001111);
            break;
        }
        case 1: // 0x400D
            // Unused
            break;
        case 2: { // 0x400E
            noise->mode           = (value & 0b10000000) == 0b10000000;
            noise->timer_register = noise_timer_table[value & 0b00001111];
            break;
        }
        case 3: { // 0x400F
            noise->length_counter = length_counter_table[value >> 3];
            break;
        }
        default:
            break;
    }
}

void write_dmc_register(dmc_oscillator* dmc, int register_num, byte value) {
    switch (register_num) {
        case 0: { // 0x4010
            dmc->irq_enabled = (value & 0b10000000) == 0b10000000;
            dmc->loop        = (value & 0b01000000) == 0b01000000;
            dmc->rate        = dmc_rate_table[value & 0b00001111];
            break;
        }
        case 1: { // 0x4011
            dmc->level = value & (byte)0b01111111;
            break;
        }
        case 2: { // 0x4012
            dmc->sample_address_register = (uint16_t)0xC000 | ((uint16_t)value << 6);
            break;
        }
        case 3: { // 0x4013
            dmc->sample_length_register = ((uint16_t)value << 4) | (uint16_t)1;
            break;
        }
        default:
            break;
    }
}

apu_memory get_apu_mem() {
    apu_memory apu_mem;
    apu_mem.cycle = 0;
    apu_mem.buffer_read_index = 0;
    apu_mem.buffer_write_index = 0;

    apu_mem.pulse1.timer_register = 0;
    apu_mem.pulse2.timer_register = 0;

    apu_mem.pulse1.enable = 0;
    apu_mem.pulse2.enable = 0;

    apu_mem.interrupt_inhibit = false;
    apu_mem.frame_counter_mode = 0;

    apu_mem.noise.enable = 0;
    apu_mem.noise.lfsr = 1; // for pseudorandom

    apu_mem.dmc.enable = false;
    apu_mem.dmc.level = 0;
    apu_mem.dmc.sample_length = 0;
    apu_mem.dmc.sample_address = 0;
    apu_mem.dmc.sample_bit = 0;

    return apu_mem;
}

float get_volume_scale(byte volume) {
    return volume == 0 ? 0 : (float)volume / 15.0f;
}

float get_dmc_sample(dmc_oscillator* dmc) {
    float level = dmc->level;
    return (level - 63.5f) / 63.5f;
}

float get_noise_sample(noise_oscillator* noise) {
    int bit = noise->lfsr & 1;
    if (!noise->enable || noise->length_counter == 0 || bit == 0 || noise->timer_register < 1) {
        return 0.0f;
    }
    else {
        float volume_scale;
        if (noise->cv_or_env) { // 1 is constant volume
            volume_scale = get_volume_scale(noise->vol_and_env_period);
        }
        else {
            volume_scale = get_volume_scale(noise->envelope_volume);
        }
        return 1.0f * volume_scale;
    }
}

float get_triangle_sample(triangle_oscillator* triangle) {
    if (triangle->enable && triangle->length_counter > 0 && triangle->linear_counter > 0) {
        float duty = triangle_duty[triangle->duty_step];
        return (duty - 7.5f) / 7.5f;
    }
    else {
        return 0.0f;
    }
}

float get_pulse_sample(pulse_oscillator* pulse) {
    if (pulse->timer_register < 8 || pulse->timer_register > 0x7FF || pulse->enable == false || pulse->length_counter == 0) {
        return 0.0f;
    }

    float volume_scale;
    if (pulse->constant_volume) {
        volume_scale = get_volume_scale(pulse->vol_and_env_period);
    }
    else {
        volume_scale = get_volume_scale(pulse->envelope_volume);
    }

    if (pulse_duty[pulse->duty_value][pulse->duty_step] == 1) {
        return 1.0f * volume_scale;
    }
    else {
        return -1.0f * volume_scale;
    }
}

void step_pulse_timer(pulse_oscillator* pulse) {
    if (pulse->timer_step == 0) {
        pulse->timer_step = pulse->timer_register;
        pulse->duty_step = (byte)((pulse->duty_step + 1) % 8);
    }
    else {
        pulse->timer_step--;
    }
}

void pulse_dec_length_counter(pulse_oscillator* pulse) {
    if (pulse->length_counter > 0 && !pulse->length_counter_halt) {
        pulse->length_counter--;
    }
}

void triangle_dec_length_counter(triangle_oscillator* triangle) {
    if (triangle->length_counter > 0 && !triangle->length_counter_halt) {
        triangle->length_counter--;
    }
}

void noise_dec_length_counter(noise_oscillator* noise) {
    if (noise->length_counter > 0 && !noise->length_counter_halt) {
        noise->length_counter--;
    }
}

void dec_length_counter(apu_memory* apu_mem) {
    pulse_dec_length_counter(&apu_mem->pulse1);
    pulse_dec_length_counter(&apu_mem->pulse2);
    triangle_dec_length_counter(&apu_mem->triangle);
    noise_dec_length_counter(&apu_mem->noise);
}

void pulse_clock_envelope(pulse_oscillator* pulse) {
    if (pulse->envelope_start) {
        pulse->envelope_start  = false;
        pulse->envelope_period_value  = pulse->vol_and_env_period;
        pulse->envelope_volume = 15;
    }
    else if (pulse->envelope_period_value > 0) {
        pulse->envelope_period_value--;
    }
    else {
        // At the end of the envelope period, decrement the volume by one.
        if (pulse->envelope_volume > 0) {
            pulse->envelope_volume--;
        }
        else if (!pulse->length_counter_halt) { // Same bit is used for this, but inverted
            pulse->envelope_volume = 15;
        }

        pulse->envelope_period_value = pulse->vol_and_env_period;
    }
}

void noise_clock_envelope(noise_oscillator* noise) {
    if (noise->envelope_start) {
        noise->envelope_start  = false;
        noise->envelope_period_value  = noise->vol_and_env_period;
        noise->envelope_volume = 15;
    }
    else if (noise->envelope_period_value > 0) {
        noise->envelope_period_value--;
    }
    else {
        // At the end of the envelope period, decrement the volume by one.
        if (noise->envelope_volume > 0) {
            noise->envelope_volume--;
        }
        else if (!noise->length_counter_halt) { // Same bit is used for this, but inverted
            noise->envelope_volume = 15;
        }

        noise->envelope_period_value = noise->vol_and_env_period;
    }
}

void clock_envelope(apu_memory* apu_mem) {
    pulse_clock_envelope(&apu_mem->pulse1);
    pulse_clock_envelope(&apu_mem->pulse2);
    noise_clock_envelope(&apu_mem->noise);
}

void pulse_clock_sweep(pulse_oscillator* pulse, int pulsenum) {
    if (pulse->sweep_enabled && pulse->sweep_counter == 0) {
        pulse->sweep_counter = pulse->sweep_period;
        uint16_t diff = pulse->timer_register >> pulse->sweep_shift_count;
        if (pulse->sweep_negate) {
            pulse->timer_register = pulse->timer_register
                    - diff
                    // Pulse channel 1 subtracts 1 more here
                    - (pulsenum == 1);
        }
        else {
            pulse->timer_register += diff;
        }
    }

    if (pulse->sweep_reload || pulse->sweep_counter == 0) {
        pulse->sweep_reload = false;
        pulse->sweep_counter = pulse->sweep_period;
    }
    else {
        if (pulse->sweep_counter > 0) {
            pulse->sweep_counter--;
        }
    }
}

void clock_sweep(apu_memory* apu_mem) {
    pulse_clock_sweep(&apu_mem->pulse1, 1);
    pulse_clock_sweep(&apu_mem->pulse2, 2);
}

void clock_triangle_linear_counter(triangle_oscillator* triangle) {
    if (triangle->linear_counter_reload) {
        triangle->linear_counter = triangle->linear_counter_load;
    }
    else if (triangle->linear_counter > 0) {
        triangle->linear_counter--;
    }

    if (triangle->length_counter_halt == 0) { // This flag is reused here.
        triangle->linear_counter_reload = false;
    }
}


void step_frame_counter(apu_memory *apu_mem) {
    switch (apu_mem->frame_counter_mode) {
        case FC_4STEP:
            apu_mem->frame_counter = (byte)((apu_mem->frame_counter + 1) % 4);
            switch (apu_mem->frame_counter) {
                case 0:
                case 2:
                    clock_envelope(apu_mem);
                    clock_triangle_linear_counter(&apu_mem->triangle);
                    break;
                case 1:
                    clock_envelope(apu_mem);
                    clock_triangle_linear_counter(&apu_mem->triangle);
                    clock_sweep(apu_mem);
                    break;
                case 3:
                    dec_length_counter(apu_mem);
                    clock_sweep(apu_mem);
                    break;
            }
            break;
        case FC_5STEP:
            apu_mem->frame_counter = (byte)((apu_mem->frame_counter + 1) % 5);
            switch (apu_mem->frame_counter) {
                case 0:
                case 2:
                    clock_envelope(apu_mem);
                    clock_triangle_linear_counter(&apu_mem->triangle);
                    break;
                case 1:
                case 3:
                    clock_envelope(apu_mem);
                    clock_triangle_linear_counter(&apu_mem->triangle);
                    dec_length_counter(apu_mem);
                    clock_sweep(apu_mem);
                    break;
            }
            break;
    }
}

void step_triangle_timer(triangle_oscillator* triangle) {
    if (triangle->timer_step == 0) {
        triangle->timer_step = triangle->timer_register;
        if (triangle->length_counter > 0 && triangle->linear_counter > 0) {
            triangle->duty_step = (byte)((triangle->duty_step + 1) % 32);
        }
    }
    else {
        triangle->timer_step--;
    }
}

void step_noise_timer(noise_oscillator* noise) {
    if (noise->timer_step == 0) {
        noise->timer_step = noise->timer_register;
        if (noise->length_counter > 0) {
            // Get the feedback bit by XORing bit 0 with either bit 6 or bit 1, depending on the mode.
            uint16_t feedback = (noise->lfsr & (uint16_t)1) ^ ((noise->lfsr >> (noise->mode ? 6 : 1)) & (uint16_t)1);
            noise->lfsr >>= 1;
            noise->lfsr |= (feedback << 14);
        }
    }
    else {
        noise->timer_step--;
    }
}

void step_dmc_timer(dmc_oscillator* dmc) {
    if (!dmc->enable || dmc->sample_bit == 0) {
        return;
    }
    if (dmc->tick == 0) {
        dmc->tick = dmc->rate;
        if ((dmc->output_buffer & 1) == 1) {
            if (dmc->level < 126) {
                dmc->level += 2;
            }
        }
        else {
            if (dmc->level > 1) {
                dmc->level -= 2;
            }
        }
        dmc->sample_bit--;
        dmc->output_buffer >>= 1;
    }
    else {
        dmc->tick--;
    }
}

void apu_step(apu_memory* apu_mem) {
    double last_cycle = apu_mem->cycle++;
    double this_cycle = apu_mem->cycle;

    if (apu_mem->buffer_write_index - apu_mem->buffer_read_index < 1) {
        //printf("Audio buffer underrun detected!\n");
    }

    if (apu_mem->cycle % 2 == 0) { // APU clock is half as fast as CPU
        step_pulse_timer(&apu_mem->pulse1);
        step_pulse_timer(&apu_mem->pulse2);
        step_noise_timer(&apu_mem->noise);
        step_dmc_timer(&apu_mem->dmc);
    }
    // Triangle clock is as fast as the CPU
    step_triangle_timer(&apu_mem->triangle);

    if ((int)(last_cycle / APU_STEPS_PER_FRAME_COUNTER_STEP) != (int)(this_cycle / APU_STEPS_PER_FRAME_COUNTER_STEP)) {
        step_frame_counter(apu_mem);
    }

    if ((int)(last_cycle / APU_STEPS_PER_SAMPLE) != (int)(this_cycle / APU_STEPS_PER_SAMPLE)) {
        // TODO other oscs, and mix them
        float pulse1_sample   = get_pulse_sample(&apu_mem->pulse1);
        float pulse2_sample   = get_pulse_sample(&apu_mem->pulse2);
        float triangle_sample = get_triangle_sample(&apu_mem->triangle);
        float noise_sample    = get_noise_sample(&apu_mem->noise);
        float dmc_sample      = get_dmc_sample(&apu_mem->dmc);
        //printf("DMC sample: %f\n", dmc_sample);

        float sample = (0.2f * pulse1_sample) +
                (0.2f * pulse2_sample) +
                (0.2f * triangle_sample) +
                (0.2f * noise_sample) +
                (0.5f * dmc_sample);
        sample *= 0.10f;
        apu_mem->buffer[(apu_mem->buffer_write_index++) % APU_RING_BUFFER_SIZE] = sample;
    }

}

void sdlCallback(void * userdata, Uint8 * stream, int length)
{
    apu_memory* apu_mem = (apu_memory*)userdata;
    float* out = (float*)stream;

    for(uint32_t i = 0; i < length / sizeof(float); i++) {
        // Read sample from ring buffer. While the ring buffer is empty, push zeroes.
        if (apu_mem->buffer_read_index >= apu_mem->buffer_write_index) {
            *out++ = 0;
        }
        else {
            *out++ = apu_mem->buffer[(apu_mem->buffer_read_index++) % APU_RING_BUFFER_SIZE];
        }
    }
}

SDL_AudioSpec sdlAudioSpec;
SDL_AudioDeviceID sdlAudioDev;

void apu_init(apu_memory* apu_mem) {

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        errx(EXIT_FAILURE, "SDL couldn't initialize! %s", SDL_GetError());
    }

    SDL_AudioSpec request;
    memset(&request, 0, sizeof(request));

    request.freq = AUDIO_SAMPLE_RATE;
    request.format = AUDIO_F32;
    request.channels = 1;
    request.samples = 32;
    request.callback = sdlCallback;
    request.userdata = apu_mem;
    sdlAudioDev = SDL_OpenAudioDevice(NULL, 0, &request, &sdlAudioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

    if (sdlAudioDev == 0) {
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    }

    SDL_PauseAudioDevice(sdlAudioDev, false);
}

void write_apu_register(apu_memory* apu_mem, int register_num, byte value) {
    if (apu_tracker_enabled) {
        dump_apu(apu_mem);
    }

    if (register_num < 0x4) {
        // Pulse 1
        write_pulse_register(&apu_mem->pulse1, register_num % 4, value);
    } else if (register_num < 0x8) {
        // Pulse 2
        write_pulse_register(&apu_mem->pulse2, register_num % 4, value);
    }
    else if (register_num < 0xC) {
        write_triangle_register(&apu_mem->triangle, register_num % 4, value);
    }
    else if (register_num < 0x10) {
        write_noise_register(&apu_mem->noise, register_num % 4, value);
    }
    else if (register_num < 0x14) {
        write_dmc_register(&apu_mem->dmc, register_num % 4, value);
    }
    else if (register_num == 0x15) {
        // Channel enable and length counter status
        apu_mem->dmc.enable = ((value >> 4) & 1) == 1;
        if (apu_mem->dmc.enable) {
            if (apu_mem->dmc.sample_length == 0) {
                apu_mem->dmc.sample_length = apu_mem->dmc.sample_length_register;
                apu_mem->dmc.sample_address = apu_mem->dmc.sample_address_register;
            }
        }
        else {
            apu_mem->dmc.sample_length = 0;
        }
        apu_mem->noise.enable = ((value >> 3) & 1) == 1;
        apu_mem->triangle.enable = ((value >> 2) & 1) == 1;
        apu_mem->pulse2.enable = ((value >> 1) & 1) == 1;
        if (!apu_mem->pulse2.enable) {
            apu_mem->pulse2.length_counter = 0;
        }

        apu_mem->pulse1.enable = (value & 1) == 1;
        if (!apu_mem->pulse1.enable) {
            apu_mem->pulse1.length_counter = 0;
        }
    }
    else if (register_num == 0x17) {
        // Frame counter
        apu_mem->frame_counter_mode = (value >> 7) & 1;
        apu_mem->interrupt_inhibit = ((value >> 6) & 1) == 1;

        if (apu_mem->frame_counter_mode == FC_5STEP) {
            clock_envelope(apu_mem);
            clock_triangle_linear_counter(&apu_mem->triangle);
            clock_sweep(apu_mem);
            dec_length_counter(apu_mem);
        }

    }
    else {
        printf("WARNING: Unhandled APU register write to 0x%04X\n", (register_num + 0x4000));
    }
}
