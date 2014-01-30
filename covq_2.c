#define C_SIZE_X (1 << split_x)
#define C_SIZE_Y (1 << split_y)

/* GLOBAL VARIABLES */
size_t tr_size;
vec_x *tr_x;
vec_y *tr_y;

c_book_x *c_x;
c_book_y *c_y;

int split_x = 0;
int split_y = 0;

int *enc_x;
int *enc_y;

double sigma_x[DIM_X];
double sigma_y[DIM_Y];
double mean_x[DIM_X];
double mean_y[DIM_Y]; // means and std devs of vector components

void print_vector(FILE *stream, double *v, int src) {
    int i;
    int max_dim = (src == X) ? DIM_X : DIM_Y;
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
void print(FILE *stream, int thing) {
    if (thing == 0) { // training set
        int i;
        fprintf(stream, "X\tY\n");
        for (i = 0; i < tr_size; i++) {
            print_vector(stream, tr_x[i], X);
            fprintf(stream, "\t");
            print_vector(stream, tr_y[i], Y);
            fprintf(stream, "\n");
        }
    }
    else if (thing == 1) { // X codebook

    }
    else if (thing == 2) { // quantized bin counts

    }
}

void print_training_set() {
    
}

void print_codebook() {
    
}

void print_quant_levels() {
    
}

int init(covq *c) {
    // unpack everything
    tr_size = c->tr_size;
    tr_x = c->tr_x;
    tr_y = c->tr_y;

    c_book_x = c->c_book_x;
    c_book_y = c->c_book_y;

    split_x = c->split_x;
    split_y = c->split_y;

    enc_x = c->enc_x;
    enc_y = c->enc_y;

    sigma_x = c->sigma_x;
    sigma_y = c->sigma_y;
    mean_x = c->mean_x;
    mean_y = c->mean_y;

    // malloc everything
    c_x = malloc(sizeof(c_book_x));
    c_y = malloc(sizeof(c_book_y));
    enc_x = malloc(sizeof(int) * tr_size);
    enc_y = malloc(sizeof(int) * tr_size);

    // quantize
    quantize();
}

/* return number of vectors outside quantize region (3 std devs from mean)
 * & puts vectors outside of 3 standard deviations in nearest bin
 * for DIM=N, Gaussian components, this is ~0.001*(1-0.999^N)/0.999
 * fraction of the vectors
 * i.e. (Vector is outlier) ~ Geometric(0.001, N)
 */
int quantize() {
    int i, dim; // iteration variables
    int outlier = 0;
    int outlier_count = 0;
    double normalized;
    // iterate through X and Y jointly
    for (i = 0; i < tr_size; i++) {
        for (dim = 0; dim < DIM_X; dim++) {
            if (tr_x[i][dim] - mean_x[dim] < -3*sigma_x[dim]) {
                enc_x[i][dim] = 0;
                outlier = 1;
            }
            else if(tr_x[i][dim] - mean_x[dim] > 3*sigma_x[dim]) {
                enc_x[i][dim] = Q_LEVELS_X - 1;
                outlier = 1;
            }
            else {
                normalized = 0.5 * (((tr_x[i][dim] - mean_x[dim]) / (3*sigma_x[dim])) + 1);
                enc_x[i][dim] = (int) (Q_LEVELS_X * normalized);
            }
        }
        for (dim = 0; dim < DIM_Y; dim++) {
            if (tr_y[i][dim] - mean_y[dim] < -3*sigma_y[dim]) {
                enc_y[i][dim] = 0;
                outlier = 1;
            }
            else if(tr_y[i][dim] - mean_y[dim] > 3*sigma_y[dim]) {
                enc_y[i][dim] = Q_LEVELS_Y - 1;
                outlier = 1;
            }
            else {
                normalized = 0.5 * (((tr_y[i][dim] - mean_y[dim]) / (3*sigma_y[dim])) + 1);
                enc_y[i][dim] = (int) (Q_LEVELS_Y * normalized);
            }
        }
        if (outlier) {
            outlier_count++;
            outlier = 0;
        }
    }
    return outlier_count;
}

/* return hamming distance between bits of indices v_1, v_2 */
int hamming_distance(int v_1, int v_2) {
    int i, count;
    count = 0;
    for (i = 0; i < sizeof(int)*8; i++) {
        // count the different bits
        count += ((v_1 & (1 << i)) ^ (v_2 & (1 << i))) >> i; // 1 or 0
    }
    return count;
}

/* src should be "X" or "Y" (macros for 0, 1 resp.) */
double trans_prob(int i, int j, int src) {
    if (src == X) {
        return pow(TRANS_PROB_X, hamming_distance(i, j)) *
            pow(1 - TRANS_PROB_X, split_x - hamming_distance(i, j));
    }
    else { // (src == Y)
        return pow(TRANS_PROB_Y, hamming_distance(i, j)) *
            pow(1 - TRANS_PROB_Y, split_y - hamming_distance(i, j));
    }
}

double dist(int q_lvl, int index, int src) {

}

/* return *index by reference */
double nearest_neighbour(int q_lvl, int *index, int src) {
    // calls dist, trans_prob
}

double centroid_update(int src) {
    // calls nearest_neighbour, trans_prob
}

void split(int src) {

}

int bsc_2_source_covq(covq *everything) {
    init(everything);
    // centroid_update
    // split

    // repack
    return 0; // yay
}