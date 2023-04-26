#ifndef RULES_H
#define RULES_H
#define NEIGHBOURHOOD_DIMENSION 8
#include <vector>
using neighbourhood = std::vector<int>;
using grid2D = std::vector<std::vector<int>>;

int countDifferent(neighbourhood elems, int k);
int countNonZero(neighbourhood elems);
int brianbrain(neighbourhood nb);
int gameOfLifeRule(neighbourhood nh);


#endif