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
        *outlier = 1;
        return 0;
    }
    else if (normalized >= 1) {
        *outlier = 1;
        return (Q_LEVELS - 1);
    }
    else {
        // within range [0, Q_LEVELS-1]
        return (int) (Q_LEVELS * normalized);
    }
}

/* Return vector centroid of quantization region indexed by x */
double quant_to_vec(int x, int src) {
    double normalized;
    normalized = ((double) x + 0.5) / Q_LEVELS;
    if (src == SRC_X) {
        return Q_LENGTH_X * normalized;
    }
    else { // src == SRC_Y
        return Q_LENGTH_Y * normalized;
    }
}

/* Store quantize bin counts of quantized training vectors into q_trset
 *
 * return number of vectors outside quantize region
 * & discards vectors outside of quantize region
 * note: (Vector is outlier) ~ Geometric(p, N) where p is the probability of
 * a given component being outside the quantize region
 */
int quantize() {
    int i, j, dim; // iteration variables
    int x_bin, y_bin;
    int outlier_count = 0;

    // initialize quantized bin counts to 0
    for (i = 0; i < Q_LEVELS; i++) {
        for (j = 0; j < Q_LEVELS; j++) {
            q_trset[i][j] = 0;
        }
    }

    // iterate through X and Y jointly
    for (i = 0; i < trset_size; i++) {
        // check if training vector an outlier
        if (trset_x[i] < Q_LENGTH_X || Q_LENGTH_X <= trset_x[i] ||
            trset_y[i] < Q_LENGTH_Y || Q_LENGTH_Y <= trset_y[i]) {
            // discard this training vector
            outlier_count++;
            continue;
        }
        x_bin = vec_to_quant(trset_x[i], NULL, SRC_X);
        y_bin = vec_to_quant(trset_y[i], NULL, SRC_Y);
        q_trset[x_bin][y_bin] += 1;
    }

    return outlier_count;
}
