#include "cellularautomata.hpp"

/**
    @brief Class and methods body of the cellularautomata.hpp file.
    Here there's the pure body of the methods declared in the c++ header.
    For more detail about what the function does, please, consult the cellularautomata.hpp file.
    Here will be provided, where necessary, details about how the work it's done.
    @file cellularAutomata.cpp
    @author Andrea Zuppolini
    @version 1 29/06/2021
*/

CellularAutomata::CellularAutomata(int rows, int columns, int (*function)(neighbourhood), int tsteps, int n_states, int numthreads)
{
    if (checkParameters(rows, columns, function, tsteps, numthreads) == false)
        exit(-1);
    states = n_states;
    num_columns = columns;
    num_rows = rows;
    timesteps = tsteps;
    rule = function;
    num_threads = numthreads;

    //Creation of the grid
    std::vector<int> column(columns, 0);
    std::vector<std::vector<int>> matrix(rows, column);
    grid = grid2D(matrix);

    //Random initialization
    randomFill();
}

CellularAutomata::CellularAutomata(int rows, int columns, int (*function)(neighbourhood), int tsteps, grid2D initial_state, int numthreads)
{
    if (checkParameters(rows, columns, function, tsteps, numthreads) == false)
        exit(-1);
    num_columns = columns;
    num_rows = rows;
    timesteps = tsteps;
    rule = function;
    grid = initial_state;
    num_threads = numthreads;
}

//PseudoRandomFill
void CellularAutomata::randomFill()
{
    srand((unsigned)time(NULL) + rand());
    for (int i = 0; i < grid.size(); i++)
        for (int j = 0; j < grid[0].size(); j++)
            grid[i][j] = rand() % states;
}

void CellularAutomata::sequentialRun()
{
    //Starting the timer
    utimer tseq("Sequential time:");
    for (int t = 0; t < timesteps; t++)
    {
        //Copy of the actual grid's state, used to compute the next state.
        grid2D deep_copy(grid);
        for (int i = 0; i < grid.size(); i++)
            for (int j = 0; j < grid[0].size(); j++)
                //Compute the rule on the current cell
                grid[i][j] = rule(getNeighbourhood(i, j, &deep_copy));
        
    }
    //The following lines were used to generate results
    //std::ofstream myfile;
    //myfile.open("report2.txt", std::ios::app);
    //myfile << "Sequential time " << std::endl;
    //myfile.close();
    //tseq.printOnReport();
}

void CellularAutomata::threadsExecution()
{
    //The commented section of the code was used to generate results
    //std::string message = "Thread Execution with" + (std::to_string(num_threads)) + " Threads";
    utimer tpar("Thread Execution time:");
    grid2D deep_copy = grid2D(grid); //local copy of the grid
    std::vector<std::thread> threads;
    pthread_barrier_t barrier1; //barrier1 used to synchronize with other threads
    pthread_barrier_t barrier2; //barrier2 used to synchronize with main thread, which in the meanwhile is updating the grid copy
    pthread_barrier_init(&barrier1, nullptr, num_threads + 1);
    pthread_barrier_init(&barrier2, nullptr, num_threads + 1);
    int delta = num_rows / num_threads;
    int exceeded = num_rows % num_threads;
    for (int i = 0; i < num_threads; i++)
    {
        //If statement to equally give the exceeded section of the grid to the threads
        if (exceeded == 0)
            threads.insert(threads.begin() + i, std::thread(&CellularAutomata::exec, this, i * delta, (i + 1) * delta, &deep_copy, &barrier1, &barrier2));
        else
        {
            threads.insert(threads.begin() + i, std::thread(&CellularAutomata::exec, this, i * delta, (i + 1) * delta + 1, &deep_copy, &barrier1, &barrier2));
            exceeded--;
        }
    }

    for (int j = 0; j < timesteps; j++)
    {
        pthread_barrier_wait(&barrier1); //Wait the threads
        deep_copy = grid2D(grid);        //Update the copy
        pthread_barrier_wait(&barrier2); //Unlock the threads
    }
    for (int t = 0; t < num_threads; t++)
        threads[t].join(); //Join the threads
    //tpar.printOnReport();
}

void CellularAutomata::exec(int a, int b, grid2D *grid_copy, pthread_barrier_t *barrier1, pthread_barrier_t *barrier2)
{
    for (int t = 0; t < timesteps; t++)
    {
        for (int i = a; i < b; i++)
            for (int j = 0; j < num_columns; j++)
                //Computing the rule on the actual cell
                grid[i][j] = rule(getNeighbourhood(i, j, grid_copy));
                
        pthread_barrier_wait(barrier1); //Wait for the other threads
        pthread_barrier_wait(barrier2); //Wait for the main thread
    }
}

