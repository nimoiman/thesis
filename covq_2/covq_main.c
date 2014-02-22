#include <stdio.h>
#include "covq.h"

int trset_size;
double *trset_x, *trset_y;


int main( int argc, const char* argv[] ){
    int i,j;
    FILE * pFile;

    srand(0);
    trset_size = 10000;
    printf("reading csv...\n");
    read_csv_trset(stdin);
    quantize();
    print_int(stdout, (int*) q_trset, Q_LEVELS, Q_LEVELS);
    printf("init...\n");
    init();
    printf("running...\n");
    run();
    
    printf("x_ij:\n");
    print_double(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("\n\ny_ij:\n");
    print_double(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);

    csvwrite_double("x_ij.csv", (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    csvwrite_double("y_ij.csv", (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
}

