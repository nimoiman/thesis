#include "covq.h"
#include <string.h>

/* Read next line of *stream into record.
 * Return 1 on success, 0 on EOF */
int get_next_csv_record(FILE *stream, double record[2]) {
    char line[1024];
    if (fgets(line, 1024, stream) != NULL) {
        sscanf(line, "%lf,%lf", &(record[0]), &(record[1]));
        return 1;
    }
    else {
        return 0;
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
