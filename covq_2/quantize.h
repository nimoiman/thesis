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

typedef struct{

    int nbins_x, nbins_y;
    double min_x, min_y;
    double max_x, max_y;
    int npoints;
    int *quant;

}uniform_quantizer;


int quantizer_init(uniform_quantizer *q, int nbins_x, int nbins_y,
        double min_x, double max_x, double min_y, double max_y);
void quantizer_destroy(uniform_quantizer *q);

int quantizer_bin(double val_x, double val_y, uniform_quantizer *q);
int quantizer_get_count(double val_x, double val_y, uniform_quantizer *q);

int quantizer_sum_marg(double val, uniform_quantizer *q);
int quantizer_count_marg(double val, uniform_quantizer *q);

void quantizer_test();

#ifdef __cplusplus
}
#endif

#endif
