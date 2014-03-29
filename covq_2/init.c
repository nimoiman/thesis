#include "covq.h"


void free_covq2(covq2 *v)
{
    free(v->I_X);
    free(v->I_Y);
    free(v->b_X);
    free(v->b_Y);
}

int malloc_covq2(covq2 *v, int N_X, int N_Y, int L_X, int L_Y)
{
    v->N_X = N_X;
    v->N_Y = N_Y;

    v->I_X = (int*) malloc(L_X * sizeof(int));
    v->I_Y = (int*) malloc(L_Y * sizeof(int));

    v->b_X = (int*) malloc(N_X * sizeof(int));
    v->b_Y = (int*) malloc(N_Y * sizeof(int));

    if(!v->I_X || !v->I_Y || !v->b_X || !v->b_Y){
        free_covq2(v);
        fprintf(stderr, "Error allocating for covq2.\n");
        return 0;
    }
    return 1;
}

