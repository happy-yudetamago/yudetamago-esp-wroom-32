
TEST_DIR       := .
TEST_CASES     := $(wildcard test_*.cpp)
TEST_CASES_BIN := $(patsubst %.cpp,%,$(TEST_CASES))
VPATH          := $(TEST_DIR)
INCLUDE_FLAGS  := -I../src

test_%: test_%.cpp
	g++ $^ -o $@ $(INCLUDE_FLAGS)

.PHONY: test
test: $(TEST_CASES_BIN)
	@echo $(TEST_CASES_BIN)
	for t in $(TEST_CASES_BIN) ; do \
		./$$t; \
	done
