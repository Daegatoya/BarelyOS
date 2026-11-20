#pragma once
#include <stdint.h>
#include <limine.h>

class PhysicalMemoryManager {
private:
	static constexpr uint64_t PAGE_SIZE = 4096;
	uint8_t* bitmap = nullptr;
	uint64_t totalPages = 0;
	uint64_t freePages = 0;
	uint64_t bitmapSize = 0;
	uint64_t hhdmOffset;
	uint64_t bitmapPhysicalAddress = 0;
	uint64_t highestEnd = 0;
	const limine_memmap_response* memmap;
public:
	PhysicalMemoryManager(const limine_memmap_response* memmap, uint64_t hhdmOffset);
	bool Initialize();
	uint64_t AllocatePage();
	bool FreePage(uint64_t physicalAddress);
	bool MarkFreePage(uint64_t pageIndex);
	uint64_t GetFreePages() const;
	uint64_t GetUsedPages() const;
};