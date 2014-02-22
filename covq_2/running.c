#include "covq.h"

void assert_globals(){
    int i,j,count = 0;
    codewords_x cw_check_x;
    codewords_y cw_check_y;

    // Assert quantized training set
    for(i = 0; i < Q_LEVELS; i++)
        for(j = 0; j < Q_LEVELS; j++)
            count += q_trset[i][j];

    assert(count == trset_size);

    // Assert encoders
    for(i = 0; i < Q_LEVELS; i++){
        assert(IN_RANGE(encoder_x[i], 0, CODEBOOK_SIZE_X));
        assert(IN_RANGE(encoder_y[i], 0, CODEBOOK_SIZE_Y));
    }
    
    // Assert codewords
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        cw_check_x[i] = 0;
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        cw_check_x[encoder_x[i]] = 1;
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        assert(cw_check_x[i] == 1);
    
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        cw_check_y[i] = 0;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        cw_check_y[encoder_y[i]] = 1;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        assert(cw_check_y[i] == 1);

    // Assert codevectors
    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            assert(IN_RANGE(cv_x[i][j], -Q_LENGTH_X/2, -Q_LENGTH_X/2));
            assert(IN_RANGE(cv_y[i][j], -Q_LENGTH_Y/2, -Q_LENGTH_Y/2));
        }
    }
}

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

void init(void) {
    // quantize
    printf("quantizing...\n");
    quantize();

    printf("init codewords...\n");
    init_binary_codewords();
    printf("init codevectors...\n");
    init_codevectors();

    printf("init encoder...\n");
    nn_update( 1 ); // Set encoders, init flag set to 1

    printf("first encoder...\n");
    nn_update( 0 ); // First real nearest neighbour update
    printf("centroid X...\n");
    centroid_update( SRC_X ); // First real centroid update
    printf("centroid Y...\n");
    centroid_update( SRC_Y );

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
        }
        printf("annealing...\n");
        anneal();
    }
    printf("finished. d = %f\n", d);
}
