#pragma once
#include <stdint.h>
#include "thread.h"
#include "../memory/pmm.h"

class Scheduler {
private:
	PhysicalMemoryManager* pmm;
	uint64_t hhdmOffset;
	Thread threads[8];
	uint64_t threadCount;
	uint64_t currentThreadIndex;
	CPUContext kernelContext{};
	bool initialized = false;
	uint64_t nextThreadId = 0;

	uint64_t Schedule();
public:
	Scheduler(uint64_t hhdmOffset, PhysicalMemoryManager* pmm);
	bool Initialize();
	bool CreateThread(void(*entryPoint)()); // We pass a function as a parameter
	void Yield();
	void ExitCurrentThread();
	uint64_t GetThreadCount() const;
	void StartCurrentThread();
};