#ifndef IO_H
#define IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h> /* FILE, stderr */
#include <string.h>

#define IO_DELIM ","

int get_num_lines(FILE *stream);
int get_next_csv_record(FILE *stream, double *record, int vector_dim);
void print_int(FILE *stream, int *arr, int rows, int cols);
void print_double(FILE *stream, double *arr, int rows, int cols);
int csvwrite_int(char *filename, int *arr, int rows, int cols);
int csvwrite_double(char *filename, double *arr, int rows, int cols);

#ifdef __cplusplus
}
#endif

#endif // IO_H