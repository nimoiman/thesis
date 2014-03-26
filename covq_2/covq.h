#ifndef COVQ_H
#define COVQ_H

#ifdef __cplusplus
extern "C"{
#endif

#define POW2(x) ((x)*(x))    

#define CI(i,j) (j * v->N_X + i)

// Includes
#include <assert.h>
#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow, exp */
#include "quantize.h"

// Codeword length should not exceed this value
#define MAX_CODEWORD_LEN 7
#define MAX_CODEBOOK_SIZE (1 << MAX_CODEWORD_LEN)

typedef struct{

    // Number of codevectors
    // Should be a power of 2
    int N_X, N_Y;

    // Encoder mappings
    int *I_X, *I_Y;

    // Codevectors
    double *x_ij, *y_ij;

    // Encoder index to transmission index
    int *b_X, *b_Y;

    // Training set
    unif_quant *q;

    // Channel
    double (*trans_prob)(int i, int j, int k, int l, int *b_X, int *b_Y);

} covq2;

/*
 * anneal.c
 */
void anneal(covq2 *v);

/*
 * init.c
 */
int malloc_covq2(covq2 *v, int N_X, int N_Y, int L_X, int L_Y);
void free_covq2(covq2 *v);

/*
 * covq.c
 */
double update1(covq2 *v);
double update2(covq2 *v);
double nearest_neighbour1_x(int qx, int *idx, covq2 *v);
double nearest_neighbour1_y(int qy, int *idx, covq2 *v);
double nearest_neighbour2_x(int qx, int *idx, covq2 *v);
double nearest_neighbour2_y(int qy, int *idx, covq2 *v);

/*
 * util.c
 */
void print_double(FILE *stream, double *arr, int rows, int cols);
void print_int(FILE *stream, int *arr, int rows, int cols);
int fprintf_int(char *filename, int *arr, int rows, int cols);
int fprintf_double(char *filename, double *arr, int rows, int cols);

/*
 * run.c
 */
double initilization_stage_covq2(covq2 *v, unif_quant *q, int N_X_final, int N_Y_final);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_H */

