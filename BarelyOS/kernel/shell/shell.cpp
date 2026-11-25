#include "shell.h"

Shell* Shell::active = nullptr;

Shell::Shell(Terminal& terminal, uint64_t availableMem, Scheduler& scheduler) : terminal(terminal), availableMem(availableMem), scheduler(scheduler), buffer{}, inputLength(0) {
    this->active = this;
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
        bool create = scheduler.CreateThread([]() {
            active->terminal.Write("\nAvailable commands: help, clear, version, mem, tasks, spawn, files, showfile, write\n\n");
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
        }
	}
	else if (StringEquals(this->buffer, "version")) {
        bool create = scheduler.CreateThread([]() {
            active->terminal.Write("\nBarelyOS v0.1\n\n");
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
        }
	}
    else if (StringEquals(this->buffer, "clear")) {
        bool create = scheduler.CreateThread([]() {
            active->terminal.Clear();
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
        }
    }
    else if (StringEquals(this->buffer, "mem")) {
        bool create = scheduler.CreateThread([]() {
            active->terminal.Write("\nUsable physical memory at boot: ");
            active->terminal.WriteNumber(active->availableMem / (1024 * 1024));
            active->terminal.Write(" MiB\n\n");
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
        }
    }
    else if (StringEquals(this->buffer, "tasks")) {
        bool create = scheduler.CreateThread([]() {
            uint64_t id;
            ThreadState state;

            active->terminal.Write("\nID\tSTATE\n\n");
            for (uint64_t i = 0; i < 8; i++) {
                bool success = active->scheduler.GetThreadInfo(i, id, state);
                if (!success) {
                    continue;
                }
                active->terminal.WriteNumber(id);
                active->terminal.Write("\t");
                switch (state) {
                case ThreadState::READY:
                    active->terminal.Write("READY");
                    break;
                case ThreadState::RUNNING:
                    active->terminal.Write("RUNNING");
                    break;
                case ThreadState::FINISHED:
                    active->terminal.Write("FINISHED");
                    break;
                default:
                    active->terminal.Write("ERROR");
                    break;
                }
                active->terminal.Write("\n");
            }

            active->terminal.Write("Amount of threads: ");
            active->terminal.WriteNumber(active->scheduler.GetThreadCount());
            active->terminal.Write("\n\n");
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
        }
    }
    else if (StringEquals(this->buffer, "spawn")) {
        this->terminal.Write("\nCreating thread...\n");
        bool create = scheduler.CreateThread([]() {
            active->terminal.Write("BarelyOS thread created!\n");
            });

        if (!create) {
            this->terminal.Write("\nFailed to create thread.\n\n");
        }
        else {
            scheduler.Yield();
            this->terminal.Write("Thread execution completed.\n\n");
        }
    }
    else if (StringEquals(this->buffer, "files")) {
        this->terminal.Write("\nCommand not yet implemented.\n\n");
    }
    else if (StringEquals(this->buffer, "showfile")) {
        this->terminal.Write("\nCommand not yet implemented.\n\n");
    }
    else if (StringEquals(this->buffer, "write")) {
        this->terminal.Write("\nCommand not yet implemented.\n\n");
    }
	else if (this->buffer[0] != '\0') {
		this->terminal.Write("\nUnknown command.\n\n");
	}
}