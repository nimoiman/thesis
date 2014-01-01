#include "vector.h"

// Initializes a vectorset for a given size. Returns 1 on success, otherwise 0
// Remember to call destroy_vector when you are done
vectorset *init_vectorset(size_t size){
    int i;
    vectorset *vset;
    double (*v)[VECTOR_DIM];

    vset = (vectorset*) malloc(sizeof(vectorset));
    if(vset){
        v = (double(*)[VECTOR_DIM]) calloc(size*VECTOR_DIM,sizeof(double));
        if(v){
            vset->size = size;
            vset->v = v;
            return vset;
        }
        free(vset);
    }
    return NULL;
}

void destroy_vectorset(vectorset *vset){
    free(vset->v);
}

// Prints a vectorset to standard output
// One vector per row, tab delimited
void print_vectorset(FILE *stream, vectorset *vset){
    int i, j;
    for(j = 0; j < VECTOR_DIM-1; j++)
        fprintf(stream, "v%d\t", j+1);
    fprintf(stream, "v%d\n", VECTOR_DIM);
    for(i = 0; i < vset->size; i++){
        for(j = 0; j < VECTOR_DIM-1; j++)
            fprintf(stream, "%f\t", vset->v[i][j]);
        fprintf(stream, "%f\n", vset->v[i][VECTOR_DIM-1]);
    }
}
    
/* Distortion function. Returns the mean-squared-error distortion between
   vector x and vector y. */
double dist(double *x, double *y){
       int i;
       double d = 0;
       for(i = 0; i < VECTOR_DIM; i++) {
           d += (x[i] - y[i])*(x[i] - y[i]);
       }
       return d;
}

int hamming_distance(int v_1, int v_2) {
    int i, count;
    count = 0;
    for (i = 0; i < sizeof(int)*8; i++) {
        // count the different bits
        count += ((v_1 & (1 << i)) ^ (v_2 & (1 << i))) >> i; // 1 or 0
    }
    return count;
}