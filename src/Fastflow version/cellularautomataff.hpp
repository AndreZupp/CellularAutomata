/**
    @brief Class and methods for Cellular automata computation using FastFlow library
    @file cellularAutomataff.hpp
    @author Andrea Zuppolini
    @version 1 29/06/2021
*/

#include <iostream>
#include <vector>
#include <chrono>
#include "utimer.cpp"
#include <ff/ff.hpp>
#include <ff/pipeline.hpp>
#include <ff/farm.hpp>
#include <ff/parallel_for.hpp>
#include "rules.hpp"
#include <chrono>

using namespace ff;
#ifndef CELLULAR_AUTOMATA_FF
#define CELLULAR_AUTOMATA_FF

//Defining aliases
using neighbourhood = std::vector<int>;
using grid2D = std::vector<std::vector<int>>;
using namespace ff;

//Class definition for the Cellular Automata fast flow version.
class CellularAutomataff
{
    //Struct defining a pair of integers
    struct PAIR
    {
        int start;
        int end;
    };

private:
    grid2D *grid;                                 /**<Variable representing the grid*/
    grid2D *old_grid;                             /**<Copy of the grid above used to compute next state*/
    int num_rows, num_columns, states, timesteps; /**<Cellular Automata params*/
    int (*rule)(neighbourhood) = nullptr;         /**<Cellular Automata update rule*/
    int num_threads;                              /**<Number of threads for the execution*/

public:
    /** 
      Default constructor
      @param rows number of rows of the grid
      @param columns number of columns of the grid
      @param function rule to use in order to compute grid's next state
      @param tsteps number of generation executed
      @param numthreads number of threads for the execution
     */
    CellularAutomataff(int rows, int columns, int (*function)(neighbourhood), int tsteps, int n_states, int numthreads);


    /** 
      Alternative constructor
      @param rows number of rows of the grid
      @param columns number of columns of the grid
      @param function rule to use in order to compute grid's next state
      @param tsteps number of generation executed
      @param initial_state provide an existing grid instead of creating a new, random, one
      @param numthreads number of threads for the execution
     */
    CellularAutomataff(int rows, int columns, int (*function)(neighbourhood), int tsteps, grid2D *initial_state, int numthreads);
    /**
      Method used to randomly initialize the grid
      @param num_states number of states of the grid. Cells values will beat most (num_states - 1)
      @return nothing, operates of the grid by reference.
    */
    void randomFill();

    /**
     Method which runs the Cellular Automata simulation using FastFlow's parallel for.
    */
    void fastFlowParallelFor();

    /**
     Method used to get all the neighbours of the cell (X,Y)
     @param x row-index of the cell
     @param y column-index of the cell
     @param grid_ reference to the grid from which the neighbourhood will be computed
     @returns a std::vector<int> containing: the current state of the cell at index [0], then, starting from the top left corner, all the other neighbours in a clockwise sense in the following index. 
    */
    std::vector<int> getNeighbourhood(int x, int y, grid2D *grid_);

    /**
     Method used to get a deep copy of the grid
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

    /*
     The following code refers to another FastFlow implementation in which an explicit declaration of the farm components is made.
    */

    //First stage of the farm, the emitter node,

    struct firstStage : ff_node_t<int, PAIR>
    {
        int status = 0, delta, exceeded, t = 0; /**<Parameter */
        CellularAutomataff *automata;           /**<Variable representing the automata*/
        std::vector<PAIR> pairs;                /**<Vector of Pairs defining a static division of the work*/

        /**
         * struct Constructor. 
         * @param obj Cellular Automata where the simulation will be run
         *       
        */

        firstStage(CellularAutomataff *obj)
        {
            {
                automata = obj;
                delta = automata->num_rows / automata->num_threads;
                exceeded = automata->num_rows % automata->num_threads;
            }
        }

        /**
         Method executed once each time the associated thread is started
         More details on the FastFlow doc.
        */

        int svc_init()
        {
            PAIR tmp;
            int pad = 0;
            for (int i = 0; i < automata->num_threads; i++)
            {
                tmp.start = i * delta + pad;
                tmp.end = (i + 1) * delta + pad;
                if (exceeded != 0)
                {
                    pad++;
                    tmp.end++;
                    exceeded--;
                }
                pairs.insert(pairs.begin() + i, tmp);
            }
            return (1);
        }

        /**
         Function executing the Emitter job.
         The emitter sends out pairs indicating the intervals of execution of the workers.
         each time a worker comes back the status is increased and when all of them ended their execution
         a new timestep is started of the execution is ended.
        */

        PAIR *svc(int *feedbacks)
        {
            if (status == (automata->num_threads - 1))
            {

                t++;
                status = 0;
                automata->old_grid = new grid2D(*(automata->grid));
            }
            if (status == 0)
            {
                for (int i = 0; i < automata->num_threads; i++)
                {
                    ff_send_out(&(pairs[i]));
                }
            }
            status++;

            if (t == automata->timesteps)
                return EOS;
            else
                return GO_ON;
        }
    };

    //Second stage of the farm, the worker nodes
    struct secondStage : ff_node_t<PAIR, int>
    {
        CellularAutomataff *automata; /**<Cellular Automata where the simulation is run */

        /**
         Constructor
         @param ca reference to the cellular automata
         */
        secondStage(CellularAutomataff *ca)
        {
            automata = ca;
        }

        /**
         Workers's job function. Each time a worker receives a pair it starts computing its part of the grid.
        */
        int *svc(PAIR *pairs)
        {

            int columns = automata->num_columns;

            for (int i = pairs->start; i < pairs->end; i++)
            {

                for (int j = 0; j < columns; j++)
                {
                    (*(automata->grid))[i][j] = automata->rule(automata->getNeighbourhood(i, j, automata->old_grid));
                }
            }
            return (new int(1));
        }
    };

    /**
     Function to declare the nodes, build the farm and run it.
     Both Emitter and worker are initialized with the current CellularAutomata.
     Workers are replicated using a std::vector.
    */
    int startFarm()
    {
        utimer farmTime("Fastflow farm time:");
        firstStage emitter(this);
        std::vector<std::unique_ptr<ff_node>> Workers;
        for (int i = 0; i < num_threads; i++)
            Workers.push_back(make_unique<secondStage>(this));
        ff_Farm<float> farm(std::move(Workers), emitter);
        farm.remove_collector(); //This is removed in order to have one more free thread.
        farm.wrap_around();      //Creates a channel between the workers and the emitter
        if (farm.run_and_wait_end() < 0)
        {
            error("running farm");
            return -1;
        }
        return 0;
    }
    /**
     Method used to re-initialize the grid
    */
    void restartGrid();


    //Setter and getter
    int getNumThreads();
    int getRows();
    int getColumns();
    int getTimeSteps();
    grid2D *getGrid();
    void setRows(int rows);
    void setColumns(int columns);
    void setGrid(grid2D *new_grid);
    void setNumThreads(int threads);
    void setRule(int (*func)(neighbourhood));
};

#endif