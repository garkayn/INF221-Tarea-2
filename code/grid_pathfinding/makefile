CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
PYTHON ?= python

ifeq ($(OS),Windows_NT)
LDLIBS += -lpsapi
endif

TARGET := grid_pathfinding
SRCS := grid_pathfinding.cpp \
        algorithms/brute_force.cpp \
        algorithms/dynamic_programming.cpp \
        algorithms/greedy.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

measurements:
	$(PYTHON) scripts/run_measurements.py

plots:
	$(PYTHON) scripts/plot_generator.py

.PHONY: all clean measurements plots
