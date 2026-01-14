#ifndef KERNEL_CPUID_H
#define KERNEL_CPUID_H

#include <stdint.h>

void cpuid(uint32_t leaf, uint32_t subleaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);

#endif