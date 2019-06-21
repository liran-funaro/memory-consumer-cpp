/*
Author: Liran Funaro <liran.funaro@gmail.com>

Copyright (C) 2006-2018 Liran Funaro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <regex>
#include <string>
#include <iostream>

#include "memory_consumer.hpp"

using std::regex;
using std::regex_constants::icase;

static const regex inputRegex("(load|memory|quit|perf|resetperf|maxrand)(?:\\s*:?\\s*(\\d+))?", icase);

int main(int argc, char** argv) {
	if(argc < 3) {
		std::cerr << "Parameters: <int: max memory (MB)> <float: sleep after write (seconds)>"
				<< std::endl;
		return 1;
	}

	unsigned long max_memory = (unsigned long) std::atol(argv[1]);
	double sleep_after_write_seconds = std::atof(argv[2]);
	MemoryConsumer mc(max_memory, sleep_after_write_seconds);

	std::smatch command_match;
	bool quit = false;
	for (std::string line; !quit && std::getline(std::cin, line);) {
		if (std::regex_search(line, command_match, inputRegex)) {
			quit = mc.doOp(command_match[1], command_match[2]);
		}
	}
	return 0;
}


