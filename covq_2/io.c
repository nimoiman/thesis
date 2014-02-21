#include "covq.h"
#include <string.h>
int trset_size;

/* Read csv file into trset_x, trset_y 
 * Assumes two colums delimited by commas, each line delimited by newline
 */
void read_csv_trset(FILE *stream) {
    int i;
    char line[1024];
    // count the lines
    for (i = 0; fgets(line, 1024, stream); i++);
    printf("num lines=%d\n", i);
    trset_size = i;
    trset_x = malloc(trset_size * sizeof(double));
    trset_y = malloc(trset_size * sizeof(double));
    
    // read file again, line by line:
    rewind(stream);
    i = 0;
    while (fgets(line, 1024, stream) != NULL) {
        sscanf(line, "%lf,%lf", trset_x+i, trset_y+i);
        i++;
    }
}

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

void print_int_array_2d(FILE *stream, int *arr, int n, int m){
    int i;
    for(i = 0; i < m; i++){
        fprintf(stream, "\n");
        print_int_array(stream, arr + m*i, n);
    }
}


void print_double_array_2d(FILE *stream, double *arr, int n, int m){
    int i;
    for(i = 0; i < m; i++){
        fprintf(stream, "\n");
        print_double_array(stream, arr + m*i, n);
    }
}
