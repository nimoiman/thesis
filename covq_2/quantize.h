#ifndef QUANTIZE_H
#define QUANTIZE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <stdio.h>

/* Includes for testing */
#include <assert.h>
#include <math.h>

typedef struct
{
    int L_X, L_Y;
    double min_x, min_y;
    double max_x, max_y;
    int npoints;
    int *quant;

}unif_quant;

enum source
{
    src_X,
    src_Y
};

int quantizer_init
(unif_quant *q, int L_X, int L_Y,
 double min_x, double max_x, double min_y, double max_y);
void quantizer_free(unif_quant *q);

double quant_to_val(int qval, enum source s, unif_quant *q);
int val_to_quant(double val, enum source s, unif_quant *q);

int quantizer_bin(int qx, int qy, unif_quant *q);
int quantizer_get_count(int qx, int qy, unif_quant *q);

void quantizer_test();

#ifdef __cplusplus
}
#endif

#endif
