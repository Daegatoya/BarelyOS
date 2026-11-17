#include "../boot/limine.h"
#include "terminal/terminal.h"
#include "fonts/bitmap_font.h"
#include "fonts/font8xx8_basic.h"
#include "keyboard/keyboard.h"

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

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t requests_end[2] = LIMINE_REQUESTS_END_MARKER;

void KernelInitialize(uint8_t* framebuffer, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp) {
    Graphics graphics(framebuffer, width, height, pitch, bpp);
    BitmapFont font(8, 8, &font8x8_basic[0][0]);
    Terminal terminal(graphics, font);
    Keyboard keyboard;

    terminal.Initialize();
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
            if (c == '\b') {
                terminal.BackSpace();
            }
            else if (c != '\0') {
                terminal.WriteChar(c);
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

    if (response == nullptr || response->framebuffer_count == 0 || response->framebuffers == nullptr) {
        for (;;) {
            __asm__ volatile("cli; hlt");
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

    KernelInitialize(static_cast<uint8_t*>(fb->address), fb->width, fb->height, fb->pitch, fb->bpp);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}