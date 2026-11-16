#include <stdint.h>
#include "../arch/io.h"
#include "scanCodeTable.h"

class Keyboard {
public:
	Keyboard();
	bool HasData();
	bool ReadScanCode(uint8_t& scanCode);
	char ScanCodeToChar(uint8_t scanCode);
};