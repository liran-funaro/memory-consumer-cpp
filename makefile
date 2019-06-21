# Default compile parameters
color?=yes

BIN_DIR=bin
CC=g++-8

CPP_FILES=$(shell find ./src -not -path '*/.*/*' -type f -name '*.cpp')
HEADER_FILES=$(shell find . -not -path '*/.*/*' -type f \( -iname \*.hpp -o -iname \*.h \))

INCLUDE_FOLDERS=include
SRC_FOLDERS=src

CPP_INCLUDE=$(foreach dir,$(INCLUDE_FOLDERS),-I$(dir))
CPP_SOURCE=$(foreach dir,$(SRC_FOLDERS),$(dir)/*.cpp)

CPP_TEST_SOURCE=$(TESTS_DIR)/$(test).cpp

CPP_FLAGS=-std=c++17 -O3 -Wall -Wextra -Werror -pedantic-errors -pthread $(CPP_INCLUDE)
ifeq (color, yes)
all::
	CPP_FLAGS += -fdiagnostics-color=always
endif

COMPILE=$(CC) $(CPP_FLAGS) $(CPP_SOURCE)
DEBUG_FLAGS=-g -D DEBUG

NAME=memory_consumer

EXEC=$(BIN_DIR)/$(NAME)
EXEC_DEBUG=$(BIN_DIR)/$(NAME)-debug

all: buildpath $(EXEC)

debug: buildpath $(EXEC_DEBUG)

$(EXEC): $(CPP_FILES) $(HEADER_FILES)
	$(COMPILE) -o $(EXEC)
	
$(EXEC_DEBUG): $(CPP_FILES) $(HEADER_FILES)
	$(COMPILE) $(DEBUG_FLAGS) -o $(EXEC_DEBUG)
	
buildpath:
	@if [ ! -d "$(BIN_DIR)" ]; then mkdir $(BIN_DIR); fi

clean:
	if [ -d "$(BIN_DIR)" ]; then rm -R $(BIN_DIR); fi

