#pragma once
#include <stdint.h>
#include "../fonts/bitmap_font.h"
#include "../memory/mem.h"

class Graphics {
private:
	uint8_t* framebuffer; // Video memory address
	uint64_t width, height, pitch; // Width, height, and amount of bytes between two lines
	uint16_t bpp; // Amount of bits per pixel

public:
	Graphics(uint8_t* framebuffer, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp);
	void PutPixel(uint64_t x, uint64_t y, uint32_t color);
	void Clear(uint32_t color);
	void DrawChar(char c, uint64_t x, uint64_t y, uint32_t color, const BitmapFont& font);
	uint64_t GetWidth();
	uint64_t GetHeight();
	void ScrollUp(uint64_t pixelRows, uint32_t backgroundColor);
};