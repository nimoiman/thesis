#include "covq.h"

#define ARR_ROWS 10
#define ARR_COLS 4

int trset_size;
double *trset_x, *trset_y;

void print_test(void){
    int i, j;
    int int_arr[ARR_COLS];
    double double_arr[ARR_COLS];
    int int_arr_2D[ARR_ROWS][ARR_COLS];
    
    printf("testing print int array...\n");

    for( i = 0; i < ARR_COLS; i++)
        int_arr[i] = i+1;

    print_int_array(stdout, int_arr, ARR_COLS);
    printf("\n");
    
    for(i = 0; i < ARR_COLS; i++)
        double_arr[i]=((double)rand()/(double)RAND_MAX);
    
    printf("testing print double array...\n");
    print_double_array(stdout, double_arr, ARR_COLS);
    printf("\n");
    
    for( i = 0; i < ARR_ROWS; i++ )
        for( j = 0; j < ARR_COLS; j++ )
            int_arr_2D[i][j] = j;

    printf("testing print int matrix...\n");
    print_int_array_2d(stdout, (int*) int_arr_2D, ARR_ROWS, ARR_COLS);
    printf("\n");

}

int main( int argc, const char* argv[] ){
    print_test();
}
