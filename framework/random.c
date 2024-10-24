#include "random.h"

static uint32_t seed = 0x4ba8d93e;

void add_entropy(uint32_t e) {
    seed += e;
}

uint32_t get_random() {
    seed ^= seed >> 7;
    seed ^= seed << 9;
    seed ^= seed >> 13;
    return seed;
}