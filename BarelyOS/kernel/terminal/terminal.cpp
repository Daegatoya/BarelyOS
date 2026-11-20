#include "terminal.h"

Terminal::Terminal(Graphics& graphics, const BitmapFont& font) : graphics(graphics), font(font) {
	this->cursorX = 0;
	this->cursorY = 0;
	this->textColor = 0x00FFFFFF;
	this->backgroundColor = 0x00000000;
}

void Terminal::Clear() {
	this->cursorX = 0;
	this->cursorY = 0;
	this->graphics.Clear(this->backgroundColor);
}

void Terminal::WriteChar(char c) {
	if (c == '\n') {
		NewLine();
		return;
	}
	const uint64_t maxColumns = this->graphics.GetWidth() / this->font.GetWidth();

	if (this->cursorX >= maxColumns) {
		NewLine();
	}

	uint64_t xPixel = this->cursorX * font.GetWidth();
	uint64_t yPixel = this->cursorY * font.GetHeight();

	this->graphics.DrawChar(c, xPixel, yPixel, this->textColor, this->font);

	this->cursorX += 1;
}

void Terminal::Initialize() {
	Clear();
}

void Terminal::NewLine() {
	this->cursorX = 0;
	this->cursorY += 1;
	const uint64_t maxRows = this->graphics.GetHeight() / this->font.GetHeight();

	if (this->cursorY >= maxRows) {
		this->graphics.ScrollUp(this->font.GetHeight(), this->backgroundColor);
		this->cursorY = maxRows - 1;
	}
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

void Terminal::BackSpace() {
	if (this->cursorX != 0) {
		this->cursorX--;
		uint64_t pixelX = this->cursorX * this->font.GetWidth();
		uint64_t pixelY = this->cursorY * this->font.GetHeight();

		for (uint64_t y = 0; y < this->font.GetHeight(); y++) {
			for (uint64_t x = 0; x < this->font.GetWidth(); x++) {
				this->graphics.PutPixel(pixelX + x, pixelY + y, this->backgroundColor);
			}
		}
	}
}

void Terminal::WriteNumber(uint64_t number) {
	char digits[21]; // 21 because uint64_t can take 20 digits + the final \0.
	digits[20] = '\0';
	int i = 20;

	do {
		i--;
		uint64_t digit = number % 10;
		digits[i] = '0' + digit;
		number /= 10;
	} while (number > 0);

	Write(&digits[i]);
}