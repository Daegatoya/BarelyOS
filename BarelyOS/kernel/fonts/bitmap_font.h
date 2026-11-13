#pragma once
#include <stdint.h>

class BitmapFont {
private:
	uint8_t width, height;
	const uint8_t* glyphs; // Binary data of the font. We want it as a constant to avoid modifying it accidentally.

public:
	BitmapFont(uint8_t width, uint8_t height, const uint8_t* glyphs);
	uint8_t GetWidth();
	uint8_t GetHeight();
	const uint8_t* GetGlyph(char c);
};