# Simple test runner for pure functions
CXX = g++
CXXFLAGS = -std=c++11 -Wall -I.

test: test/test_pure_functions.cpp src/LatchImageHelper.h
	$(CXX) $(CXXFLAGS) test/test_pure_functions.cpp -o test_runner
	./test_runner
	@echo "✨ Tests completed successfully!"

clean:
	rm -f test_runner

.PHONY: test clean