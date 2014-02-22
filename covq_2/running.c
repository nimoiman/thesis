#include "covq.h"

void init_binary_codewords(void){
    int i;
    for( i = 0; i < CODEBOOK_SIZE_X; i++)
        bin_cw_x[i] = i;

    for( i = 0; i < CODEBOOK_SIZE_Y; i++)
        bin_cw_y[i] = i;
}

void init(FILE *stream) {
    // quantize
    printf("quantizing...\n");
    printf("& init codevectors...\n");
    quantize(stream);

    printf("init codewords...\n");
    init_binary_codewords();

    printf("X codevectors:\n");
    print_double_array_2d(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("\nY codevectors:\n");
    print_double_array_2d(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("\n");
    printf("init encoder...\n");
    nn_update( 1 ); // Set encoders, init flag set to 1

    printf("first encoder...\n");
    nn_update( 0 ); // First real nearest neighbour update
    printf("centroid X...\n");
    centroid_update( SRC_X ); // First real centroid update
    printf("centroid Y...\n");
    centroid_update( SRC_Y );

    printf("X codevectors:\n");
    print_double_array_2d(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("\nY codevectors:\n");
    print_double_array_2d(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("\n");
    printf("init anneal...\n");

    anneal();
}

void run(void){
    int i, j;
    double d;
    for( j = 0; j < 4; j++ ){
        for( i = 0; i < 20; i++ ){
            d = nn_update( 0 );
            centroid_update( SRC_X );
            centroid_update( SRC_Y );
            printf("j=%d,i=%d,d=%f\n",j,i,d);
        }
        printf("annealing...\n");
        anneal();
    }
    printf("finished. d = %f\n", d);
}
