#ifndef VQ_H
#define VQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include "vector.h"

#define LBG_EPS 0.01
#define CODE_VECTOR_DISPLACE 0.0001

double nearest_neighbour(double *x, vectorset *c, int *idx);
double centroid_update(vectorset *train, vectorset *c_old, vectorset *c_new, int *count);
vectorset *lbgvq(vectorset *train, int nsplits);

#ifdef __cplusplus
}
#endif

#endif /* VQ_H */

