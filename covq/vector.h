#ifndef VECTOR_H
#define VECTOR_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include "vector.h"

#define VECTOR_DIM 2

typedef struct{
    size_t size;
    double (*v)[VECTOR_DIM];
} vectorset;

vectorset *init_vectorset(size_t size);
void destroy_vectorset(vectorset *v);
void print_vector(FILE *stream, double *v);
void print_vectorset(FILE *stream, vectorset *v);
int hamming_distance(int v_1, int v_2);
int vector_equals(double *v_1, double *v_2);

double dist(double *x, double *y);

#ifdef __cplusplus
}
#endif
    
#endif /* VECTOR_H */

