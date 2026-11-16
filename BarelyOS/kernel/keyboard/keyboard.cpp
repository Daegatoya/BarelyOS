#include "keyboard.h"

Keyboard::Keyboard() {

}

bool Keyboard::HasData() {
	uint8_t status = Inb(0x64); // 0x64 being the port for the PS/2 controller
	if (status & 0x01) { // We apply a mask to only select bit 0 and compare with the bitwise AND operation, basic computer math!
		return true;
	}
	else {
		return false;
	}
}

bool Keyboard::ReadScanCode(uint8_t& scanCode) {
	if (!HasData()) {
		return false;
	}
	scanCode = Inb(0x60); // 0x60 being where we must read the input byte
	return true;
}

char Keyboard::ScanCodeToChar(uint8_t scanCode) {
    if (!(scanCode & 0x80)) {
        return scanCodeTable[scanCode];
    }

    return '\0';
}