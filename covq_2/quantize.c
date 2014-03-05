#include "quantize.h"

int init_quantizer(uniform_quantizer *q, int levels, double min, double max){

    q->levels = levels;
    q->num = 0;
    q->min = min;
    q->max = max;
    q->len = max - min;
    q->bins = (int*) calloc(levels, sizeof(int));

    
    if(q->bins == NULL){
        fprintf(stderr, "Could not allocate memory for uniform quantizer.\n");
        return 0;
    }

    return 1;
}

void destroy_quantizer(uniform_quantizer *q){
    free(q->bins);
    q->levels = 0;
    q->num = 0;
    q->min = 0;
    q->max = 0;
    q->len = 0;
    q->bins = NULL;
}

int val_to_quant(double val, uniform_quantizer *q){
    double normalized_val;
    int bin;

    normalized_val = val - q->min; // 0 to q_max - q_min
    normalized_val = normalized_val / q->len; // 0 to 1
    normalized_val = q->levels * normalized_val; // 0 to q->levels
    bin = (int) normalized_val; // 0 to q->levels - 1

    if( bin < 0 )
        return 0;
    else if(bin > q->levels - 1)
        return q->levels - 1;
    else
        return bin;
}

double quant_to_val(int qval, uniform_quantizer *q){
    double val;
    
    val = (qval + 0.5) / q->levels;
    val = q->len * val;
    val = val + q->min;

    return val;
}

int bin_val(double val, uniform_quantizer *q){
    int qval;

    qval = val_to_quant(val, q);
    q->bins[qval]++;
    q->num++;
    return 1;
}
