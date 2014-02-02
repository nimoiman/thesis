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
#define Q_LENGTH 2 // length of quantized vector components

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

typedef int (*q_vec)[Q_LEVELS];
typedef double (c_book)[FINAL_C_SIZE_X][FINAL_C_SIZE_Y];

typedef struct covq {
    int tr_size;
    int *tr_x;
    int *tr_y;

    c_book *c_x;
    c_book *c_y;

    int *enc_x; // maps training vectors to quantized levels
    int *enc_y; // e.g. a low vector might have value {0,...,0}

    int split_x;
    int split_y;

    double sigma_x;
    double sigma_y;
    double mean_x;
    double mean_y; // means and std devs of vector components
};

int bsc_2_source_covq(covq *params);
void print(FILE *stream, int thing);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_2_S_H */

