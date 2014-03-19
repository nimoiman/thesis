#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /* FILE, stderr */
#include <string.h>
#include "vector.h" /* vectorset */

#define IO_DELIM ","

uint get_num_cols(FILE *stream);
size_t get_num_lines(FILE *stream);
int get_next_csv_record(FILE *stream, double *record, uint vector_dim);
int read_cw_map_csv(FILE *stream, uint *cw_map, uint rate);
void print_int(FILE *stream, int *arr, int size);
void print_2d_int(FILE *stream, int **arr, int rows, int cols);
void print_vectorset(FILE *stream, vectorset *vset);
void print_vector(FILE *stream, double *v, int dim);

#ifdef __cplusplus
}
#endif

#endif // IO_H