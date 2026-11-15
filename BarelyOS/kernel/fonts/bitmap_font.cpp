#include "bitmap_font.h"

BitmapFont::BitmapFont(uint8_t width, uint8_t height, const uint8_t* glyphs) : width(width), height(height), glyphs(glyphs) {

}

uint8_t BitmapFont::GetWidth() const {
	return this->width;
}

uint8_t BitmapFont::GetHeight() const {
	return this->height;
}

const uint8_t* BitmapFont::GetGlyph(char c) const {
	if (this->glyphs == nullptr) {
		return nullptr;
	}
	uint8_t ascii = static_cast<uint8_t>(c);
	if (ascii >= 128) {
		return nullptr;
	}
	uint64_t offset = ascii * this->height;
	return this->glyphs + offset;
}
