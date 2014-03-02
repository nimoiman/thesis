#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#define POW2(x) ((x)*(x))    
#define IN_RANGE(x, low, high) ((low) <= (x) && (x) <= (high))

#define CODEBOOK_SIZE_X (1 << p->cwlen_x)
#define CODEBOOK_SIZE_Y (1 << p->cwlen_y)
#define CV_IDX(i,j) (j * CODEBOOK_SIZE_X + i)
#define TS_IDX(i,j) (j * p->qlvls * i)

// Includes
#include <assert.h>
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow, exp */

// Source Indicators
#define SRC_X 0
#define SRC_Y 1

// Codeword length should not exceed this value
#define MAX_CODEWORD_LEN 4
#define MAX_CODEBOOK_SIZE (1 << MAX_CODEWORD_LEN)
#define TRSET_SIZE_MAX 10000
typedef struct{

    /*
     * Uniform Quantizer Parameters
     * The number of uniform quantization levels is determined by qlvls. The
     * length of the entire quantization interval is given by qlen_x for the
     * x-source and qlen_y for the y-source. The width of each individual
     * quantization interval is therefore qlen_x/qlvls for the x-source and
     * qlen_y/qlvls for the y-source.
     *
     */
    unsigned int qlvls;
    double qlen_x, qlen_y;

    /*
     * Encoder Parameters
     * 
     */
    unsigned char cwlen_x, cwlen_y;

    /*
     * Training Set
     *
     */
    double *trset_x;
    double *trset_y;
    unsigned int trset_size;

    /*
     * Channel Transition Probability Function
     *
     */
    double (*transition_prob)(int i, int j, int k, int l);
} params_covq2;

typedef struct{

    int *qtrset;
    int *encoder_x, *encoder_y;
    double *codevec_x, *codevec_y;
    int *cwmap_x, *cwmap_y;
} covq2;

typedef struct{
    double *simset_x;
    double *simset_y;
    int simset_size;
    void (*channel_sim)(int i, int j, int *k, int *l);
} sim_covq2;
// Simulated Annealing (anneal.c)
void anneal();

/*
 * util.c
 *
 */
int vec_to_quant(double x, int *outlier, int src, params_covq2 *p);
double quant_to_vec(int qlvl, int src, params_covq2 *p);
void destroy_covq2_struct(covq2 *c);
void print_double(FILE *stream, double *arr, int rows, int cols);
int fprintf_int(char *filename, int *arr, int rows, int cols);
int fprintf_double(char *filename, double *arr, int rows, int cols);
void assert_globals(covq2 *c, params_covq2 *p);
int run(covq2 *c, double *d, params_covq2 *p);

// COVQ (covq.c)
double nn_update(int init, covq2 *c, params_covq2 *p);
double nearest_neighbour(int qlvl1, int *idx, int init, int src, covq2 *c, params_covq2 *p);
void centroid_update(int src, covq2 *c, params_covq2 *p);


#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

