/*
 Author: Liran Funaro <liran.funaro@gmail.com>

 Copyright (C) 2006-2018 Liran Funaro

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without e
 ven the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef INCLUDE_WORKER_HPP_
#define INCLUDE_WORKER_HPP_

#include <vector>
#include <atomic>
#include <regex>
#include <random>
#include <algorithm>
#include <functional>
#include <climits>
#include <thread>
#include <memory>

#include <sys/mman.h>

#include "memory.hpp"
#include "stats.hpp"

using std::vector;
using std::atomic_bool;
using std::atomic_ulong;
using std::atomic_uint;
using std::unique_ptr;
using std::thread;

using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;


class Worker {
public:
	random_bytes_engine rbe;
	atomic_bool running = {false};

	unsigned long sleep_after_write_ms;

	Memory& memory;
	Stats& stats;

	unique_ptr<thread> t;

	Worker(unsigned long sleep_after_write_ms, Memory& memory, Stats& stats) :
		sleep_after_write_ms(sleep_after_write_ms), memory(memory), stats(stats) {
	}

	void startWorker() {
		if(running.load())
			return;

		t = std::make_unique<thread>(&Worker::run, this);
		running.store(true);
	}

	void stopWorker() {
		running.store(false);
	}

	void join() {
		if(t.get())
			t->join();
	}

	void run() {
		while (running.load()) {
			randomWrite();
			if (running.load() && sleep_after_write_ms > 1)
				 std::this_thread::sleep_for(std::chrono::milliseconds(sleep_after_write_ms));
		}
	}

	void randomWrite() {
		if (memory.max_rand <= 0)
			return;
		std::uniform_int_distribution<unsigned int> dist(0, memory.max_rand.load());
		unsigned int index = dist(rbe);
		stats.requests++;

		if (index >= memory.mem_top)
			return;

		memory.randomWrite(index, rbe);
		stats.hits++;
	}
};


#endif /* INCLUDE_WORKER_HPP_ */
