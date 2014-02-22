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

// IO
#define IO_DELIM ", "

// Source Indicators
#define SRC_X 0
#define SRC_Y 1

// Transition Probabilities for BSC
#define TRANS_PROB_X 0.2
#define TRANS_PROB_Y 0.2

// Uniform Quantizer Parameters
#define Q_LEVELS 25
#define Q_LENGTH_X 3
#define Q_LENGTH_Y 3

// Length of binary codewords (in bits), should be less than 8
#define CODEWORD_LEN_X 3
#define CODEWORD_LEN_Y 4

#define CODEBOOK_SIZE_X (1 << CODEWORD_LEN_X)
#define CODEBOOK_SIZE_Y (1 << CODEWORD_LEN_Y)

// Typedefs
typedef int quant_lvls[Q_LEVELS][Q_LEVELS];
typedef double codevectors[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
typedef int encoder[Q_LEVELS];
typedef int codewords_x[CODEBOOK_SIZE_X];
typedef int codewords_y[CODEBOOK_SIZE_Y];

// Output Global Variables
extern quant_lvls q_trset;
extern encoder encoder_x;
extern encoder encoder_y;
extern codevectors cv_x;
extern codevectors cv_y; 
extern codewords_x bin_cw_x;
extern codewords_y bin_cw_y;

// Input Global Variables
extern int trset_size;
extern double *trset_x, *trset_y;

// Simulated Annealing (anneal.c)
void anneal();
double eucl_dist(int i, int j, int k, int el);

// Printing and IO (io.c)
void print_int(FILE *stream, int *arr, int rows, int cols);
void print_double(FILE *stream, double *arr, int rows, int cols);
int csvwrite_int(char *filename, int *arr, int rows, int cols);
int csvwrite_double(char *filename, double *arr, int rows, int cols);
void read_csv_trset(FILE *stream);

// Quantization (quantize.c)
int vec_to_quant(double x, int *outlier, int src);
double quant_to_vec(int x, int src);
int quantize();

// COVQ (covq.c)
double channel_prob(int i, int j, int k, int l);
double nearest_neighbour(int q_lvl, int *index, int src, int init);
double nn_update();
void centroid_update(int src);
int bsc_2_source_covq();

// Running (running.c)
void assert_globals(void);
void init(void);
void run(void);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

