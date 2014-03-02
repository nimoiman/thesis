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
        assert(IN_RANGE(encoder_x[i], 0, CODEBOOK_SIZE_X-1));
        assert(IN_RANGE(encoder_y[i], 0, CODEBOOK_SIZE_Y-1));
    }
    
    // Assert codewords
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        cw_check_x[i] = 0;
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        cw_check_x[bin_cw_x[i]] = 1;
    for(i = 0; i < CODEBOOK_SIZE_X; i++)
        assert(cw_check_x[i] == 1);
    
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        cw_check_y[i] = 0;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        cw_check_y[bin_cw_y[i]] = 1;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++)
        assert(cw_check_y[i] == 1);
}

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
    for( j = 0; j < 1; j++ ){
        for( i = 0; i < 50; i++ ){
            centroid_update( SRC_X );
            d = nn_update( 0 );
            centroid_update( SRC_Y );
            d = nn_update( 0 );
        }
        anneal();
    }
    printf("finished. d = %f\n", d);
}
