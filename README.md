# CellularAutomata

This folder contains the deliver of the SPM course project at @Unipi of Andrea Zuppolini.

The project is: Cellular Automata framework, a small library that implements simple parallel Cellular Automata.
The code is mostly ultimated and details about how to run it and the design choices will be provided in the report.

However, there are some short instruction to run the code.

## Requisites:
- The compiler used is gcc9 but it also works with gcc10 (the version used on the server).
- In order to compile the framework it is necessary to go into one of the project folders (Fastflow version or Normal version)
  and run the command: make.
- Please notice that make needs to know where g++ is, so it may be the case that the g++ path has to be updated in the makefile.
  The makefile also provides an output executable called "simulation" or "fastflowsimulation", if you don't want to spend a lot of time  
  writing compiling instructions for other cpp files, you can easily modify those 2.

## Basic usage:

1) Include "cellularautomata.hpp" or "cellularautomataff.hpp" depending on which version of the framework you want to use.
2) Include "rules.hpp" in order to have at least 2 rules (brianbrain and gameOfLife) to use, without needing to write your one.
3) Create Cellular automata object (you can follow the example in the test.cpp file)
4) Execute the simulation the way you prefer. (read below)

In the normal version there're 3 ways to execute the simulation:
- Sequential run
- Thread execution
- OpenMP parallel for

In the Fastflow version there're 2 ways to execute the simulation
- Fastflow parallel for
- Fastflow farm
