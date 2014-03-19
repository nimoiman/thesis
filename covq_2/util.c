#include "covq.h"


#define ANNEAL_LOOP_NUM 1
#define ROTATE_LOOP_NUM 50
#define IO_DELIM ", "

void print_int(FILE *stream, int *arr, int rows, int cols){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols - 1; j++)
            fprintf(stream, "%d%s", arr[cols*i + j], IO_DELIM);
        fprintf(stream, "%d\n", arr[cols*i + j]);
    }
}

void print_double(FILE *stream, double *arr, int rows, int cols){
    int i, j;
    for(i = 0; i < rows; i++){
        for(j = 0; j < cols - 1; j++)
            fprintf(stream, "%f%s", arr[cols*i + j], IO_DELIM);
        fprintf(stream, "%f\n", arr[cols*i + j]);
    }
}

int fprintf_int(char *filename, int *arr, int rows, int cols){
    FILE *pFile = fopen(filename, "w");

    if( pFile != NULL ){
        print_int(pFile, arr, rows, cols);
        fclose(pFile);
        return 1;
    }else{
        fprintf(stderr, "ERROR: Could not open %s\n", filename);
        return 0;
    }
}

int fprintf_double(char *filename, double *arr, int rows, int cols){
    FILE *pFile = fopen(filename, "w");

    if( pFile != NULL ){
        print_double(pFile, arr, rows, cols);
        fclose(pFile);
        return 1;
    }else{
        fprintf(stderr, "ERROR: Could not open %s\n", filename);
        return 0;
    }
}

