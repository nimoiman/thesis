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
int *q_marg_x;
int *q_marg_y;

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

/* for a given source vector, returns the nearest code vector and respective
 * distortion with respect to the dist() function. The source quantization
 * level should be passed, along with a pointer which will point to the nearest
 * code vector index, and the source. */
double nearest_neighbour(int q_lvl, int *index, int src) {
	int src_count[C_SIZE_Y];
	double src_sum[C_SIZE_Y], rcv_prob[C_SIZE_Y], rcv_avg[C_SIZE_Y];
	double p, d, d_best, var, val_x, val_y, d_term, *dist;
	int i, j, k, l, num, c, count, i_best;

	for(i = 0; i < Q_LEVELS_Y; i++){
		num = q_tr[q_lvl][i];
		c = enc_y[i];
		val_y = quant_to_vec(i, Y);

		count += num;
		src_count[c] += num;
		src_sum[c] += num * val_y;
		var += num * val_y * val_y;
	}
	var /= count;
	
	for(l = 0; l < C_SIZE_Y; l++){
		for(j = 0; j < C_SIZE_Y; j++){
			p = trans_prob(j,l, Y);
			rcv_prob[l] += p * src_count[j];
			rcv_avg[l] += p * src_sum[j];
		}
		rcv_prob[l] /= count;
		rcv_avg[l] /= count;
	}

	val_x = quant_to_vec(q_lvl, X);
	dist = src_sum;
	for(k = 0; k < C_SIZE_X; k++){
		dist[k] = 0;
		for(l = 0; l < C_SIZE_Y; l++){
			d_term = (val_x-c_x[k][l]) * (val_x-c_x[k][l])
				+ c_y[k][l] * c_y[k][l]
				- 2 * c_y[k][l] * rcv_avg[l];
			dist[k] += rcv_prob[l] * d_term;
		}
	}

	for(i = 0; i < C_SIZE_X; i++){
		d = 0;
		for(k = 0; k < C_SIZE_X; k++)
			d += trans_prob(i,k,X) * dist[k];
		if( d < d_best ){
			d_best = d;
			i_best = i;
		}
	}

	*index = i_best;
	return var + d_best;
}

double nn_update(){
	double d_total, d;
	int i, j, num;
	
	for(i = 0; i < Q_LEVELS; i++){
		d = nearest_neighbour(i, enc_x + i, X);
		num = 0;
		for(j = 0; j < Q_LEVELS; j++)
			num += q_tr[i][j];
		d_total += num * d;
	}

	for(j = 0; j < Q_LEVELS; j++){
		d = nearest_neighbour(j, enc_y + j, Y);
		num = 0;
		for(i = 0; i < Q_LEVELS; i++)
			num += q_tr[i][j];
		d_total += num * d;
	}

	return d_total / tr_size;
}

int vec_to_quant(double x, int src){
	return 0;
}

double quant_to_vec(int x, int src){
	return 0;
}
/* Updates the centroids for a given source by applying the centroid condition. */
void centroid_update(int src) {
	int count[C_SIZE_X][C_SIZE_Y];
	double sum[C_SIZE_X][C_SIZE_Y];
	int num, i, j, k, l, idx_x, idx_y;
	double c, val_x, d_x, d_y, d_total, p_x, p_y, numer, denom;

	for(i = 0; i < C_SIZE_X; i++){
		idx_x = enc_x[i];
		val_x = quant_to_vec(i);
		for(j = 0; j < C_SIZE_Y; j++){
			idx_y = enc_y[j];
			num = q_tr[i][j];
			count[idx_x][idx_y] += num;
			sum[idx_x][idx_y] += num * val_x;
		}
	}

	for(k = 0; k < C_SIZE_X; k++){
		for(l = 0; l < C_SIZE_Y; l++){
			numer = 0;
			denom = 0;
			for(i = 0; i < C_SIZE_X; i++){
				p_x = trans_prob(i,k,X);
				for(j = 0; j < C_SIZE_Y; j++){
					p_y = trans_prob(j,l,Y);
					numer += p_x * p_y * sum[i][j];
					denom += p_x * p_y * count[i][j];
				}
			}
			c_x[k][l] = numer / denom;
		}
	}
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
