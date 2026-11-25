#include "scheduler.h"
static Scheduler* active;

extern "C" void ThreadStart() {
	active->StartCurrentThread();
}

extern "C" void ContextSwitch(CPUContext* oldContext, CPUContext* newContext);
extern "C" void ThreadTrampoline();

Scheduler::Scheduler(uint64_t hhdmOffset, PhysicalMemoryManager* pmm) : hhdmOffset(hhdmOffset), pmm(pmm) {
	this->threadCount = 0;
	this->currentThreadIndex = UINT64_MAX; // We don't initialize at 0 because no thread exists yet!
}

bool Scheduler::Initialize() {
	if (this->pmm == nullptr || this->initialized) {
		return false;
	}

	for (uint64_t i = 0; i < 8; i++) {
		this->threads[i] = Thread{};
	}

	if (active != nullptr) {
		return false;
	}

	active = this;
	this->initialized = true;
	return true;
}

uint64_t Scheduler::Schedule() {
	uint64_t i = this->currentThreadIndex == UINT64_MAX ? 0 : (this->currentThreadIndex + 1) % 8;

	for (uint64_t offset = 0; offset < 8; offset++) {
		uint64_t index = (i + offset) % 8;

		if (this->threads[index].state == ThreadState::READY) {
			return index;
		}
	}

	return UINT64_MAX;
}

void Scheduler::Yield() {
	if (!this->initialized) {
		return;
	}

	uint64_t previousIndex = this->currentThreadIndex;

	if (this->currentThreadIndex < 8) {
		this->threads[this->currentThreadIndex].state = ThreadState::READY;
	}

	uint64_t indexReady = Schedule();

	if (indexReady == UINT64_MAX) {
		return;
	}

	this->currentThreadIndex = indexReady;
	this->threads[this->currentThreadIndex].state = ThreadState::RUNNING;
	if (previousIndex == indexReady) {
		return;
	}
	CPUContext* oldContext = previousIndex == UINT64_MAX ? &this->kernelContext : &this->threads[previousIndex].context;
	ContextSwitch(oldContext, &this->threads[this->currentThreadIndex].context);

	if (previousIndex == UINT64_MAX) {
		this->currentThreadIndex = UINT64_MAX;
		for (int i = 0; i < 8; i++) {
			if (this->threads[i].state == ThreadState::FINISHED) {
				if (this->pmm->FreePage(this->threads[i].stackPhysicalAddress)) {
					this->threads[i] = Thread{};
				}
			}
		}
	}
}

uint64_t Scheduler::GetThreadCount() const {
	return this->threadCount;
}

void Scheduler::ExitCurrentThread() {
	if (this->currentThreadIndex >= 8) {
		return;
	}

	if (this->threads[this->currentThreadIndex].state != ThreadState::RUNNING) {
		return;
	}

	Thread* tmp = &this->threads[this->currentThreadIndex];
	tmp->state = ThreadState::FINISHED;
	this->threadCount--;
	ContextSwitch(&tmp->context, &this->kernelContext);
}

void Scheduler::StartCurrentThread() {
	if (this->currentThreadIndex >= 8) {
		return;
	}

	void (*entryPoint)() = this->threads[this->currentThreadIndex].entryPoint;
	entryPoint();
	ExitCurrentThread();
}

bool Scheduler::CreateThread(void(*entryPoint)()) {
	if (!this->initialized || entryPoint == nullptr) {
		return false;
	}

	uint64_t indexAvailable = UINT64_MAX;

	for (uint64_t i = 0; i < 8; i++) {
		if (this->threads[i].state == ThreadState::UNUSED) {
			indexAvailable = i;
			break;
		}
	}

	if (indexAvailable == UINT64_MAX) {
		return false;
	}

	uint64_t page = this->pmm->AllocatePage();

	if (page == UINT64_MAX) {
		return false;
	}

	if (this->hhdmOffset > UINT64_MAX - page ||
		UINT64_MAX - (page + this->hhdmOffset) < 4096) {
		this->pmm->FreePage(page);
		return false;
	}

	uint64_t stackVirtualBase = page + this->hhdmOffset;
	uint64_t stackVirtualTop = stackVirtualBase + 4096;
	uint64_t initialRsp = stackVirtualTop - sizeof(uint64_t);
	CPUContext context{};
	context.RSP = initialRsp;

	uint64_t* returnAddress = reinterpret_cast<uint64_t*>(initialRsp);
	*returnAddress = reinterpret_cast<uint64_t>(&ThreadTrampoline);

	this->threads[indexAvailable].stackBase = reinterpret_cast<uint64_t*>(stackVirtualBase);
	this->threads[indexAvailable].stackPhysicalAddress = page;
	this->threads[indexAvailable].entryPoint = entryPoint;
	this->threads[indexAvailable].context = context;
	this->threads[indexAvailable].id = this->nextThreadId++;
	this->threads[indexAvailable].state = ThreadState::READY;

	this->threadCount++;

	return true;
}

bool Scheduler::GetThreadInfo(uint64_t index, uint64_t& id, ThreadState& state) const {
	if (index >= 8 || this->threads[index].state == ThreadState::UNUSED) {
		return false;
	}

	const Thread& tmp = this->threads[index];
	id = tmp.id;
	state = tmp.state;
	return true;
}