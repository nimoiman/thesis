#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include "vector.h"

typedef struct{
    size_t size;
    int dim;
    double **v;
} vectorset;

vectorset *init_vectorset(size_t size, int dim);
void destroy_vectorset(vectorset *v);
int hamming_distance(int v_1, int v_2);
int vector_equals(double *v_1, double *v_2, int dim);

double dist(double *x, double *y, int dim);

#ifdef __cplusplus
}
#endif
    
#endif /* VECTOR_H */

