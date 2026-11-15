#pragma once
#include <stddef.h>
#include <stdint.h>

class Mem {
public:
	Mem();
	static void* Memset(void* dest, int value, size_t count); // Set
	static void* Memcpy(void* dest, const void* src, size_t count); // Copy
	static int Memcmp(const void* a, const void* b, size_t count); // Compare
	static void* Memmove(void* dest, const void* src, size_t count); // Move
};