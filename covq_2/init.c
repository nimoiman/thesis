#include "covq.h"

void set_codevec_covq2(int i, int j, double val, enum source s, covq2 *v)
{
    if(s == src_X){
        v->x_ij[(size_t)v->N_X*j + i] = val;
    }
    else{
        v->y_ij[(size_t)v->N_X*j + i] = val;
    }
}

double get_codevec_covq2(int i, int j, enum source s, covq2 *v)
{
    if(s == src_X){
        return v->x_ij[(size_t)v->N_X*j + i];
    }
    else{
        return v->y_ij[(size_t)v->N_X*j + i];
    }
}

void free_covq2(covq2 *v)
{
    free(v->I_X);
    free(v->I_Y);
    free(v->x_ij);
    free(v->y_ij);
    free(v->b_X);
    free(v->b_Y);
}

int malloc_covq2(covq2 *v, int N_X, int N_Y, int L_X, int L_Y)
{
    v->N_X = N_X;
    v->N_Y = N_Y;

    v->I_X = (int*) malloc(L_X * sizeof(int));
    v->I_Y = (int*) malloc(L_Y * sizeof(int));

    v->x_ij = (double*) malloc((size_t) N_X * N_Y * sizeof(double));
    v->y_ij = (double*) malloc((size_t) N_X * N_Y * sizeof(double));

    v->b_X = (int*) malloc(N_X * sizeof(int));
    v->b_Y = (int*) malloc(N_Y * sizeof(int));

    if(!v->I_X || !v->I_Y || !v->x_ij || !v->y_ij || !v->b_X || !v->b_Y){
        free_covq2(v);
        fprintf(stderr, "Error allocating for covq2.\n");
        return 0;
    }
    return 1;
}

