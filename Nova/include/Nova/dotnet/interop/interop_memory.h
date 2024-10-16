#pragma once
#include <stdint.h>

void *NvInteropMemoryAlloc(size_t size);
void NvInteropMemoryFree(void *ptr);
