#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "utimer.cpp"
#include <omp.h>
#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>
using neighbourhood = std::vector<int>;
using grid2D = std::vector<std::vector<int>>;
using namespace ff;
int countLiving(neighbourhood elems)
{
    int living = 0;
    for (int i = 1; i < elems.size(); i++)
        if (elems[i] != 0)
            living++;
    return living;
}

int gameOfLifeRule(neighbourhood nh)
{
    int living_neighbours = countLiving(nh);

    if (living_neighbours < 2 || living_neighbours > 3)
        return 0;

    if (living_neighbours == 3 && nh[0] == 0)
        return 1;

    return nh[0];
}

class CellularAutomata
{
private:
    int rows, columns;
    grid2D grid;
    int (*rule)(neighbourhood) = nullptr;
    int timesteps;

public:
    CellularAutomata(int rows, int columns, int (*function)(neighbourhood), int timesteps, bool random_init)
    {
        this->columns = columns;
        this->rows = rows;
        this->timesteps = timesteps;
        std::vector<int> column(columns, 0);
        std::vector<std::vector<int>> matrix(rows, column);
        grid = matrix;
        rule = function;
        if (random_init == true)
            randomFill();
        if (rows * columns < 100)
            printMatrix();
    }

    void playGameOfLife()
    {
        rule = gameOfLifeRule;
    }

    void randomFill()
    {
        srand((unsigned)time(NULL) + rand());
        for (int i = 0; i < grid.size(); i++)
            for (int j = 0; j < grid[0].size(); j++)
                grid[i][j] = rand() % 2;
    }

    void sequentialRun()
    {
        utimer tseq("tseq");
        for (int t = 0; t < timesteps; t++)
        {
            grid2D deep_copy(grid);
            for (int i = 0; i < grid.size(); i++)
            {
                for (int j = 0; j < grid[0].size(); j++)
                {
                    grid[i][j] = rule(getNeighbourhood(i, j, &deep_copy));
                }
            }
        }
    }

    void threadsExecution(int num_threads)
    {
        utimer tpar("tpar");
        grid2D deep_copy = grid2D(grid);
        int delta = rows / num_threads, exceeded = rows % num_threads;
        std::vector<std::thread> threads;
        pthread_barrier_t barrier1;
        pthread_barrier_t barrier2;
        pthread_barrier_init(&barrier1, nullptr, num_threads + 1);
        pthread_barrier_init(&barrier2, nullptr, num_threads + 1);
        for (int i = 0; i < num_threads; i++)
        {
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
            pthread_barrier_wait(&barrier1);
            deep_copy = grid2D(grid);
            //printMatrix();
            pthread_barrier_wait(&barrier2);
        }
        for (int t = 0; t < num_threads; t++)
            threads[t].join();
    }

    void exec(int a, int b, grid2D *grid_copy, pthread_barrier_t *barrier1, pthread_barrier_t *barrier2)
    {
        for (int t = 0; t < timesteps; t++)
        {
            for (int i = a; i < b; i++)
                for (int j = 0; j < columns; j++)
                {
                    grid[i][j] = rule(getNeighbourhood(i, j, grid_copy));
                }
            pthread_barrier_wait(barrier1);
            pthread_barrier_wait(barrier2);
        }
    }

    void ompParallelFor(int numthreads)
    {

        utimer my_timer("tomp");
        for (int t = 0; t < timesteps; t++)
        {
            printMatrix();
            grid2D deep_copy(grid);
#pragma omp parallel for num_threads(numthreads) collapse(2)
            for (int i = 0; i < grid.size(); i++)
            {

                for (int j = 0; j < grid[0].size(); j++)
                {
                    grid[i][j] = rule(getNeighbourhood(i, j, &deep_copy));
                }
            }
        }
    }

    std::vector<int> getNeighbourhood(int x, int y, grid2D *deep_copy)
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

    void fastFlow(int num_threads)
    {
        ParallelFor pf(12);
        int iter = columns;
        utimer tff("tff");
        grid2D previousGrid(grid), updatedGrid(grid);
        for (int t = 0; t < timesteps; t++)
        {
            printMatrix();
            pf.parallel_for(
                0, rows, 1, 0, [this, &previousGrid, &updatedGrid, iter](const long i)
                {
                    for (int j = 0; j < iter; j++)
                        updatedGrid[i][j] = rule(getNeighbourhood(i, j, &previousGrid));
                },
                num_threads);
            previousGrid = grid2D(updatedGrid);
            grid = grid2D(updatedGrid);
        }
        grid = updatedGrid;
    }

    grid2D copyGrid()
    {
        return grid2D(grid);
    }

    void printMatrix()
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

    int upperRow(int index)
    {
        if (index == 0)
            return rows - 1;
        else
            return index - 1;
    }

    int lowerRow(int index)
    {
        if (index == (rows - 1))
        {
            return 0;
        }
        else
            return index + 1;
    }

    int rightColumn(int index)
    {
        if (index == (columns - 1))
            return 0;
        else
            return index + 1;
    }

    int leftColumn(int index)
    {
        if (index == 0){
            return columns - 1;
        }
        else
            return index - 1;
    }

    grid2D getGrid() { return this->grid; }

    // Integer Functions to get positions and fill the matrix
};
