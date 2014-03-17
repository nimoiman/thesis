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
    uint dim;
    double **v;
} vectorset;

vectorset *init_vectorset(size_t size, uint dim);
void destroy_vectorset(vectorset *v);

inline uint hamming_distance(uint v_1, uint v_2) {
    return __builtin_popcount(v_1 ^ v_2);
}

int vector_equals(double *v_1, double *v_2, uint dim);

double dist(double *x, double *y, uint dim);

#ifdef __cplusplus
}
#endif
    
#endif /* VECTOR_H */

