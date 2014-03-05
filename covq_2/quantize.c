#include "quantize.h"

int init_quantizer(uniform_quantizer *q, int nbins, double min, double max){

    q->nbins = nbins;
    q->npoints = 0;
    q->min = min;
    q->max = max;
    q->bins = (int*) calloc(nbins, sizeof(int));

    
    if(q->bins == NULL){
        fprintf(stderr, "Could not allocate memory for uniform quantizer.\n");
        return 0;
    }

    return 1;
}

void destroy_quantizer(uniform_quantizer *q){
    free(q->bins);
    q->nbins = 0;
    q->npoints = 0;
    q->min = 0;
    q->max = 0;
    q->bins = NULL;
}

int val_to_quant(double val, uniform_quantizer *q){
    double normalized_val;
    int bin;
    double len = q->max - q->min;

    normalized_val = val - q->min; // 0 to q_max - q_min
    normalized_val = normalized_val / len; // 0 to 1
    normalized_val = q->nbins * normalized_val; // 0 to q->levels
    bin = (int) normalized_val; // 0 to q->levels - 1

    if( bin < 0 )
        return 0;
    else if(bin > q->nbins - 1)
        return q->nbins - 1;
    else
        return bin;
}

double quant_to_val(int qval, uniform_quantizer *q){
    double val;
    double len = q->max - q->min;
    
    val = (qval + 0.5) / q->nbins;
    val = len * val;
    val = val + q->min;

    return val;
}

int bin_val(double val, uniform_quantizer *q){
    int qval;

    qval = val_to_quant(val, q);
    q->bins[qval]++;
    q->npoints++;
    return 1;
}
