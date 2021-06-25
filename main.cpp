#include "CellularAutomata.cpp"



int brianbrain(neighbourhood nb){
    int living_neighbour = countLiving(nb);
    if(nb[0] == 0 && living_neighbour == 2)
        return 1;
    if(nb[0] == 1)
        return 2;
    if(nb[0] == 2)
        return 0;
    return nb[0];
}




int main(){
    CellularAutomata cellular_automata = CellularAutomata(10000,10000, nullptr, 40, true);
    cellular_automata.playGameOfLife();
    cellular_automata.threadsExecution(24);
}
//3074 with O3
//

