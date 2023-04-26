/**
    @brief Class and methods for Cellular automata computation using c++ Threads
    @file cellularAutomata.hpp
    @author Andrea Zuppolini
    @version 1 29/06/2021
*/

#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <omp.h>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

//Defining aliases
using neighbourhood = std::vector<int>;
using grid2D = std::vector<std::vector<int>>;
using namespace ff;

class CellularAutomata
{
private:
    grid2D grid;                          /**<Variable representing the grid*/
    int num_rows, num_columns, states, num_threads;    /**<Cellular Automata params*/
    int (*rule)(neighbourhood) = nullptr; /**<Cellular Automata update rule*/
    int timesteps;                        /**<Number of epochs*/

public:
    /** 
      Default constructor
      @param rows number of rows of the grid
      @param columns number of columns of the grid
      @param function rule to use in order to compute grid's next state
      @param tsteps number of generation executed
      @param random_init if true, the grid is randomly initialized
      @param numthreads number of threads for the execution
     */
    CellularAutomata(int rows, int columns, int (*function)(neighbourhood), int tsteps, int n_states, int numthreads);

    /** 
      Alternative constructor
      @param rows number of rows of the grid
      @param columns number of columns of the grid
      @param function rule to use in order to compute grid's next state
      @param tsteps number of generation executed
      @param initial_state provide an existing grid instead of creating a new, random, one
      @param numthreads number of threads for the execution
     */
    CellularAutomata(int rows, int columns, int (*function)(neighbourhood), int tsteps, grid2D initial_state, int numthreads);

    /**
     Method executing the sequential version of the CellularAutomata a single thread goes cell by cell updating the states
    */
    void sequentialRun();

    /**
     Method implementing the Thread execution of the Cellular Automata. it used the num_threads initialized at the construction
    */
    void threadsExecution();

    /**
     Method implementing a Thread execution of the Cellular Automata using OpenMP Parallel For
    */
    void ompParallelFor();

    /**
     Method executed by the threads created in the threadsExecution() method.
     @param a starting point of the interval
     @param b ending point of the interval
     @param grid_copy local copy of the grid's previous state
     @param barrier1 first of two barriers. This one is used in order to wait all the other threads executions.
     @param barrier2 Barrier used to wait the main thread which is updating the local copy of the grid's previous state
    */
    void exec(int a, int b, grid2D *grid_copy, pthread_barrier_t *barrier1, pthread_barrier_t *barrier2);

    /**
     Function used to random fill the grid. The number used are in the interval [0, states[
    */
    void randomFill();

    /**
     Method used to get all the neighbours of the cell (X,Y)
     @param x row-index of the cell
     @param y column-index of the cell
     @param grid_ reference to the grid from which the neighbourhood will be computed
     @returns a std::vector<int> containing: the current state of the cell at index [0], then, starting from the top left corner, all the other neighbours in a clockwise sense in the following index. 
    */
    std::vector<int> getNeighbourhood(int x, int y, grid2D *grid_);

    /**
     Method used to generate a deep copy of the grid
     @returns a deep copy of the grid
    */
    grid2D copyGrid();

    /**
     Method that prints the Cellular Automata grid
    */
    void printMatrix();

    /**
     Series of methods to get the next/previous column/row following a toroidal behaviour
     @param index value of row/column, the methods's name indicated which is the direction followed
     @returns the next index of row/column in the direction indicated by the methods's name 
    */
    int upperRow(int index);
    int lowerRow(int index);
    int leftColumn(int index);
    int rightColumn(int index);

    /**
     Method which check the correctness of some of the constructor's parameters.
     @param rows number of rows
     @param columns number of columns
     @param function Cellular Automata's update rule
     @param tsteps number of timesteps
     @param numthreads number of threads
     @returns whether the parameters are correct or not
    */
    bool checkParameters(int rows, int columns, int (*function)(neighbourhood), int tsteps, int numthreads);

    /**
     Setter and Getter methods
    */ 
    int getNumThreads();
    int getRows();
    int getColumns();
    int getTimeSteps();
    grid2D getGrid();
    void setRows(int rows);
    void setColumns(int columns);
    void setGrid(grid2D new_grid);
    void setNumThreads(int threads);
    void setRule(int (*func)(neighbourhood));
    

    /**
     Method used to re-initialize the grid
    */
    void restartGrid();
};

#endif
