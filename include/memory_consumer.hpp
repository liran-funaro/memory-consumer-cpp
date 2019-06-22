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
#include <algorithm>
#include <functional>
#include <climits>
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <sys/mman.h>

#include "memory.hpp"
#include "stats.hpp"
#include "worker.hpp"

using std::vector;
using std::atomic_ulong;
using std::atomic_uint;
using std::unique_ptr;
using std::string;


class MemoryConsumer {
	unsigned long sleep_after_write_ms;

	Memory memory;
	Stats stats;

	vector<unique_ptr<Worker>> workers;

	std::mutex load_lock;
	std::condition_variable load_cv;
	atomic_uint load_target = {0};
	unique_ptr<thread> load_thread;

	std::mutex memory_lock;
	std::condition_variable memory_cv;
	atomic_uint memory_target = {0};
	unique_ptr<thread> memory_thread;

public:
	MemoryConsumer(unsigned int max_memory, double sleep_after_write_seconds) :
		sleep_after_write_ms((unsigned long)(sleep_after_write_seconds * 1000)),
		memory(max_memory) {
		load_thread = std::make_unique<thread>(&MemoryConsumer::changeLoadThread, this);
		memory_thread = std::make_unique<thread>(&MemoryConsumer::changeMemoryThread, this);
	}

	void addLoad() {
		unique_ptr<Worker> w(std::make_unique<Worker>(sleep_after_write_ms, memory, stats));
		w->startWorker();
		workers.push_back(std::move(w));
	}

	void reduceLoad() {
		int sz = workers.size();
		if (sz == 0)
			return;

		unique_ptr<Worker> w = std::move(workers.back());
		workers.pop_back();
		w->stopWorker();
		w->join();
	}

	void changeMemoryThread() {
		while(true) {
			std::unique_lock<std::mutex> lk(memory_lock);
			memory_cv.wait(lk, [this]{return memory.size() != memory_target.load();});
			auto memoryTarget = memory_target.load();
			memory_lock.unlock();

			if (memory.size() < memoryTarget)
				memory.allocMemory(1);
			else if (memory.size() > memoryTarget)
				memory.releaseMemory(memory.size() - memoryTarget);
		}
	}

	void changeMemory(unsigned int memoryTarget) {
		std::unique_lock<std::mutex> lk(memory_lock);
		this->memory_target.store(memoryTarget);
		memory_cv.notify_one();
	}

	void changeLoadThread() {
		while(true) {
			std::unique_lock<std::mutex> lk(load_lock);
			load_cv.wait(lk, [this]{return workers.size() != load_target.load();});
			auto loadTarget = load_target.load();
			load_lock.unlock();

			if (workers.size() < loadTarget)
				addLoad();
			else if (workers.size() > loadTarget)
				reduceLoad();
		}
	}

	void changeLoad(unsigned int loadTarget) {
		std::unique_lock<std::mutex> lk(load_lock);
		this->load_target.store(loadTarget);
		load_cv.notify_one();
	}

	bool doOp(string op, string param) {
		if (op.compare("load") == 0)
			changeLoad((unsigned int) std::stoul(param));
		else if (op.compare("memory") == 0)
			changeMemory((unsigned int) std::stoul(param));
		else if (op.compare("perf") == 0) {
			auto perf = stats.perf();
			auto UUID = std::stoul(param);
			std::cout << "memory: " << memory.size()
					  << ", load: " << workers.size()
					  << ", hit-rate: "<< perf.hit_rate
					  << ", throughput: " << perf.throughput
					  << ", duration: " << perf.duration
					  << ", UUID: " << UUID
					  << std::endl;
		} else if (op.compare("resetperf") == 0)
			stats.resetperf();
		else if  (op.compare("maxrand") == 0)
			memory.max_rand.store((unsigned int) std::stoul(param));
		else if (op.compare("quit") == 0) {
			changeLoad(0);
			changeMemory(0);
			return true;
		}

		return false;
	}
};


#endif /* INCLUDE_MEMORY_CONSUMER_HPP_ */
