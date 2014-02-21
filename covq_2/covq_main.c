#include "covq.h"

int trset_size;
double *trset_x, *trset_y;

int main( int argc, const char* argv[] ){
    init();
    run();
    
    printf("x_ij:\n");
    print_double_array_2d(stdout, (double*) cv_x, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
    printf("y_ij:\n");
    print_double_array_2d(stdout, (double*) cv_y, CODEBOOK_SIZE_X, CODEBOOK_SIZE_Y);
}
