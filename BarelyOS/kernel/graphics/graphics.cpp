#include "graphics.h"

Graphics::Graphics(uint8_t* framebuffer, uint64_t width, uint64_t height, uint64_t pitch, uint16_t bpp) : framebuffer(framebuffer), width(width), height(height), pitch(pitch), bpp(bpp) {

}

/*
Puts a pixel on a frambuffer
@param x : x coordinate of the pixel
@param y : y coordinate of the pixel
@param color : color of the pixel in 0x00RRGGBB format
*/
void Graphics::PutPixel(uint64_t x, uint64_t y, uint32_t color){
    if (this->framebuffer == nullptr) {
        return;
    }
    if (x >= width || y >= height) {
        return;
    }
    if (this->bpp != 32) {
        return;
    }
    uint8_t* pixel = this->framebuffer + y * this->pitch + x * 4; // Points to the byte of the pixel in the framebuffer
    *(uint32_t*)pixel = color; // We want to write 4 bytes (32 bits) because one pixel is 4 bytes (32 bits) in 0x00RRGGBB format
}

void Graphics::Clear(uint32_t color) {
    for (uint64_t j = 0; j < this->height; j++) {
        for (uint64_t i = 0; i < this->width; i++) {
            PutPixel(i, j, color);
        }
    }
}

void Graphics::DrawChar(char c, uint64_t x, uint64_t y, uint32_t color, const BitmapFont& font) {
    const uint8_t* glyph = font.GetGlyph(c);
    if (glyph == nullptr) {
        return;
    }
    uint8_t height = font.GetHeight();
    uint8_t width = font.GetWidth();

    for (int i = 0; i < height; i++) {
        uint8_t tmp = glyph[i];
        for (int j = 0; j < width; j++) {
            if (tmp & (0x80 >> j)) {
                PutPixel(x + j, y + i, color);
            }
        }
    }
}

