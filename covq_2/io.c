#include "covq_2.h"

void print_vector(FILE *stream, double *v, int src) {
    int i;
    int max_dim = (src == SRC_X) ? DIM_X : DIM_Y;
    fprintf(stream, "(");
    for (i = 0; i < max_dim; i++) {
        fprintf(stream, "%f", v[i]);
        if (i != max_dim - 1) {
            fprintf(stream, ", ");
        }
    }
    fprintf(stderr, ")");
}

/* prints various things to stream in human-readable format */
void print_h(FILE *stream, int thing) {
    if (thing == 0) { // training set
        int i;
        fprintf(stream, "X\tY\n");
        for (i = 0; i < tr_size; i++) {
            print_vector(stream, &tr_x[i], SRC_X);
            fprintf(stream, "\t");
            print_vector(stream, &tr_y[i], SRC_Y);
            fprintf(stream, "\n");
        }
    }
    else if (thing == 1) { // X codebook

    }
    else if (thing == 2) { // quantized bin counts
        int i, j;
        for (i = 0; i < Q_LEVELS_X; i++) {
            for (j = 0; j < Q_LEVELS_Y; j++) {
                fprintf(stream, "%d", q_tr[i][j]);
                if (j != Q_LEVELS_Y - 1) {
                    fprintf(stream, "  ");
                }
            }
            if (i != Q_LEVELS_X - 1) {
                fprintf(stream, "\n");
            }
        }
        fprintf(stream, "\n");
    }
}

/* prints various things to stream in csv format */
void print(FILE *stream, int thing) {
    if (thing == 0) { // training set

    }
    else if (thing == 1) { // X codebook

    }
    else if (thing == 2) { // quantized bin counts
        // fprintf(stderr, "about to print q_tr\n");
        int i, j;
        for (i = 0; i < Q_LEVELS_X; i++) {
            for (j = 0; j < Q_LEVELS_Y; j++) {
                fprintf(stream, "%d", q_tr[i][j]);
                if (j != Q_LEVELS_Y - 1) {
                    fprintf(stream, ",");
                }
            }
            if (i != Q_LEVELS_X - 1) {
                fprintf(stream, ";");
            }
        }
        fprintf(stream, "\n");
    }
}

void print_training_set() {
    
}

void print_codebook() {
    
}

void print_quant_levels() {
    
}
