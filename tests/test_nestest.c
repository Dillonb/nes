#include <stdlib.h>
#include "unity.h"
#include <src/cpu.h>
#include <src/opcode_names.h>
#include <src/mem.h>
#include <src/system.h>

#include <stdint.h>
#include <src/debugger.h>

typedef struct step_t {
    uint16_t address;
    byte a;
    byte x;
    byte y;
    byte p;
    byte sp;
    int ppu_x;
    int ppu_y;
    int cycles;
} log_step;

rom* r;
memory* mem;

void test_load_rom(void) {
    if (r != NULL) {
        free(r);
    }

    r = read_rom("nestest.nes");
    mem = get_blank_memory(r);
    mem->pc = 0xC000; // nestest: automated tests start at a different address
    mem->p  = 0x24; // nestest: the irrelevant bits are set differently in the log.
}

int num_steps = 8991;
log_step steps[8991];
void test_load_steps(void) {
    FILE* fp = fopen("nestest.log", "rb");

    for (int step = 0; step < num_steps; step++) {
        log_step temp_step;

        char buf[10];

        TEST_ASSERT_NOT_NULL(fgets(buf, 5, fp));

        temp_step.address = (uint16_t) strtol(buf, NULL, 16);

        // Skip the raw byte values & disassembly
        fseek(fp, 46, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 3, fp));
        temp_step.a = (byte)strtol(buf, NULL, 16);

        // Skip label
        fseek(fp, 3, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 3, fp));
        temp_step.x = (byte)strtol(buf, NULL, 16);

        // Skip label
        fseek(fp, 3, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 3, fp));
        temp_step.y = (byte)strtol(buf, NULL, 16);

        // Skip label
        fseek(fp, 3, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 3, fp));
        temp_step.p = (byte)strtol(buf, NULL, 16);

        // Skip label
        fseek(fp, 4, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 3, fp));
        temp_step.sp = (byte)strtol(buf, NULL, 16);

        // Skip label
        fseek(fp, 5, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 4, fp));
        temp_step.ppu_x = (int)strtol(buf, NULL, 10);

        // Skip label
        fseek(fp, 1, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 4, fp));
        temp_step.ppu_y = (int)strtol(buf, NULL, 10);

        // Skip label
        fseek(fp, 5, SEEK_CUR);

        TEST_ASSERT_NOT_NULL(fgets(buf, 9, fp));
        temp_step.cycles = (int)strtol(buf, NULL, 10);

        steps[step] = temp_step;
    }
}

void print_byte(byte b) {
    for(int i = 7; i >= 0; i--) {
        printf("%d", (b & mask_flag(i)) > 0);
    }
}

int test_total_cycles = 7; // nestest.log starts at 7

int get_ppu_x(memory* mem) {
    int x = get_screen_x(&mem->ppu_mem) + 1;
    return x;
}

int get_ppu_y(memory* mem) {
    return get_screen_y(&mem->ppu_mem);
}

void print_step_info(int index, log_step stepdata) {
    bool success = true;
    if (stepdata.address != mem->pc) {
        printf("FAIL: We should be at address 0x%04X - we are at address 0x%04X\n", stepdata.address, mem->pc);
        success = false;
    }
    if (stepdata.a != mem->a) {
        printf("FAIL: Accumulator should be 0x%02X - but is 0x%02X\n", stepdata.a, mem->a);
        success = false;
    }
    if (stepdata.x != mem->x) {
        printf("FAIL: X should be 0x%02X - but is 0x%02X\n", stepdata.x, mem->x);
        success = false;
    }
    if (stepdata.y != mem->y) {
        printf("FAIL: Y should be 0x%02X - but is 0x%02X\n", stepdata.y, mem->y);
        success = false;
    }
    if (stepdata.cycles != test_total_cycles) {
        printf("FAIL: Cycles should be %d - but is %d\n", stepdata.cycles, test_total_cycles);
        success = false;
    }
    if (stepdata.p != mem->p) {
        printf("FAIL: P should be 0x%02X - but is 0x%02X\n", stepdata.p, mem->p);
        printf("In binary: NV-BDIZC\n");
        printf("Expected:  ");
        print_byte(stepdata.p);
        printf("\nActual:    ");
        print_byte(mem->p);
        printf("\n");
        success = false;
    }
    int ppu_x = get_ppu_x(mem);
    int ppu_y = get_ppu_y(mem);
    if (stepdata.ppu_x != ppu_x) {
        printf("FAIL: ppu_x should be %d but is %d\n", stepdata.ppu_x, ppu_x);
    }
    if (stepdata.ppu_y != ppu_y) {
        printf("FAIL: ppu_y should be %d but is %d\n", stepdata.ppu_y, ppu_y);
    }
    if (success) {
        char *disassembly = disassemble(mem, mem->pc);
        printf("%04d $%04X OPC: 0x%02X | %-20s a: 0x%02X x: 0x%02X y: 0x%02X p: 0x%02X sp: 0x%02X ppu: %03d,%03d cycles: %d\n", index, mem->pc, read_byte(mem, mem->pc), disassembly, mem->a, mem->x,
               mem->y, mem->p, mem->sp, ppu_x, ppu_y, test_total_cycles);
        free(disassembly);
    }
}

void test_run_rom(void) {
    for (int step = 0; step < num_steps; step++) {
        log_step stepdata = steps[step];
        print_step_info(step, stepdata);
        TEST_ASSERT_EQUAL_UINT16(stepdata.address, mem->pc);
        TEST_ASSERT_EQUAL_UINT8(stepdata.a, mem->a);
        TEST_ASSERT_EQUAL_UINT8(stepdata.x, mem->x);
        TEST_ASSERT_EQUAL_UINT8(stepdata.y, mem->y);
        TEST_ASSERT_EQUAL_UINT8(stepdata.p, mem->p);
        TEST_ASSERT_EQUAL_UINT8(stepdata.sp, mem->sp);
        TEST_ASSERT_EQUAL_INT(stepdata.cycles, test_total_cycles);
        int ppu_x = get_ppu_x(mem);
        TEST_ASSERT_EQUAL_INT(stepdata.ppu_x, ppu_x);
        int ppu_y = get_ppu_y(mem);
        TEST_ASSERT_EQUAL_INT(stepdata.ppu_y, ppu_y);

        int cycles = system_step(mem);
        printf("Took %d cycles\n", cycles);
        test_total_cycles += cycles;
    }
}



int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_load_rom);
    RUN_TEST(test_load_steps);
    RUN_TEST(test_run_rom);
    return UNITY_END();
}
