#include "shell.h"

Shell::Shell(Terminal& terminal, uint64_t availableMem) : terminal(terminal), availableMem(availableMem), buffer{}, inputLength(0) {
}

void Shell::HandleInput(char c) {
    if (c == '\b') {
        if (this->inputLength > 0) {
            this->inputLength--;
            this->buffer[this->inputLength] = '\0';
            this->terminal.BackSpace();
        }
    }
    else if (c == '\n') {
        this->terminal.NewLine();

        this->ExecuteCommand();

        this->inputLength = 0;
        this->buffer[0] = '\0';

        this->terminal.Write("BarelyOS> ");
    }
    else if (c != '\0' && this->inputLength < 127) {
        this->buffer[this->inputLength] = c;
        this->inputLength++;
        this->buffer[this->inputLength] = '\0';

        this->terminal.WriteChar(c);
    }
}

// Intern method to compare two strings
bool StringEquals(const char* a, const char* b) {
	uint32_t i = 0;

	while (a[i] != '\0' && b[i] != '\0') {
		if (a[i] != b[i]) {
			return false;
		}
		i++;
	}

	return a[i] == b[i];
}

void Shell::ExecuteCommand() {
	if (StringEquals(this->buffer, "help")) {
		this->terminal.Write("Available commands: help, clear, version, mem, tasks, spawn, files, showfile, write\n");
	}
	else if (StringEquals(this->buffer, "version")) {
		this->terminal.Write("BarelyOS v0.1\n");
	}
    else if (StringEquals(this->buffer, "clear")) {
        this->terminal.Clear();
    }
    else if (StringEquals(this->buffer, "mem")) {
        this->terminal.Write("Usable physical memory at boot: ");
        this->terminal.WriteNumber(this->availableMem / (1024 * 1024));
        this->terminal.Write(" MiB\n");
    }
    else if (StringEquals(this->buffer, "tasks")) {
        this->terminal.Write("Command not yet implemented.\n");
    }
    else if (StringEquals(this->buffer, "spawn")) {
        this->terminal.Write("Command not yet implemented.\n");
    }
    else if (StringEquals(this->buffer, "files")) {
        this->terminal.Write("Command not yet implemented.\n");
    }
    else if (StringEquals(this->buffer, "showfile")) {
        this->terminal.Write("Command not yet implemented.\n");
    }
    else if (StringEquals(this->buffer, "write")) {
        this->terminal.Write("Command not yet implemented.\n");
    }
	else if (this->buffer[0] != '\0') {
		this->terminal.Write("Unknown command.\n");
	}
}