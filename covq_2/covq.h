#ifndef COVQ_2_S_H
#define COVQ_2_S_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */
#include <math.h> /* pow, exp */

#define SRC_X 0
#define SRC_Y 1

#define TRANS_PROB_X 0.2
#define TRANS_PROB_Y 0.2

#define Q_LEVELS 25

#define CODEWORD_LEN_X 3
#define CODEWORD_LEN_Y 5

#define CODEBOOK_SIZE_X 1 << CODEWORD_LEN_X
#define CODEBOOK_SIZE_Y 1 << CODEWORD_LEN_Y

#if CODEBOOK_SIZE_X > CODEBOOK_SIZE_Y 
#define CODEBOOK_SIZE_MAX CODEBOOK_SIZE_X
#else
#define CODEBOOK_SIZE_MAX CODEBOOK_SIZE_Y
#endif

typedef int quant[Q_LEVELS][Q_LEVELS];
typedef double codevectors[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];

typedef struct covq {
    int tr_size;
    double *tr_x;
    double *tr_y;

    c_book c_x;
    c_book c_y;

    // maps quantized levels to codebook scalars
    int *enc_x;
    int *enc_y;

    // holds the bin counts for each quantization level - for now, there
    // are exactly Q_LEVELS_X*Q_LEVELS_Y joint quantization levels,
    // i.e. X and Y are scalar valued
    quant q_tr;

    // means and std devs of vector components
    double sigma_x[DIM_X];
    double sigma_y[DIM_Y];
    double mean_x[DIM_X];
    double mean_y[DIM_Y];
} covq;

extern quant q_trset;
extern int encoder_x[Q_LEVELS];
extern int encoder_y[Q_LEVELS];
extern codevectors cv_x;
extern codevectors cv_y; 
extern int bin_cw_x[CODE_BOOK_SIZE_X];
extern int bin_cw_y[CODE_BOOK_SIZE_Y];

int bsc_2_source_covq(struct covq *params);
void print(FILE *stream, int thing);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_2_S_H */

