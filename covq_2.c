#include "covq_2.h"
#define POW2(x) (x * x)
/* GLOBAL VARIABLES */
size_t tr_size;
int *tr_x;
int *tr_y;

c_book c_x;
c_book c_y;

int split_x = 0;
int split_y = 0;

int *enc_x;
int *enc_y;

q_vec q_tr;

double sigma_x;
double sigma_y;
double mean_x;
double mean_y; // means and std devs of vector components

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
void print(FILE *stream, int thing) {
    if (thing == 0) { // training set
        int i;
        fprintf(stream, "X\tY\n");
        for (i = 0; i < tr_size; i++) {
            print_vector(stream, tr_x[i], SRC_X);
            fprintf(stream, "\t");
            print_vector(stream, tr_y[i], SRC_Y);
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

int init(struct covq *c) {
    // unpack everything
    tr_size = c->tr_size;
    tr_x = c->tr_x;
    tr_y = c->tr_y;

    c_x = c->c_x;
    c_y = c->c_y;

    split_x = c->split_x;
    split_y = c->split_y;

    enc_x = c->enc_x;
    enc_y = c->enc_y;

    sigma_x = c->sigma_x;
    sigma_y = c->sigma_y;
    mean_x = c->mean_x;
    mean_y = c->mean_y;

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

/* Computes the channel transition probability from index i to index j for a
 * given source (src). */
double trans_prob(int i, int j, int src) {
	double p = 1;
	double prob;
	int len, k, diff;

	diff = i ^ j;
	len = (src == SRC_X) ? split_x : split_y;
	prob = (src == SRC_X) ? TRANS_PROB_X : TRANS_PROB_Y;
	for(k = 0; k < len; k++)
		p *= (diff>>i & 1) ? prob : (1-prob);
	return p;

/* for a given source vector, returns the nearest code vector and respective
 * distortion with respect to the dist() function. The source quantization
 * level should be passed, along with a pointer which will point to the nearest
 * code vector index, and the source. */
double nearest_neighbour(int q_lvl, int *index, int src) {
	int src_count[C_SIZE_MAX];
	double src_sum[C_SIZE_MAX], rcv_prob[C_SIZE_MAX], rcv_avg[C_SIZE_MAX];
	double p, d, d_best, var, val, *dist;
	int i, j, k, l, src1, src2, num, c, count, i_best, c_size1, c_size2, *enc2;
	c_book *c1, *c2;

	// set primary and secondary source parameters
	if(src == SRC_X){
		c_size1 = C_SIZE_X;
		c_size2 = C_SIZE_Y;
		c1 = &c_x;
		c2 = &c_y;
		src1 = SRC_X;
		src2 = SRC_Y;
		enc2 = enc_y;
	}else{
		c_size1 = C_SIZE_Y;
		c_size2 = C_SIZE_X;
		c1 = &c_y;
		c2 = &c_x;
		src1 = SRC_Y;
		src2 = SRC_X;
		enc2 = enc_x;
	}
	
	// Rest of comments assume primary source is X.
	// Roles are swapped when the primary source is Y.

	// Compute:
	// E[Y^2 | X = x] (var)
	// number of points that map to each J source index (src_count)
	// used to compute probabilities of sending J index: P(J = j | X = x)
	// sum of points that map to each J source index (src_sum)
	for(i = 0; i < Q_LEVELS; i++){
		// Iterate across Y vals when src1 == SRC_X
		// Iterate across X vals when src1 == SRC_Y
		if(src1 == SRC_X)
			num = q_tr[q_lvl][i];
		else
			num = q_tr[i][q_lvl];
		c = enc2[i]; // Encoding index
		val = quant_to_vec(i, src2); // value

		count += num;
		src_count[c] += num;
		src_sum[c] += num * val;
		var += num * val * val;
	}
	var /= count;
	
	// for src1 == SRC_X, compute:
	// P(L = l | X = x) for all l (rcv_prob)
	// That is, probability of receiving l from the secondary source.
	// P(L = l | X = x) = sum_j( p(j,l) * P(J = j | X = x) )
	// Also compute E[Y | L = l, X = x] * P(J = j | X = x) for all l (rcv_avg)
	for(l = 0; l < c_size2; l++){
		for(j = 0; j < c_size2; j++){
			p = trans_prob(j,l, src2);
			rcv_prob[l] += p * src_count[j];
			rcv_avg[l] += p * src_sum[j];
		}
		rcv_prob[l] /= count;
		rcv_avg[l] /= count;
	}

	val = quant_to_vec(q_lvl, src1); //Get x value from quantizer index
	dist = src_sum; // reuse src_sum for new array, dist
	// Compute E[(X-X_hat)^2 + (Y-Y_hat)^2 | X = x, K = k] - E[Y^2|X=x,K=k]
	// That is, expected distortion given that k was received for primary source.
	// Minus E[Y^2|X=x,K=k] because it is independent of K
	// It is added later by the variable var which was computed earlier.
	for(k = 0; k < c_size1; k++){
		dist[k] = 0;
		for(l = 0; l < c_size2; l++){
			dist[k] += rcv_prob[l] * (
				POW2( (val-(*c1)[k][l]) )
				+ POW2( (*c2)[k][l] ) )
				- 2 * (*c2)[k][l] * rcv_avg[l];
		}
	}

	// Iterate through each index i that can be sent for primary source.
	// Save the one which results in lowest expected distortion at receiver.
	for(i = 0; i < c_size1; i++){
		d = 0;
		for(k = 0; k < c_size1; k++)
			d += trans_prob(i,k,src1) * dist[k];
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
	
	//TODO check dist. equation
	for(i = 0; i < Q_LEVELS; i++){
		d = nearest_neighbour(i, enc_x + i, SRC_X);
		num = 0;
		for(j = 0; j < Q_LEVELS; j++)
			num += q_tr[i][j];
		d_total += num * d;
	}

	for(j = 0; j < Q_LEVELS; j++){
		d = nearest_neighbour(j, enc_y + j, SRC_Y);
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
				p_x = trans_prob(i,k,SRC_X);
				for(j = 0; j < C_SIZE_Y; j++){
					p_y = trans_prob(j,l,SRC_Y);
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

int bsc_2_source_covq(covq *params) {
    init(params);
    // centroid_update
    // split

    // repack
    return 0; // yay
}
