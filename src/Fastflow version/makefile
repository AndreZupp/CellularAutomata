CXX=g++
CXXFLAGS= -pthread -I -std=c++17 -fopenmp -O3
DEPS = cellularautomataff.hpp rules.hpp

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)
fastflowsimulation: test.o cellularautomataff.o rules.o utimer.o
	$(CXX) -o fastflowsimulation test.o cellularautomataff.o rules.o $(CXXFLAGS)
