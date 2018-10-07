# Copyright 2018 Yudetamago. All rights reserved.
# Use of this source code is governed by a MIT
# license that can be found in the LICENSE file.

TEST_DIR       := .
TEST_CASES     := $(wildcard test_*.cpp)
TEST_CASES_BIN := $(patsubst %.cpp,%,$(TEST_CASES))
VPATH          := $(TEST_DIR)
INCLUDE_FLAGS  := -I../src
CPP_FLAGS      := -Wall -Werror $(INCLUDE_FLAGS) -lCppUTest -lCppUTestExt

test_%: test_%.cpp
	g++ $^ -o $@ $(CPP_FLAGS)

.PHONY: test
test: $(TEST_CASES_BIN)
	@echo $(TEST_CASES_BIN)
	for t in $(TEST_CASES_BIN) ; do \
		./$$t; \
	done

.PHONY: clean
clean:
	-rm -f $(TEST_CASES_BIN)
