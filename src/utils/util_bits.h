#ifndef EMULATOR_UTIL_BITS_H
#define EMULATOR_UTIL_BITS_H

#include <stdint.h>

int test_bit(const uint8_t* array, uint32_t bit);
void xor_bit(uint8_t* array, uint32_t bit, int second_operand);

#endif //EMULATOR_UTIL_BITS_H
