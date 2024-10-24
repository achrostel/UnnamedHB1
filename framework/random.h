#ifndef RANDOM_H
#define RANDOM_H

#include <stdint.h>

void add_entropy(uint32_t e);

uint32_t get_random();

#endif // RANDOM_H