#include "covq.h"

/* Convert 'vector' x to quantization level */
int vec_to_quant(double x, int *outlier, int src) {
    double normalized;
    int q_lvls;
    double mean, spread;

    if (src == SRC_X) {
        q_lvls = Q_LEVELS_X;
        mean = mean_x[0];
        spread = range_x[0];
    }
    else { // src == SRC_Y
        q_lvls = Q_LEVELS_Y;
        mean = mean_y[0];
        spread = range_y[0];
    }

    normalized = 0.5 * (((x - mean) / (spread)) + 1);
    if (x - mean < spread) {
        *outlier = 1;
        return 0;
    }
    else if (x - mean > spread) {
        *outlier = 1;
        return (q_lvls - 1);
    }
    else {
        normalized = 0.5 * (((x - mean) / spread) + 1);
        return (int) (q_lvls * normalized);
    }
}

/* Return center of quantization region indexed by x */
double quant_to_vec(int x, int src) {
    if (src == SRC_X) {
        return range_x[0] * ((((double) (2 * x) + 1) / Q_LEVELS_X) - 1) + mean_x[0];
    }
    else { // src == SRC_Y
        return range_y[0] * ((((double) (2 * x) + 1) / Q_LEVELS_Y) - 1) + mean_y[0];
    }
}

/* Store quantize bin counts of quantized training vectors into q_tr
 *
 * return number of vectors outside quantize region
 * & puts vectors outside of quantize region in nearest bin
 * note: (Vector is outlier) ~ Geometric(p, N) where p is the probability of
 * a given component being outside the quantize region
 */
int quantize() {
    int i, j, dim; // iteration variables
    int x_bin, y_bin;
    int outlier = 0;
    int outlier_count = 0;

    // initialize quantized bin counts to 0
    for (i = 0; i < Q_LEVELS_X; i++) {
        for (j = 0; j < Q_LEVELS_Y; j++) {
            q_tr[i][j] = 0;
        }
    }

    // iterate through X and Y jointly
    for (i = 0; i < tr_size; i++) {
        x_bin = vec_to_quant(tr_x[i], &outlier, SRC_X);
        y_bin = vec_to_quant(tr_y[i], &outlier, SRC_Y);
        q_tr[x_bin][y_bin] += 1;

        if (outlier) {
            outlier_count++;
            outlier = 0;
        }
    }

    return outlier_count;
}
