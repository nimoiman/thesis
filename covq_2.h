#ifndef COVQ_2_S_H
#define COVQ_2_S_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h> /* sprintf */
// #include <float.h> /* DBL_MAX */
#include <stdlib.h> /* malloc, free, size_t */

#define X 0
#define Y 1

#define TRANS_PROB_X 0.2
#define TRANS_PROB_Y 0.2

#define LBG_EPS 0.001
#define CODE_VECTOR_DISPLACE 0.001

#define Q_LEVELS_X 500 // number of quantized levels
#define Q_LEVELS_Y 500
#define Q_LENGTH 2 // length of quantized vector components

#define SPLITS_X 3
#define SPLITS_Y 5

#define FINAL_C_SIZE_X (1 << SPLITS_X)
#define FINAL_C_SIZE_Y (1 << SPLITS_Y)

#define DIM_X 2
#define DIM_Y 2

typedef double (*vec_x)[DIM_X];
typedef double (*vec_y)[DIM_Y];
typedef int (q_vec)[Q_LEVELS_X][Q_LEVELS_Y];
typedef double (c_book_x)[C_SIZE_X][C_SIZE_Y][DIM_X];
typedef double (c_book_y)[C_SIZE_X][C_SIZE_Y][DIM_Y];

typedef struct covq {
    int tr_size;
    vec_x *tr_x;
    vec_y *tr_y;

    c_book_x *c_x;
    c_book_y *c_y;

    int *enc_x;
    int *enc_y;

    q_vec *q_tr;
};

int bsc_2_source_covq(covq *everything);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_2_S_H */

