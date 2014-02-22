#include <stdio.h>
#include "covq.h"

int trset_size;


int main( int argc, const char* argv[] ){
    int i,j;
    FILE * pFile;
    double qlvls[Q_LEVELS];

    srand(0);
    trset_size = 10000;
    printf("init...\n");
    init(stdin);
    assert_globals();
    printf("running...\n");
    run();
    assert_globals();

    csvwrite_double("x_ij.csv", (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    csvwrite_double("y_ij.csv", (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);

    csvwrite_int("x_encoder.csv", encoder_x, 1, Q_LEVELS);
    csvwrite_int("y_encoder.csv", encoder_y, 1, Q_LEVELS);

    for(i = 0; i < Q_LEVELS; i++)
        qlvls[i] = quant_to_vec(i, SRC_X);
    csvwrite_double("qlvls_x.csv", qlvls, 1, Q_LEVELS);

    for(i = 0; i < Q_LEVELS; i++)
        qlvls[i] = quant_to_vec(i, SRC_Y);
    csvwrite_double("qlvls_y.csv", qlvls, 1, Q_LEVELS);

}

