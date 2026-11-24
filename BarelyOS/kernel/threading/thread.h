#pragma once
#include <stdint.h>
#include "cpucontext.h"

enum class ThreadState {
	UNUSED,
	READY,
	RUNNING,
	FINISHED
};

// We use a struct because the object does NOT have responsibilities. It is only used to give important data to the scheduler
struct Thread {
	uint64_t id;
	ThreadState state;
	uint64_t* stackBase;
	uint64_t stackPhysicalAddress;
	CPUContext context;
	void(*entryPoint)();
};