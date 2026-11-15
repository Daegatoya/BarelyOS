#include "mem.h"

Mem::Mem() {

}

void* Mem::Memset(void* dest, int value, size_t count) {
	unsigned char* convDest = static_cast<unsigned char*>(dest);
	unsigned char convVal = static_cast<unsigned char>(value);

	for (size_t i = 0; i < count; i++) {
		*(convDest + i) = convVal;
	}

	return dest;
}

void* Mem::Memcpy(void* dest, const void* src, size_t count) {
	unsigned char* convDest = static_cast<unsigned char*>(dest);
	const unsigned char* convSrc = static_cast<const unsigned char*>(src);

	for (size_t i = 0; i < count; i++) {
		*(convDest + i) = *(convSrc + i);
	}

	return dest;
}

int Mem::Memcmp(const void* a, const void* b, size_t count) {
	const unsigned char* convA = static_cast<const unsigned char*>(a);
	const unsigned char* convB = static_cast<const unsigned char*>(b);

	for (size_t i = 0; i < count; i++) {
		if (convA[i] == convB[i]) {
			continue;
		}
		if (convA[i] < convB[i]) {
			return -1;
		}
		if (convA[i] > convB[i]) {
			return 1;
		}
	}
	return 0;
}

void* Mem::Memmove(void* dest, const void* src, size_t count) {
	unsigned char* convDest = static_cast<unsigned char*>(dest);
	const unsigned char* convSrc = static_cast<const unsigned char*>(src);
	uintptr_t destAddr = reinterpret_cast<uintptr_t>(dest);
	uintptr_t srcAddr = reinterpret_cast<uintptr_t>(src);

	if (destAddr > srcAddr && destAddr - srcAddr < count) {
		for (size_t i = count; i > 0; i--) {
			*(convDest + (i - 1)) = *(convSrc + (i - 1));
		}
	}
	else {
		for (size_t i = 0; i < count; i++) {
			*(convDest + i) = *(convSrc + i);
		}
	}

	return dest;
}