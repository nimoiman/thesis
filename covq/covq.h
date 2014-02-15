#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include "vector.h"
#include "channel.h"

#define LBG_EPS 0.001
#define CODE_VECTOR_DISPLACE 0.001
#define EMPIRICAL_GRANULARITY 100

vectorset *bsc_covq(vectorset *train, int n_splits);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

