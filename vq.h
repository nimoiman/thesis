#ifndef VQ_H
#define VQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* printf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */

#define VECTOR_DIM 2
#define LBG_EPS 0.01
#define CODE_VECTOR_DISPLACE 0.0001

typedef struct{
	size_t size;
	double (*v)[VECTOR_DIM];
} vectorset;

vectorset *init_vectorset(size_t size);
void destroy_vectorset(vectorset *v);
void print_vectorset(vectorset *v);

double dist(double *x, double *y);
double nearest_neighbour(double *x, vectorset *c, int *idx);
double centroid_update(vectorset *train, vectorset *c_old, vectorset *c_new, int *count);
vectorset *lbgvq(vectorset *train, int nsplits);

#ifdef __cplusplus
}
#endif

#endif /* VQ_H */

