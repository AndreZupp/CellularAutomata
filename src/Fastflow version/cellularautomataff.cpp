#include "cellularautomataff.hpp"

/**
    @brief Class and methods body of the cellularautomataff.hpp file.
    Here there's the pure body of the methods declared in the c++ header.
    For more detail about what the function does, please, consult the cellularautomataff.hpp file.
    Here will be provided, where necessary, details about how the work it's done.
    @file cellularAutomataff.cpp
    @author Andrea Zuppolini
    @version 1 29/06/2021
*/

CellularAutomataff::CellularAutomataff(int rows, int columns, int (*function)(neighbourhood), int tsteps, int n_states, int numthreads)
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
    grid = new grid2D(matrix);

    randomFill();
    //Grid copy used to compute next state
    old_grid = new grid2D(*grid);
}

CellularAutomataff::CellularAutomataff(int rows, int columns, int (*function)(neighbourhood), int tsteps, grid2D *initial_state, int numthreads)
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

void CellularAutomataff::fastFlowParallelFor()
{
    //The commented lines were used to generate results in the reports
    //std::string message = "FastFlow execution with" + (std::to_string(num_threads)) + " Threads";
    //timer is started
    utimer tff("Fastflow parallel for time:");
    ParallelFor pf(num_threads);
    int iter = num_columns;
    //Deep copies of the grid created to work indipendently from the object
    grid2D previousGrid(*(grid)), updatedGrid(*(grid));
    for (int t = 0; t < timesteps; t++)
    {
        //Static division of the job between the workers
        pf.parallel_for(
            0, num_rows, 1, 1, [this, iter, &updatedGrid, &previousGrid](const long i)
            {
                for (int j = 0; j < iter; j++)
                    updatedGrid[i][j] = rule(getNeighbourhood(i, j, &previousGrid));
            },
            num_threads);
        previousGrid = grid2D(updatedGrid);
    }
    grid = new grid2D(updatedGrid);
    //tff.printOnReport();
}

grid2D *CellularAutomataff::getGrid()
{
    return grid;
}

grid2D CellularAutomataff::copyGrid()
{
    return grid2D(*(grid));
}

void CellularAutomataff::printMatrix()
{
    for (int i = 0; i < (*grid).size(); i++)
    {
        for (int j = 0; j < (*grid)[0].size(); j++)
            std::cout << (*grid)[i][j] << '\t';
        std::cout << '\n';
    }
    std::cout << "\n \n \n";
    std::cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << std::endl;
}

int CellularAutomataff::upperRow(int index)
{
    if (index == 0)
        return num_rows - 1;
    else
        return index - 1;
}

int CellularAutomataff::lowerRow(int index)
{
    if (index == (num_rows - 1))
    {
        return 0;
    }
    else
        return index + 1;
}

int CellularAutomataff::rightColumn(int index)
{
    if (index == (num_columns - 1))
        return 0;
    else
        return index + 1;
}

int CellularAutomataff::leftColumn(int index)
{
    if (index == 0)
    {
        return num_columns - 1;
    }
    else
        return index - 1;
}

std::vector<int> CellularAutomataff::getNeighbourhood(int x, int y, grid2D *deep_copy)
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

bool CellularAutomataff::checkParameters(int rows, int columns, int (*function)(neighbourhood), int tsteps, int numthreads)
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

//PSEUDO randomFill
void CellularAutomataff::randomFill()
{

    srand((unsigned)time(NULL) + rand());
    for (int i = 0; i < (*grid).size(); i++)
        for (int j = 0; j < (*grid)[0].size(); j++)

            (*grid)[i][j] = rand() % states;
}

void CellularAutomataff::restartGrid()
{
    randomFill();
}


int CellularAutomataff::getNumThreads(){return num_threads;}
int CellularAutomataff::getColumns(){return num_columns;}
int CellularAutomataff::getRows(){return num_rows;}
int CellularAutomataff::getTimeSteps(){return timesteps;}
void CellularAutomataff::setNumThreads(int threads){num_threads=threads;}
void CellularAutomataff::setColumns(int columns){num_columns=columns;}
void CellularAutomataff::setRows(int rows){num_rows=rows;}
void CellularAutomataff::setGrid(grid2D *new_grid){grid=new_grid; setRows((*new_grid).size()); setColumns((*new_grid)[0].size());}
void CellularAutomataff::setRule(int(*func)(neighbourhood)){rule=func;}
