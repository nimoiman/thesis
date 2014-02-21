#include "covq.h"


void init_codevectors(void){
    int i, j;

    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            cv_x[i][j] = trset_x[i*CODEBOOK_SIZE_Y + j];
            cv_y[i][j] = trset_y[i*CODEBOOK_SIZE_Y + j];
        }
    }
}

void init_binary_codewords(void){
    int i;
    for( i = 0; i < CODEBOOK_SIZE_X; i++)
        bin_cw_x[i] = i;

    for( i = 0; i < CODEBOOK_SIZE_Y; i++)
        bin_cw_y[i] = i;
}

int init(void) {
    // quantize
    quantize();

    init_binary_codewords();
    init_codevectors();
    nn_update( 1 ); // Set encoders, init flag set to 1

    nn_update( 0 ); // First real nearest neighbour update
    centroid_update( SRC_X ); // First real centroid update
    centroid_update( SRC_Y );

    anneal();
    return 1;
}
