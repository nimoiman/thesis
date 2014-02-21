#include "covq.h"

int trset_size;
double *trset_x, *trset_y;

int main( int argc, const char* argv[] ){
    int i,j;
    trset_size = 1000;
    for( i = 0; i < 50; i++ )
        printf("%d\n", rand_lim(100));
    return 1;
    printf("reading csv...\n");
    read_csv_trset(stdin);
    quantize();
    print_int_array_2d(stdout, (int*) q_trset, Q_LEVELS, Q_LEVELS);
    printf("init...\n");
    init();
    print_int_array(stdout, bin_cw_x, CODEBOOK_SIZE_X);
    printf("\n");
    print_int_array(stdout, bin_cw_y, CODEBOOK_SIZE_Y);
    printf("\n");
    return 0;
    printf("running...\n");
    run();
    
    printf("x_ij:\n");
    print_double_array_2d(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("y_ij:\n");
    print_double_array_2d(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
}
