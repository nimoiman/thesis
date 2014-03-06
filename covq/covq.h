#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* log2 */
#include "vector.h" /* dist */
#include "channel.h" /* channel_prob */
#include "anneal.h" /* anneal */
#include "io.h" /* get_num_lines, print_vector, print_vectorset,
                 * get_next_csv_record */

#define LBG_EPS 0.001
#define CODE_VECTOR_DISPLACE 0.01

vectorset *bsc_covq(vectorset *train, uint *cw_map, uint n_splits,
                    double error_prob);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

