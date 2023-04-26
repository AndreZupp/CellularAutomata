#include "rules.hpp"
int countDifferent(neighbourhood elems, int k){
    int different = 0;
    for (int i = 1; i < elems.size(); i++)
        if (elems[i] != k)
            different++;
    return different;
}

int countNonZero(neighbourhood elems)
{
    return countDifferent(elems, 0);
}


int brianbrain(neighbourhood nb){
    int living_neighbour = countDifferent(nb,0);
    if(nb[0] == 0 && living_neighbour == 2)
        return 1;
    if(nb[0] == 1)
        return 2;
    if(nb[0] == 2)
        return 0;
    return nb[0];
}


int gameOfLifeRule(neighbourhood nh)
{
    int living_neighbours = countDifferent(nh,0);

    if (living_neighbours < 2 || living_neighbours > 3)
        return 0;

    if (living_neighbours == 3 && nh[0] == 0)
        return 1;

    return nh[0];
}






