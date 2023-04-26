#include "rules.hpp"
#include "cellularautomata.hpp"


int main(){
   CellularAutomata ca(10000,10000, gameOfLifeRule, 40, 2, 2);

   std::cout << "2 Threads:" << std::endl;
   ca.sequentialRun();
   ca.restartGrid();
   ca.threadsExecution();
   ca.restartGrid();

   ca.setNumThreads(4);
   std::cout << "4 Threads:" << std::endl;
   ca.threadsExecution();
   ca.restartGrid();

   ca.setNumThreads(8);
   std::cout << "8 Threads:" << std::endl;
   ca.threadsExecution();
   ca.restartGrid();

   ca.setNumThreads(11);
   std::cout << "8 Threads:" << std::endl;
   ca.threadsExecution();
   ca.restartGrid();
   }
