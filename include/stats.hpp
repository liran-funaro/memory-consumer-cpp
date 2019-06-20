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
#ifndef INCLUDE_STATS_HPP_
#define INCLUDE_STATS_HPP_

#include <atomic>
#include <climits>
#include <memory>

#include <iomanip>
#include <ctime>
#include <chrono>

#include <sys/mman.h>

#include "memory.hpp"
#include "worker.hpp"

using std::atomic_ulong;
using std::unique_ptr;


class StatsResult {
public:
	double duration = 0;
	double hit_rate = 0;
	double throughput = 0;
};


class Stats {
public:
	std::chrono::steady_clock::time_point measureStart;
	atomic_ulong hits;
	atomic_ulong requests;

	Stats() : hits(0), requests(0) {}

	void resetperf() {
		measureStart = std::chrono::steady_clock::now();
		hits.store(0);
		requests.store(0);
	}

	StatsResult perf() {
		StatsResult res;
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
		res.duration = (double) std::chrono::duration_cast<std::chrono::seconds>(
				now - measureStart).count();
		if (res.duration > 0) {
			res.hit_rate = (double) (hits.load()) / res.duration;
			res.throughput = (double) (requests.load()) / res.duration;
		}

		return res;
	}
};


#endif /* INCLUDE_STATS_HPP_ */
