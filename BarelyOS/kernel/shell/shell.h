#pragma once
#include <stdint.h>
#include "../terminal/terminal.h"

class Shell {
private:
	char buffer[128];
	uint32_t inputLength;
	Terminal& terminal;
	uint64_t availableMem;
public:
	Shell(Terminal& terminal, uint64_t availableMem);
	void HandleInput(char c);
	void ExecuteCommand();
};