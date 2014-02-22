#include "covq.h"
#include <string.h>

/* Read csv file into trset_x, trset_y 
 * Assumes two colums delimited by commas, each line delimited by newline
 */
void read_csv_trset(FILE *stream) {
    int i;
    char line[1024];
    // count the lines
    trset_x = malloc(trset_size * sizeof(double));
    trset_y = malloc(trset_size * sizeof(double));
    
    // read file again, line by line:
    i = 0;
    while (fgets(line, 1024, stream) != NULL) {
        sscanf(line, "%lf,%lf", trset_x+i, trset_y+i);
        i++;
    }
}

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

int csvwrite_int(char *filename, int *arr, int rows, int cols){
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


int csvwrite_double(char *filename, double *arr, int rows, int cols){
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
