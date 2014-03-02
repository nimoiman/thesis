#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#define POW2(x) ((x)*(x))    
#define IN_RANGE(x, low, high) ((low) <= (x) && (x) <= (high))

// Includes
#include <assert.h>
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow, exp */

// Source Indicators
#define SRC_X 0
#define SRC_Y 1

// Simulated Annealing Constants
#define TEMP_INIT 10.0
#define COOLING_RATE 0.8
#define TEMP_FINAL 0.00025
#define PHI 5 // energy drops until temperature drop
#define PSI 200 // rejected swaps until temperature drop

// Codeword length should not exceed this value
#define MAX_CODEWORD_LEN 8
#define MAX_CODEBOOK_SIZE (1 << MAX_CODEWORD_LEN)

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

// Simulated Annealing (anneal.c)
void anneal();

// Quantization (quantize.c)
int vec_to_quant(double x, int *outlier, int src);
double quant_to_vec(int x, int src);
int quantize(FILE *stream);

// COVQ (covq.c)
double nearest_neighbour(int qlvl1, int *idx, int init, int src, covq2 *c, params_covq2 *p);
void centroid_update(int src, covq2 *c, params_covq2 *p);
double channel_prob(int i, int j, int k, int l);
double nn_update();
int bsc_2_source_covq();

// Running (running.c)
void init_binary_codewords(void);
void init(FILE *stream);
void assert_globals(void);
void run(void);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

