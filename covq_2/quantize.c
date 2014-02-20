
/* Convert 'vector' x to quantization level */
int vec_to_quant(double x, int *outlier, int src) {
	double normalized;
	int q_lvls;
	double mean, sigma;

	if (src == SRC_X) {
		q_lvls = Q_LEVELS_X;
		mean = mean_x[0];
		sigma = sigma_x[0];
	}
	else { // src == SRC_Y
		q_lvls = Q_LEVELS_Y;
		mean = mean_y[0];
		sigma = sigma_y[0];
	}

	normalized = 0.5 * (((x - mean) / (3*sigma)) + 1);
	if (x - mean < -3*sigma) {
		*outlier = 1;
		return 0;
	}
	else if (x - mean > 3*sigma) {
		*outlier = 1;
		return (q_lvls - 1);
	}
	else {
		normalized = 0.5 * (((x - mean) / (3*sigma)) + 1);
		return (int) (q_lvls * normalized);
	}
}

/* Return center of quantization region indexed by x */
double quant_to_vec(int x, int src) {
	if (src == SRC_X) {
		return 3 * sigma_x[0] * ((((double) (2 * x) + 1) / Q_LEVELS_X) - 1) + mean_x[0];
	}
	else { // src == SRC_Y
		return 3 * sigma_y[0] * ((((double) (2 * x) + 1) / Q_LEVELS_Y) - 1) + mean_y[0];
	}
}

/* Store quantize bin counts of quantized training vectors into q_tr
 *
 * return number of vectors outside quantize region (3 std devs from mean)
 * & puts vectors outside of 3 standard deviations in nearest bin
 * for DIM=N, Gaussian components, this is ~0.001*(1-0.999^N)/0.999
 * fraction of the vectors
 * i.e. (Vector is outlier) ~ Geometric(0.001, N)
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
