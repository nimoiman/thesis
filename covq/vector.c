#include "vector.h"

// Initializes a vectorset for a given size. Returns 1 on success, otherwise 0
// Remember to call destroy_vector when you are done
vectorset *init_vectorset(size_t size, int dim){
    int i;
    vectorset *vset;
    double **v;

    vset = (vectorset*) malloc(sizeof(vectorset));
    if (vset) {
        v = (double**) malloc(size * sizeof(double*));
        for (i = 0; i < size; i++) {
            v[i] = calloc(dim, sizeof(double));
            if (!v[i]) {
                for (; i >=0; i--) {
                    free(v[i]);
                }
                free(v);
                break;
            }
        }
        if (v) {
            vset->size = size;
            vset->dim = dim;
            vset->v = v;
            return vset;
        }
        free(vset);
    }
    return NULL;
}

void destroy_vectorset(vectorset *vset){
    int i;
    for (i = 0; i < vset->size; i++) {
        free(vset->v[i]);
    }
    free(vset->v);
    free(vset);
}
    
/* Distortion function. Returns the squared-error distortion between
   vector x and vector y. */
double dist(double *x, double *y, int dim){
    int i;
    double d = 0;
    for (i = 0; i < dim; i++) {
        d += (x[i] - y[i])*(x[i] - y[i]);
    }

    return d;
}

int hamming_distance(int v_1, int v_2) {
    return __builtin_popcount(v_1 ^ v_2);
}

int vector_equals(double *v_1, double *v_2, int dim) {
    int i;
    for (i = 0; i < dim; i++) {
        if (v_1[i] != v_2[i]) {
            return 0;
        }
    }
    return 1;
}