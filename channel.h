#include <math.h> /* log, sqrt */
#include <stdlib.h> /* rand, srand, RAND_MAX */

#define PI 3.14159

double drand();
void box_muller(vectorset *vectors, double mean, double var);
