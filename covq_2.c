#include "covq_2.h"

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

q_vec *q_tr;

void print_training_set() {
    
}

void print_codebook() {
    
}

void print_quant_levels() {
    
}

int init(covq *everything) {
    // unpack everything
    // mallocs everything
    // quantize
}

/* return number of vectors outside quantize region */
int quantize() {

}

/* Computes the channel transition probability from index i to index j for a
 * given source (src). */
double trans_prob(int i, int j, int src) {
	double p = 1;
	double prob;
	int len, k, diff;

	diff = i ^ j;
	len = (src == X) ? split_x : split_y;
	prob = (src == X) ? TRANS_PROB_X : TRANS_PROB_Y;
	for(k = 0; k < len; k++)
		p *= (diff>>i & 1) ? prob : (1-prob);
	return p;
}

/* Computes the distortion between a source and code vector for a given source.
 * The quantized level should be input, along with the index to the code
 * vector, and the source.  Returns the expected mean squared distortion
 * between q_lvl and the estimate at the source for the given index to be
 * transmitted. */
double dist(int q_lvl, int index, int src) {
}

/* for a given source vector, returns the nearest code vector and respective
 * distortion with respect to the dist() function. The source quantization
 * level should be passed, along with a pointer which will point to the nearest
 * code vector index, and the source. */
double nearest_neighbour(int q_lvl, int *index, int src) {
}

/* Updates the centroids for a given source by applying the centroid condition. */
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
