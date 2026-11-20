#include "pmm.h"

PhysicalMemoryManager::PhysicalMemoryManager(const limine_memmap_response* memmap, uint64_t hhdmOffset) : memmap(memmap), hhdmOffset(hhdmOffset) {
}

bool PhysicalMemoryManager::Initialize() {
    if (this->initialized || this->memmap == nullptr || this->memmap->entries == nullptr) {
        return false;
    }
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

        uint64_t bitmapFirstPage = this->bitmapPhysicalAddress / this->PAGE_SIZE;
        uint64_t bitmapEndPage = bitmapFirstPage + bitmapPages;

        for (uint64_t i = 0; i < memmap->entry_count; i++) {
            auto* entry = memmap->entries[i];

            if (entry == nullptr || entry->type != LIMINE_MEMMAP_USABLE) {
                continue;
            }
            else {
                uint64_t firstPage = (entry->base / this->PAGE_SIZE) + (entry->base % this->PAGE_SIZE != 0);
                if (entry->length > UINT64_MAX - entry->base) {
                    continue;
                }
                uint64_t regionEnd = entry->base + entry->length;
                uint64_t endPage = regionEnd / this->PAGE_SIZE;

                for (uint64_t page = firstPage; page < endPage; page++) {
                    if (page >= bitmapFirstPage && page < bitmapEndPage) {
                        continue;
                    }
                    else {
                        MarkFreePage(page);
                    }
                }
            }
        }
    }
    this->initialized = true;
    return true;
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

uint64_t PhysicalMemoryManager::AllocatePage() {
    if (!this->initialized || this->bitmap == nullptr || this->freePages == 0) {
        return UINT64_MAX;
    }
    for (uint64_t i = 0; i < this->bitmapSize; i++) {
        uint8_t bits = this->bitmap[i];
        for (uint8_t bitIndex = 0; bitIndex < 8; bitIndex++) {
            uint8_t mask = 1u << bitIndex;

            uint64_t pageIndex = i * 8 + bitIndex;

            if (pageIndex >= this->totalPages) {
                continue;
            }

            if ((bits & mask) == 0) {
                this->bitmap[i] |= mask;
                this->freePages--;

                return pageIndex * this->PAGE_SIZE;
            }
        }
    }
    return UINT64_MAX;
}

bool PhysicalMemoryManager::FreePage(uint64_t physicalAddress) {
    if (!this->initialized || this->bitmap == nullptr) {
        return false;
    }

    if (physicalAddress % this->PAGE_SIZE != 0) {
        return false;
    }

    uint64_t pageIndex = physicalAddress / this->PAGE_SIZE;

    if (pageIndex >= this->totalPages) {
        return false;
    }

    uint64_t bitmapFirstPage = this->bitmapPhysicalAddress / this->PAGE_SIZE;

    uint64_t bitmapPages = (this->bitmapSize / this->PAGE_SIZE) + (this->bitmapSize % this->PAGE_SIZE != 0);

    if (pageIndex >= bitmapFirstPage && pageIndex - bitmapFirstPage < bitmapPages) {
        return false;
    }

    bool usable = false;

    for (uint64_t i = 0; i < this->memmap->entry_count; i++) {
        auto* entry = this->memmap->entries[i];

        if (entry == nullptr || entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        if (entry->length > UINT64_MAX - entry->base) {
            continue;
        }

        uint64_t regionEnd = entry->base + entry->length;

        if (physicalAddress >= entry->base && physicalAddress < regionEnd && regionEnd - physicalAddress >= this->PAGE_SIZE) {
            usable = true;
            break;
        }
    }

    if (!usable) {
        return false;
    }

    uint64_t byteIndex = pageIndex / 8;
    uint8_t bitIndex = pageIndex % 8;
    uint8_t mask = static_cast<uint8_t>(1u << bitIndex);

    if ((this->bitmap[byteIndex] & mask) == 0) {
        return false;
    }

    this->MarkFreePage(pageIndex);
    return true;
}

uint64_t PhysicalMemoryManager::GetFreePages() const {
    return this->freePages;
}

uint64_t PhysicalMemoryManager::GetUsedPages() const {
    return this->totalPages - this->freePages;
}