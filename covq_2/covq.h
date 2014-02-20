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
#define Q_LENGTH_X 1
#define Q_LENGTH_Y 1

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
typedef int prob_ij[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];

extern trset_size;
double *trset_x, *trset_y;
extern quant q_trset;
extern int encoder_x[Q_LEVELS];
extern int encoder_y[Q_LEVELS];
extern codevectors cv_x;
extern codevectors cv_y; 
extern int bin_cw_x[CODE_BOOK_SIZE_X];
extern int bin_cw_y[CODE_BOOK_SIZE_Y];

int vec_to_quant(double x, int *outlier, int src);
double quant_to_vec(int x, int src);
int quantize();
int bsc_2_source_covq(struct covq *params);
void print(FILE *stream, int thing);

#ifdef __cplusplus
}
#endif

#endif /* COVQ_2_S_H */

