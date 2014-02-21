#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

// Includes
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow, exp */

// IO
#define IO_DELIM "\t"
// Source Indicators
#define SRC_X 0
#define SRC_Y 1

// Transition Probabilities for BSC
#define TRANS_PROB_X 0.2
#define TRANS_PROB_Y 0.2

// Uniform Quantizer Parameters
#define Q_LEVELS 25
#define Q_LENGTH_X 1
#define Q_LENGTH_Y 1

// Length of binary codewords (in bits), should be less than 8
#define CODEWORD_LEN_X 3
#define CODEWORD_LEN_Y 5

#define CODEBOOK_SIZE_X 1 << CODEWORD_LEN_X
#define CODEBOOK_SIZE_Y 1 << CODEWORD_LEN_Y

// Maximum codebook size
#if CODEBOOK_SIZE_X > CODEBOOK_SIZE_Y 
#define CODEBOOK_SIZE_MAX CODEBOOK_SIZE_X
#else
#define CODEBOOK_SIZE_MAX CODEBOOK_SIZE_Y
#endif

// Typedefs
typedef int quant[Q_LEVELS][Q_LEVELS];
typedef double codevectors[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
typedef int prob_ij[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];

// Input Global Variables
extern int trset_size;
extern double *trset_x, *trset_y;

// Output Global Variables
extern quant q_trset;
extern int encoder_x[Q_LEVELS];
extern int encoder_y[Q_LEVELS];
extern codevectors cv_x;
extern codevectors cv_y; 
extern int bin_cw_x[CODEBOOK_SIZE_X];
extern int bin_cw_y[CODEBOOK_SIZE_Y];

// Simulated Annealing (anneal.c)
void anneal();

// Printing and IO (io.c)
void print_int_array(FILE *stream, int *arr, int len);
void print_double_array(FILE *stream, double *arr, int len);
void print_int_array_2d(FILE *stream, int *arr, int n, int m);
void print_double_array_2d(FILE *stream, double *arr, int n, int m);
void read_csv_trset(FILE *stream);

// Quantization (quantize.c)
int vec_to_quant(double x, int *outlier, int src);
double quant_to_vec(int x, int src);
int quantize();

// COVQ (covq.c)
double channel_prob(int i, int j, int k, int l);
double nearest_neighbour(int q_lvl, int *index, int src);
double nn_update();
void centroid_update(int src);
int bsc_2_source_covq();

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

