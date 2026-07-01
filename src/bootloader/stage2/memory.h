#pragma once
#include "stdint.h"

// Copies n bytes from memory area source to memory area destination
void far* memcpy(void far* dst, const void far* src, uint16_t num);

// fills  the first n bytes of the memory area pointed to by s with the constant byte c.
void far* memset(void far* ptr, int value, uint16_t num);

// Compares the first n bytes (each interpreted as unsigned char) of thememory areas s1 and s2.
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);
