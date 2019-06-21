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
#ifndef INCLUDE_MEMORY_HPP_
#define INCLUDE_MEMORY_HPP_

#include <vector>
#include <atomic>
#include <random>
#include <algorithm>
#include <functional>
#include <climits>
#include <memory>

#include <sys/mman.h>

#include <iostream>
#include <iomanip>

using std::vector;
using std::atomic_uint;
using std::generate;
using std::size_t;

using random_bytes_engine = std::independent_bits_engine<
    std::default_random_engine, CHAR_BIT, unsigned char>;


#define PAGE_SIZE ((1<<12))


class Memory {
public:
	static const unsigned long mb = (unsigned long) (1<<20);

public:
	const unsigned long max_memory;
	atomic_uint max_rand;
	atomic_uint mem_top;
	unsigned char *mem_arr;

	Memory(unsigned long max_memory) : max_memory(max_memory), max_rand(0), mem_top(0) {
		mem_arr = (unsigned char *) mmap(NULL, max_memory * mb,
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
		if (mem_arr == MAP_FAILED) {
			perror("mmap");
			exit(EXIT_FAILURE);
		}
	}

	~Memory() {
		munmap(mem_arr, mem_top * mb);
	}

	size_t size() const {
		return mem_top;
	}

	void allocMemory(unsigned int size_mb) {
		unsigned int mem_top = this->mem_top.load();
		if (mem_top >= max_memory)
			return;
		if (mem_top + size_mb > max_memory)
			size_mb = max_memory - mem_top;

		auto t = mem_arr + (mem_top * mb);
		auto e =  t + (size_mb * mb);
		for(;t < e; t += PAGE_SIZE)
			t[0] = '\0';
		this->mem_top.store(mem_top + size_mb);
	}

	void releaseMemory(unsigned int size_mb) {
		unsigned int mem_top = this->mem_top.load();
		if (mem_top == 0)
			return;

		if (size_mb > mem_top)
			size_mb = mem_top;

		mem_top -= size_mb;
		this->mem_top.store(mem_top);

		madvise(mem_arr + (mem_top*mb), (max_memory - mem_top)*mb,
				MADV_DONTNEED); //MADV_REMOVE
	}

	void randomWrite(unsigned int index, random_bytes_engine& rbe) {
		if (index >= mem_top.load())
			return;
		auto t = mem_arr + (index * mb);
		generate(t, t + mb, std::ref(rbe));
	}
};

#endif /* INCLUDE_MEMORY_HPP_ */
