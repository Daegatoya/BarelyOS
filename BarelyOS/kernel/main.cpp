#include "../boot/limine.h"
#include "terminal/terminal.h"
#include "fonts/bitmap_font.h"
#include "fonts/font8xx8_basic.h"
#include "keyboard/keyboard.h"
#include "shell/shell.h"
#include "memory/pmm.h"

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t requests_start[4] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile uint64_t base_revision[3] = LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile limine_framebuffer_request framebuffer_request = {
    LIMINE_FRAMEBUFFER_REQUEST_ID,
    0,
    nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile limine_memmap_request memmap_request = {
    LIMINE_MEMMAP_REQUEST_ID,
    0,
    nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile limine_hhdm_request hhdm_request = {
    LIMINE_HHDM_REQUEST_ID,
    0,
    nullptr
};

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t requests_end[2] = LIMINE_REQUESTS_END_MARKER;

void KernelInitialize(uint8_t* framebuffer, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp, uint64_t totalMemory, const limine_memmap_response* memmap, uint64_t hhdmOffset) {
    Graphics graphics(framebuffer, width, height, pitch, bpp);
    BitmapFont font(8, 8, &font8x8_basic[0][0]);
    Terminal terminal(graphics, font);
    Keyboard keyboard;
    Shell shell(terminal, totalMemory);
    PhysicalMemoryManager pmm(memmap, hhdmOffset);


    terminal.Initialize();
    if (!pmm.Initialize()) {
        terminal.Write("Physical Memory Manager initialization failed.\n");

        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    terminal.Write("Physical Memory Manager initialized.\n");
    terminal.Write("\nInitializing Physical Memory Manager tests...\n");

    uint64_t freeBefore = pmm.GetFreePages();

    // 1. Allocate three physical pages
    if (freeBefore < 3) {
        terminal.Write("Physical Memory Manager test failed: insufficient free pages.\n");
    }
    else {
        uint64_t page1 = pmm.AllocatePage();
        uint64_t page2 = pmm.AllocatePage();
        uint64_t page3 = pmm.AllocatePage();

        // 2. Validate allocated pages
        bool allocationsValid =
            page1 != UINT64_MAX &&
            page2 != UINT64_MAX &&
            page3 != UINT64_MAX &&
            page1 != page2 &&
            page1 != page3 &&
            page2 != page3 &&
            pmm.GetFreePages() == freeBefore - 3;

        if (!allocationsValid) {
            terminal.Write("Physical Memory Manager allocation test failed.\n");
        }
        else {
            terminal.Write("Physical Memory Manager allocation test completed successfully.\n");

            // 3. Validate HHDM address conversions
            bool addressesValid =
                page1 <= UINT64_MAX - hhdmOffset &&
                page2 <= UINT64_MAX - hhdmOffset &&
                page3 <= UINT64_MAX - hhdmOffset;

            if (!addressesValid) {
                terminal.Write("Physical Memory Manager address validation failed.\n");
            }
            else {
                volatile uint64_t* ptr1 =
                    reinterpret_cast<volatile uint64_t*>(page1 + hhdmOffset);

                volatile uint64_t* ptr2 =
                    reinterpret_cast<volatile uint64_t*>(page2 + hhdmOffset);

                volatile uint64_t* ptr3 =
                    reinterpret_cast<volatile uint64_t*>(page3 + hhdmOffset);

                // 4. Write test values to physical memory
                *ptr1 = 0x123456789ABCDEF0ULL;
                *ptr2 = 0xDEADBEEFCAFEBABEULL;
                *ptr3 = 0x1122334455667788ULL;

                // 5. Verify stored values
                bool memoryValid =
                    *ptr1 == 0x123456789ABCDEF0ULL &&
                    *ptr2 == 0xDEADBEEFCAFEBABEULL &&
                    *ptr3 == 0x1122334455667788ULL;

                if (memoryValid) {
                    terminal.Write("Physical Memory Manager read/write test completed successfully.\n");
                }
                else {
                    terminal.Write("Physical Memory Manager read/write test failed.\n");
                }
            }

            // 6. Release allocated pages
            bool freed1 = pmm.FreePage(page1);
            bool freed2 = pmm.FreePage(page2);
            bool freed3 = pmm.FreePage(page3);

            bool releaseValid =
                freed1 && freed2 && freed3 &&
                pmm.GetFreePages() == freeBefore;

            if (releaseValid) {
                terminal.Write("Physical Memory Manager release test completed successfully.\n");
            }
            else {
                terminal.Write("Physical Memory Manager release test failed.\n");
            }
        }
    }
    terminal.Write("BarelyOS v0.1\n");
    terminal.Write("An operating system. Barely.\n");
    terminal.Write("Hello World!\n\n");
    terminal.Write("BarelyOS> ");

    uint8_t scanCode = 0;

    while (true) {
        bool hasDataToRead = keyboard.ReadScanCode(scanCode);
        if (!hasDataToRead) {
            continue;
        }
        else {
            char c = keyboard.ScanCodeToChar(scanCode);
            if (c != '\0') {
                shell.HandleInput(c);
            }
        }
    }
}

extern "C" void kmain(){
    if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    auto* response = framebuffer_request.response;
    auto* memmap = memmap_request.response;
    auto* hhdm = hhdm_request.response;

    if (response == nullptr || response->framebuffer_count == 0 || response->framebuffers == nullptr) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    if (memmap == nullptr || memmap->entries == nullptr) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    if (hhdm == nullptr) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    uint64_t totalUsableMemory = 0;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        if (memmap->entries[i] == nullptr || memmap->entries[i]->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }
        else {
            totalUsableMemory += memmap->entries[i]->length;
        }
    }

    auto* fb = response->framebuffers[0];

    if (fb == nullptr || fb->address == nullptr || fb->bpp != 32 || fb->memory_model != LIMINE_FRAMEBUFFER_RGB) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    if (fb->red_mask_size != 8 || fb->red_mask_shift != 16 || fb->green_mask_size != 8 || fb->green_mask_shift != 8 || fb->blue_mask_size != 8 || fb->blue_mask_shift != 0) {
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    KernelInitialize(static_cast<uint8_t*>(fb->address), fb->width, fb->height, fb->pitch, fb->bpp, totalUsableMemory, memmap, hhdm->offset);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}