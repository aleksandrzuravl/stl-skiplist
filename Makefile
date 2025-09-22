CXX ?= g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -I.
TESTFLAGS = -lgtest -lgtest_main -pthread
TEST_TARGET = skip_list_test

all: test

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): skip_list_test.cpp skip_list.h skip_list_impl.h
	$(CXX) $(CXXFLAGS) $< -o $@ $(TESTFLAGS)

clean:
	rm -f $(TEST_TARGET)

format:
	clang-format -i skip_list.h skip_list_impl.h skip_list_test.cpp

.PHONY: all test clean format
