#pragma once
#include <stdint.h>

struct CPUContext {
	uint64_t RSP;
	uint64_t RBX;
	uint64_t RBP;
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;
};