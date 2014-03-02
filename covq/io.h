#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /* FILE, stderr */
#include <string.h>
#include "vector.h" /* vectorset */

#define IO_DELIM ","

int get_num_lines(FILE *stream);
int get_next_csv_record(FILE *stream, double *record, int vector_dim);
void print_int(FILE *stream, int *arr, int size);
void print_2d_int(FILE *stream, int **arr, int rows, int cols);
void print_vectorset(FILE *stream, vectorset *vset);
void print_vector(FILE *stream, double *v, int dim);

#ifdef __cplusplus
}
#endif

#endif // IO_H