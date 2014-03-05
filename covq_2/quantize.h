#ifndef QUANTIZE_H
#define QUANTIZE_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <stdio.h>

typedef struct{

    int nbins;
    int npoints;
    double min;
    double max;
    int *bins;

}uniform_quantizer;


int val_to_quant(double val, uniform_quantizer *q);
double quant_to_val(int qval, uniform_quantizer *q);

int init_quantizer(uniform_quantizer *q, int levels, double min, double max);
void destroy_quantizer(uniform_quantizer *q);

int bin_val(double val, uniform_quantizer *q);

#ifdef __cplusplus
}
#endif

#endif
