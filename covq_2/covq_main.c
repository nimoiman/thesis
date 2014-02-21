#include "covq.h"

int trset_size;
double *trset_x, *trset_y;

int main( int argc, const char* argv[] ){
    trset_size = 1000;
    printf("reading csv...\n");
    read_csv_trset(stdin);
    printf("training set x:\n");
    print_double_array(stdout, (double*) trset_x, trset_size);
    printf("training set y:\n");
    print_double_array(stdout, (double*) trset_y, trset_size);
    printf("init...\n");
    init();
    printf("running...\n");
    run();
    
    printf("x_ij:\n");
    print_double_array_2d(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("y_ij:\n");
    print_double_array_2d(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
}
