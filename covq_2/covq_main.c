#include "covq.h"

int trset_size;
double *trset_x, *trset_y;

void print_test(void){
    int i;
    int int_arr[25];
    double double_arr[25];
    
    printf("testing print int array...\n");

    for( i = 0; i < 25; i++)
        int_arr[i] = i+1;

    print_int_array(stdout, int_arr, 25);
    printf("\n");
    
    for(i = 0; i < 25; i++)
        double_arr[i]=((double)rand()/(double)RAND_MAX);
    
    printf("testing print double array...\n");
    print_double_array(stdout, double_arr, 25);
    printf("\n");
}
int main( int argc, const char* argv[] ){
    print_test();
}
