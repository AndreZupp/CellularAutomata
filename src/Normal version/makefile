CXX=g++
CXXFLAGS= -pthread -I -std=c++17 -fopenmp -O3
DEPS = cellularautomata.hpp rules.hpp

%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)
simulation: test.o cellularautomata.o rules.o utimer.o
	$(CXX) -o simulation test.o cellularautomata.o rules.o $(CXXFLAGS)
