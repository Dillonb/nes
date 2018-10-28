#include "unity/unity.h"
#include <src/mem.h>

memory mock_memory() {
  memory mem;

  mem.a = 0x00;
  mem.x = 0x00;
  mem.y = 0x00;
  mem.sp = 0xFD;
  mem.p = 0x34;
  mem.pc = 0x0000; // For tests, start reading at 0x0000 so we don't need to load a real ROM

  return mem;
}

void test_pflags() {
  memory mem = mock_memory();
  mem.p = 0x00;
  set_p_negative(&mem);
  TEST_ASSERT_EQUAL_UINT8(0b10000000, mem.p);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_pflags);
  return UNITY_END();
}
