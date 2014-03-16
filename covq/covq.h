#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* log2 */
#include <string.h> /* atoi, atof */
#include "vector.h" /* dist */
#include "channel.h" /* channel_prob */
#include "anneal.h" /* anneal */
#include "io.h" /* get_num_lines, print_vector, print_vectorset,
                 * get_next_csv_record */

vectorset *bsc_covq(vectorset *train, uint *cw_map, uint n_splits,
                    double error_prob, double lbg_eps,
                    double code_vector_displace);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

