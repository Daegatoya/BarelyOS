#include "terminal.h"

Terminal::Terminal(Graphics& graphics, BitmapFont& font) : graphics(graphics), font(font) {
	this->cursorX = 0;
	this->cursorY = 0;
	this->textColor = 0x00FFFFFF;
	this->backgroundColor = 0x00000000;
}

void Terminal::Initialize() {
	this->cursorX = 0;
	this->cursorY = 0;
	Clear();
}

void Terminal::NewLine() {
	this->cursorX = 0;
	this->cursorY += 1;
}

void Terminal::Write(const char* text) {
	int i = 0;
	if (text == nullptr) {
		return;
	}
	while (*(text + i) != '\0') {
		WriteChar(*(text + i));
		i++;
	}
}

void Terminal::WriteChar(char c) {
	if (c == '\n') {
		NewLine();
		return;
	}
	uint64_t xPixel = this->cursorX * font.GetWidth();
	uint64_t yPixel = this->cursorY * font.GetHeight();
	graphics.DrawChar(c, xPixel, yPixel, this->textColor, this->font);
	this->cursorX += 1;
	// TODO: line return, manage of bottom of the screen
}

void Terminal::Clear() {
	this->cursorX = 0;
	this->cursorY = 0;
	graphics.Clear(this->backgroundColor);
}