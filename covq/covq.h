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
struct retval {
    double theor_distortion;
    double empir_distortion;
    double snr;
    double sqnr;
    double psnr;
    
    vectorset *test_out;
    vectorset *test_in;
    vectorset *codebook;
    uint *cw_map;
};


struct cmdline {
    int train;
    char *train_csv;
    char *codebook_out;
    char *cw_map_out;
    double lbg_eps;
    double codevector_displace;

    int test;
    char *test_in_csv;
    char *codebook_in;
    char *cw_map_in;
    char *test_out_csv;

    int seed;
    uint dim;
    uint nsplits;
    double bep;
};

void free_retval(struct retval *rv, struct cmdline *opts);

double bsc_covq(vectorset *train, vectorset *codebook, uint *cw_map,
                uint n_splits, double error_prob, double lbg_eps,
                double code_vector_displace);

void run(struct cmdline *opts, struct retval *rv);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

