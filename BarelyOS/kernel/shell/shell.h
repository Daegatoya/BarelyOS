#pragma once
#include <stdint.h>
#include "../terminal/terminal.h"

class Shell {
private:
	char buffer[128];
	uint32_t inputLength;
	Terminal& terminal;
public:
	Shell(Terminal& terminal);
	void HandleInput(char c);
	void ExecuteCommand();
};