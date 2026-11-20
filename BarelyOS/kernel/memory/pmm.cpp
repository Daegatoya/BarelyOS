#include "pmm.h"

PhysicalMemoryManager::PhysicalMemoryManager(const limine_memmap_response* memmap, uint64_t hhdmOffset) : memmap(memmap), hhdmOffset(hhdmOffset) {
}

bool PhysicalMemoryManager::Initialize() {
    bool bitmapFound = false;

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        auto* entry = memmap->entries[i];

        if (entry == nullptr || entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length > UINT64_MAX - entry->base) {
            continue;
        }

        uint64_t regionEnd = entry->base + entry->length;

        if (regionEnd > highestEnd) {
            highestEnd = regionEnd;
        }
    }

    this->totalPages = (this->highestEnd / 4096) + (this->highestEnd % 4096 != 0);
    this->bitmapSize = (this->totalPages / 8) + (this->totalPages % 8 != 0);
    uint64_t bitmapPages = this->bitmapSize / this->PAGE_SIZE;

    if (this->bitmapSize % this->PAGE_SIZE != 0) {
        bitmapPages++;
    }

    if (bitmapPages > UINT64_MAX / this->PAGE_SIZE) {
        return false;
    }

    uint64_t bitmapReservedBytes = bitmapPages * this->PAGE_SIZE;

    for (uint64_t i = 0; i < this->memmap->entry_count; i++) {
        auto* entry = memmap->entries[i];

        if (entry == nullptr || entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uint64_t alignedBase = entry->base;
        uint64_t remainder = alignedBase % this->PAGE_SIZE;

        if (remainder != 0) {
            uint64_t adjustment = this->PAGE_SIZE - remainder;

            if (alignedBase > UINT64_MAX - adjustment) {
                continue;
            }

            alignedBase += adjustment;
        }

        if (entry->length > UINT64_MAX - entry->base) {
            continue;
        }

        uint64_t regionEnd = entry->base + entry->length;

        if (alignedBase > regionEnd) {
            continue;
        }

        uint64_t remainingBytes = regionEnd - alignedBase;

        if (remainingBytes >= bitmapReservedBytes) {
            this->bitmapPhysicalAddress = alignedBase;
            bitmapFound = true;
            break;
        }
    }
    if (!bitmapFound) {
        return false;
    }
    else {
        if (this->bitmapPhysicalAddress > UINT64_MAX - this->hhdmOffset) {
            return false;
        }

        this->bitmap = reinterpret_cast<uint8_t*>(this->bitmapPhysicalAddress + this->hhdmOffset);

        for (uint64_t i = 0; i < this->bitmapSize; i++) {
            this->bitmap[i] = 0xFF; // We put all bits in the bitmap to 1
        }
    }
}

void PhysicalMemoryManager::MarkFreePage(uint64_t pageIndex) {
    if (pageIndex >= this->totalPages) {
        return;
    }
    uint64_t byteIndex = pageIndex / 8;
    uint8_t bitIndex = pageIndex % 8;

    uint8_t mask = 1u << bitIndex;
    uint8_t reversedMask = ~mask;
    if ((this->bitmap[byteIndex] & mask) != 0) {
        this->bitmap[byteIndex] &= reversedMask;
        this->freePages++;
    }
}