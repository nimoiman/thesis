#include "covq.h"

void print_int_array(FILE *stream, int *arr, int len){
    int i;

    for(i = 0; i < len - 1; i++)
        fprintf(stream, "%d%s", arr[i], IO_DELIM);
    fprintf(stream, "%d", arr[len - 1]);
}

void print_double_array(FILE *stream, double *arr, int len){
    int i;

    for(i = 0; i < len - 1; i++)
        fprintf(stream, "%f%s", arr[i], IO_DELIM);
    fprintf(stream, "%f", arr[len - 1]);
}

void print_int_array_2d(FILE *stream, int *arr, int rows, int cols){
    int i;
    for(i = 0; i < rows; i++){
        fprintf(stream, "\n");
        print_int_array(stream, arr + cols*i, cols);
    }
}


void print_double_array_2d(FILE *stream, double *arr, int rows, int cols){
    int i;
    for(i = 0; i < rows; i++){
        fprintf(stream, "\n");
        print_double_array(stream, arr + cols*i, cols);
    }
}
