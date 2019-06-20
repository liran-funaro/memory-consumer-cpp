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
#ifndef INCLUDE_MEMORY_CONSUMER_HPP_
#define INCLUDE_MEMORY_CONSUMER_HPP_

#include <vector>
#include <atomic>
#include <random>
#include <algorithm>
#include <functional>
#include <climits>
#include <memory>
#include <string>
#include <iostream>

#include <sys/mman.h>

#include "memory.hpp"
#include "stats.hpp"
#include "worker.hpp"

using std::vector;
using std::atomic_ulong;
using std::atomic_uint;
using std::unique_ptr;
using std::string;

using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;

class MemoryConsumer {
public:


public:
	random_bytes_engine rbe;
	unsigned long sleep_after_write_ms;

	Memory memory;
	Stats stats;

	vector<unique_ptr<Worker>> workers;

public:
	MemoryConsumer(unsigned int max_memory, double sleep_after_write_seconds) :
		sleep_after_write_ms((unsigned long)(sleep_after_write_seconds * 1000)),
		memory(max_memory) {
	}

	void addLoad() {
		unique_ptr<Worker> w(std::make_unique<Worker>(sleep_after_write_ms, memory, stats));
		w->startWorker();
		workers.push_back(std::move(w));
	}

	unique_ptr<Worker> reduceLoad() {
		int sz = workers.size();
		if (sz == 0)
			return NULL;

		unique_ptr<Worker> w = std::move(workers.back());
		workers.pop_back();
		w->stopWorker();
		return w;
	}

	void changeMemory(unsigned int mem_target) {
		while (memory.size() < mem_target)
			memory.allocMemory(mem_target - memory.size());
		while (memory.size() > mem_target)
			memory.releaseMemory(memory.size() - mem_target);
	}

	void changeLoad(unsigned int load_target) {
		while (workers.size() < load_target)
			addLoad();

		vector<unique_ptr<Worker>> terminated;
		while (workers.size() > load_target)
			terminated.push_back(reduceLoad());

		for (unique_ptr<Worker>& w : terminated)
			w->join();
	}

	bool doOp(string op, string param) {
		if (op.compare("load") == 0)
			changeLoad((unsigned int) std::stoi(param));
		else if (op.compare("memory") == 0)
			changeMemory((unsigned int) std::stoi(param));
		else if (op.compare("perf") == 0) {
			auto perf = stats.perf();
			std::cout << "memory: " << memory.mem_top.load()
					  << ", load: " << workers.size()
					  << ", hit-rate: "<< perf.hit_rate
					  << ", throughput: " << perf.throughput
					  << ", duration: " << perf.duration
					  << std::endl;
		} else if (op.compare("resetperf") == 0)
			stats.resetperf();
		else if  (op.compare("maxrand") == 0)
			memory.max_rand.store((unsigned int) std::stoi(param));
		else if (op.compare("quit") == 0) {
			changeLoad(0);
			changeMemory(0);
			return true;
		}

		return false;
	}
};


#endif /* INCLUDE_MEMORY_CONSUMER_HPP_ */
