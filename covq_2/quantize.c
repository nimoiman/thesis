#include "covq.h"

/* Convert 'vector' x to quantization level assumes zero mean */
int vec_to_quant(double x, int *outlier, int src) {
    double normalized;
    int q_lvls;
    double q_length;

    q_length = (src == SRC_X) ? Q_LENGTH_X : Q_LENGTH_Y;

    // normalize to [0, 1)
    normalized = (x / q_length) + 0.5;
    if (normalized < 0) {
        if (outlier) {
            *outlier = 1;
        }
        return 0;
    }
    else if (normalized >= 1) {
        if (outlier) {
            *outlier = 1;
        }
        return (Q_LEVELS - 1);
    }
    else {
        // within range [0, Q_LEVELS-1]
        if (outlier) {
            *outlier = 0;
        }
        return (int) (Q_LEVELS * normalized);
    }
}

/* Return vector centroid of quantization region indexed by x */
double quant_to_vec(int x, int src) {
    double normalized;
    normalized = ((double) x + 0.5) / Q_LEVELS; // [0, 1)
    if (src == SRC_X) {
        return Q_LENGTH_X * (normalized - 0.5); // [-Q_LENGTH_X/2, Q_LENGTH_X/2)
    }
    else { // src == SRC_Y
        return Q_LENGTH_Y * (normalized - 0.5); // [-Q_LENGTH_Y/2, Q_LENGTH_Y/2)
    }
}

/* Store quantize bin counts of quantized training vectors into q_trset
 *
 * return number of vectors outside quantize region
 * & discards vectors outside of quantize region
 * note: (Vector is outlier) ~ Geometric(p, N) where p is the probability of
 * a given component being outside the quantize region
 */
int quantize(FILE *stream) {
    int i, j; // iteration variables
    int x_bin, y_bin;
    double record[2]; // buffer for new training record
    int outlier_count = 0;
    int outlier_x = 0, outlier_y = 0;

    // initialize global trset_size to 0
    trset_size = 0;

    // initialize quantized bin counts to 0
    for (i = 0; i < Q_LEVELS; i++) {
        for (j = 0; j < Q_LEVELS; j++) {
            q_trset[i][j] = 0;
        }
    }

    // iterate through X and Y jointly
    while (get_next_csv_record(stream, record)) {
        x_bin = vec_to_quant(record[0], &outlier_x, SRC_X);
        y_bin = vec_to_quant(record[1], &outlier_y, SRC_Y);
        if (outlier_x || outlier_y) {
            outlier_count++;
        }
        else {
            // only count non-outliers
            trset_size += 1;
            q_trset[x_bin][y_bin] += 1;

            // TODO: remove this, come up with better initial codevectors
            // for now, take first CODEBOOK_SIZE_X * CODEBOOKSIZE_Y training
            // vectors as the initial codevectors
            if (trset_size < CODEBOOK_SIZE_X * CODEBOOK_SIZE_Y) {
                i = trset_size % CODEBOOK_SIZE_X;
                j = trset_size / CODEBOOK_SIZE_X;
                cv_x[i][j] = record[0];
                cv_y[i][j] = record[1];
            }
        }
    }
    if (trset_size < CODEBOOK_SIZE_X * CODEBOOK_SIZE_Y) {
        // we have less training vectors than codevectors
        fprintf(stderr, "Training set supplied is smaller than codebook size\n");
        exit(1);
    }
    return outlier_count;
}
