#ifndef SHINOBU_CLOCK_H
#define SHINOBU_CLOCK_H

#include <atomic>
#include <chrono>

#include "core/reference.h"

typedef std::chrono::milliseconds milliseconds;

class ShinobuClock : public Reference {
	std::atomic<uint64_t> last_recorded_time;

public:
	ShinobuClock() {
		std::atomic_init(&last_recorded_time, (uint64_t)0);
	}

	uint64_t get_current_offset_msec() {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		std::chrono::high_resolution_clock::time_point lrt(milliseconds(last_recorded_time.load(std::memory_order_seq_cst)));
		auto diff = std::chrono::duration_cast<milliseconds>(now - lrt);
		return diff.count();
	}

	void measure() {
		uint64_t ms_count = std::chrono::time_point_cast<milliseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count();
		last_recorded_time.store(ms_count, std::memory_order_seq_cst);
	}
};
#endif