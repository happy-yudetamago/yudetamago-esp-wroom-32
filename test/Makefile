
TEST_DIR       := .
TEST_CASES     := $(wildcard test_*.cpp)
TEST_CASES_BIN := $(patsubst %.cpp,%,$(TEST_CASES))
VPATH          := $(TEST_DIR)
INCLUDE        :=

test_%: test_%.cpp
	g++ $^ -o $@

.PHONY: test
test: $(TEST_CASES_BIN)
	@echo $(TEST_CASES_BIN)
	for t in $(TEST_CASES_BIN) ; do \
		./$$t; \
	done
