#ifndef VQ_H
#define VQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* printf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow */

#define VECTOR_DIM 5
#define LBG_EPS 0.01
#define CODE_VECTOR_DISPLACE 0.0001

typedef struct _vectorset vectorset;

int init_vectorset(vectorset *vset, size_t size);
void destroy_vectorset(vectorset *v);
void print_vectorset(vectorset *v);

double dist(double *x, double *y);
double nearest_neighbour(double *x, vectorset *c, int *idx);
double centroid_update(vectorset *train, vectorset **c1, vectorset **c2, int *count);
vectorset *lbgvq(vectorset *v, int nsplits);

#ifdef __cplusplus
}
#endif

#endif /* VQ_H */

