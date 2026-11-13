#pragma once
#include "graphics/graphics.h"

class Terminal{
private:
    uint64_t cursorX, cursorY;
    uint32_t textColor, backgroundColor;
    Graphics& graphics; // Reference to the graphics engine
    const BitmapFont& font;
public:
    Terminal(Graphics& graphics, const BitmapFont& font);
    void Initialize();
    void Write(const char* text);
    void WriteChar(char c);
    void NewLine();
    void Clear();
};