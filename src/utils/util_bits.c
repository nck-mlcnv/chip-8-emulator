#include "util_bits.h"

// Tests the requested bit of the uint8_t array.
int test_bit(const uint8_t* array, uint32_t bit)
{
    uint32_t index = bit / 8;
    uint32_t carry = bit - (index * 8);
    switch (carry)
    {
        case 0:
            return 0 != (array[index] & 0b10000000);
        case 1:
            return 0 != (array[index] & 0b01000000);
        case 2:
            return 0 != (array[index] & 0b00100000);
        case 3:
            return 0 != (array[index] & 0b00010000);
        case 4:
            return 0 != (array[index] & 0b00001000);
        case 5:
            return 0 != (array[index] & 0b00000100);
        case 6:
            return 0 != (array[index] & 0b00000010);
        case 7:
            return 0 != (array[index] & 0b00000001);
        default:
            return 0;
    }
}

// XORs the requested bit inside the array with the given bit literal as second_operand.
void xor_bit(uint8_t* array, uint32_t bit, int second_operand)
{
    if (second_operand == 0)
        return;
    uint32_t index = bit / 8;
    uint32_t carry = bit - (index * 8);
    uint8_t mask = 1 << (7 - carry);
    array[index] ^= mask;
}
