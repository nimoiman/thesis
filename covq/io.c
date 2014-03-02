#include "io.h"

int get_num_lines(FILE *stream) {
    char line[1024];
    int i = 0;
    while(fgets(line, 1024, stream) != NULL) {
        i++;
    }
    return i;
}

/* Read next line of *stream into record.
 * Return 1 on success, 0 on EOF */
int get_next_csv_record(FILE *stream, double *record, int vector_dim) {
    char line[1024];
    char *token;
    int i;
    if (fgets(line, 1024, stream) != NULL) {
        // get first token:
        token = strtok(line, IO_DELIM);
        sscanf(token, "%lf", &(record[0]));
        // get next tokens up to vector_dim
        for (i = 1; i < vector_dim; i++) {
            token = strtok(NULL, IO_DELIM);
            sscanf(token, "%lf", &(record[i]));
        }
        return 1;
    }
    else {
        return 0;
    }
}

void print_vector(FILE *stream, double *v, int dim) {
    int i;
    for (i = 0; i < dim; i++) {
        fprintf(stream, "%f%s", v[i], (i < dim - 1) ? IO_DELIM : "\n");
    }
}

// Prints a vectorset to file stream
// One vector per row, delimited by IO_DELIM
void print_vectorset(FILE *stream, vectorset *vset) {
    int i, j;
    for (i = 0; i < vset->size; i++) {
        print_vector(stream, vset->v[i], vset->dim);
    }
}

void print_int(FILE *stream, int *arr, int size) {
    int i;
    for (i = 0; i < size; i++) {
        fprintf(stream, "%d%s", arr[i], (i < size-1) ? IO_DELIM : "\n");
    }
}

void print_2d_int(FILE *stream, int **arr, int rows, int cols) {
    int i, j;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            fprintf(stream, "%d%s", arr[i][j], (j < cols-1) ? IO_DELIM: "\n");
        }
    }
}