void CellularAutomata::ompParallelFor()
{
    //Commented lines of code were used to generate the results
    //std::string message = "OMP parallel For with" + (std::to_string(numthreads)) + " Threads";
    //Starting the timer
    utimer my_timer("OpenMP parallel for time:");
    for (int t = 0; t < timesteps; t++)
    {

        grid2D deep_copy(grid);                   //local copy of the grid
#pragma omp parallel for num_threads(num_threads) //Collapse the cycles
        for (int i = 0; i < grid.size(); i++)
        {

            for (int j = 0; j < grid[0].size(); j++)
            {
                //compute the rule on the actual cell
                grid[i][j] = rule(getNeighbourhood(i, j, &deep_copy));
            }
        }
    }
    //my_timer.printOnReport();
}

std::vector<int> CellularAutomata::getNeighbourhood(int x, int y, grid2D *deep_copy)
{
    std::vector<int> neighbourhood;
    int i = 0;
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[x][y]);                        //Actual state
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[upperRow(x)][leftColumn(y)]);  //upper left cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[upperRow(x)][y]);              //upper cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[upperRow(x)][rightColumn(y)]); //upper right cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[x][leftColumn(y)]);            //left cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[x][rightColumn(y)]);           // right cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[lowerRow(x)][leftColumn(y)]);  // lower left cell
    neighbourhood.insert(neighbourhood.begin() + i++, (*deep_copy)[lowerRow(x)][y]);              //lower cell
    neighbourhood.insert(neighbourhood.begin() + i, (*deep_copy)[lowerRow(x)][rightColumn(y)]);
    return neighbourhood;
}

grid2D CellularAutomata::copyGrid()
{
    return grid2D(grid);
}

void CellularAutomata::printMatrix()
{

    for (int i = 0; i < grid.size(); i++)
    {
        for (int j = 0; j < grid[i].size(); j++)
            std::cout << grid[i][j] << '\t';
        std::cout << '\n';
    }
    std::cout << "\n \n \n";
    std::cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << std::endl;
}

int CellularAutomata::upperRow(int index)
{
    if (index == 0)
        return num_rows - 1;
    else
        return index - 1;
}

int CellularAutomata::lowerRow(int index)
{
    if (index == (num_rows - 1))
    {
        return 0;
    }
    else
        return index + 1;
}

int CellularAutomata::rightColumn(int index)
{
    if (index == (num_columns - 1))
        return 0;
    else
        return index + 1;
}

int CellularAutomata::leftColumn(int index)
{
    if (index == 0)
    {
        return num_columns - 1;
    }
    else
        return index - 1;
}

grid2D CellularAutomata::getGrid() { return this->grid; }

void CellularAutomata::restartGrid()
{
    randomFill();
}

bool CellularAutomata::checkParameters(int rows, int columns, int (*function)(neighbourhood), int tsteps, int numthreads)
{
    bool flag = true;

    if (rows <= 0 || columns <= 0)
    {
        std::cerr << "Error: rows or columns value wasn't valid" << std::endl;
        flag = false;
    }

    if (tsteps <= 0)
    {
        std::cerr << "Error: timesteps values wasn't strictly positive" << std::endl;
        flag = false;
    }

    if (numthreads <= 0)
    {
        std::cerr << "Error: the number of threads wasn't strictly positive" << std::endl;
        flag = false;
    }
    if (function == nullptr)
    {
        std::cerr << "Error: rule provided wasn't valid" << std::endl;
        flag = false;
    }
    return flag;

    
}

int CellularAutomata::getNumThreads(){return num_threads;}
int CellularAutomata::getColumns(){return num_columns;}
int CellularAutomata::getRows(){return num_rows;}
int CellularAutomata::getTimeSteps(){return timesteps;}
void CellularAutomata::setNumThreads(int threads){num_threads=threads;}
void CellularAutomata::setColumns(int columns){num_columns=columns;}
void CellularAutomata::setRows(int rows){num_rows=rows;}
void CellularAutomata::setGrid(grid2D new_grid){grid=new_grid; setRows(new_grid.size()); setColumns(new_grid[0].size());}
void CellularAutomata::setRule(int(*func)(neighbourhood)){rule=func;}

// Integer Functions to get positions and fill the matrix
