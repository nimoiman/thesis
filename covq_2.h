#ifndef COVQ_2_S_H
#define COVQ_2_S_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
#include <stdlib.h> /* malloc, free, size_t */

#define SRC_X 0
#define SRC_Y 1

#define TRANS_PROB_X 0.2
#define TRANS_PROB_Y 0.2

#define LBG_EPS 0.001
#define CODE_VECTOR_DISPLACE 0.001

#define Q_LEVELS 25
#define Q_LEVELS_X 25
#define Q_LEVELS_Y 25

#define DIM_X 1
#define DIM_Y 1

#define SPLITS_X 3
#define SPLITS_Y 5
#define C_SIZE_X 3
#define C_SIZE_Y 5

#define FINAL_C_SIZE_X (1 << SPLITS_X)
#define FINAL_C_SIZE_Y (1 << SPLITS_Y)

#if SPLITS_X > SPLITS_Y
#define C_SIZE_MAX (1 << SPLITS_X)
#else
#define C_SIZE_MAX (1 << SPLITS_Y)
#endif

typedef int q_vec[Q_LEVELS_X][Q_LEVELS_Y];
typedef double c_book[FINAL_C_SIZE_X][FINAL_C_SIZE_Y];

typedef struct covq {
    int tr_size;
    double *tr_x;
    double *tr_y;

    c_book c_x;
    c_book c_y;

    // maps quantized levels to codebook scalars
    int *enc_x;
    int *enc_y;

    // holds the number of the current split in the LBGVQ iteration
    // i.e. the size of the codebooks are (1 << split_x) and (1 << split_y)
    // respectively.
    int split_x;
    int split_y;

    // holds the bin counts for each quantization level - for now, there
    // are exactly Q_LEVELS_X*Q_LEVELS_Y joint quantization levels,
    // i.e. X and Y are scalar valued
    q_vec q_tr;

    // means and std devs of vector components
    double sigma_x[DIM_X];
    double sigma_y[DIM_Y];
    double mean_x[DIM_X];
    double mean_y[DIM_Y];
} covq;

int bsc_2_source_covq(struct covq *params);
void print(FILE *stream, int thing);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_2_S_H */

