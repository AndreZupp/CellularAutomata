#include "cellularautomataff.hpp"
#include "rules.hpp"
int main(){
    
    CellularAutomataff ca(10000,10000,gameOfLifeRule,10, 2, 2);
    std::cout << "2 Threads" << std::endl;
    ca.fastFlowParallelFor();
    ca.restartGrid();
    ca.setNumThreads(4);
    std::cout << "4 Threads:" << std::endl;
    ca.fastFlowParallelFor();
    ca.restartGrid();
    ca.setNumThreads(8);
    std::cout << "8 Threads:" << std::endl;
    ca.fastFlowParallelFor();
    ca.restartGrid();
    ca.setNumThreads(10);
    std::cout << " 12 Threads "<< std::endl;
    ca.fastFlowParallelFor();
    
    return(1);
}