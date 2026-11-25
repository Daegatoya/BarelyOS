#pragma once
#include <stdint.h>
#include "../terminal/terminal.h"
#include "../threading/scheduler.h"

class Shell {
private:
	char buffer[128];
	uint32_t inputLength;
	Terminal& terminal;
	uint64_t availableMem;
	static Shell* active;
	Scheduler& scheduler;

public:
	Shell(Terminal& terminal, uint64_t availableMem, Scheduler& scheduler);
	void HandleInput(char c);
	void ExecuteCommand();
};