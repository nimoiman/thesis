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
double trans_prob(int i, int j, int k, int l) {
	double p = 1;
	int n, diff;

	diff = i ^ k;
	for(n = 0; n < split_x; n++)
		p *= (diff>>n & 1) ? TRANS_PROB_X: (1-TRANS_PROB_X);
	diff = j ^ l;
	for(n = 0; n < split_y; n++)
		p *= (diff>>n & 1) ? TRANS_PROB_Y: (1-TRANS_PROB_Y);
	return p;

/* for a given source vector, returns the nearest code vector and respective
 * distortion with respect to the dist() function. The source quantization
 * level should be passed, along with a pointer which will point to the nearest
 * code vector index, and the source. */
double nearest_neighbour(int q_lvl, int *index, int src) {
	int src_count[C_SIZE_MAX];
	double src_prob[C_SIZE_MAX], src_expected_val[C_SIZE_MAX];
	double p, d, d_best, var, val;
	int i, j, k, l, src1, src2, num, count, i_best, c_size1, c_size2, *enc2;
	c_book *c1, *c2;

	val = quant_to_vec(q_lvl, src);

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
	
	for(i = 0; i < Q_LEVELS; i++){
		if(src1 == SRC_X)
			num = q_tr[q_lvl][i];
		else
			num = q_tr[i][q_lvl];
		j = enc2[i]; // Encoding index
		val = quant_to_vec(i, src2); // value

		count += num;
		src_prob[j] += num;
		src_expected_val[j] += num * val;
		src_var += num * val * val;
	}
	var /= count;
	for(j = 0; j < c_size2; j++){
		src_expected_val[j] /= src_prob[j];
		src_prob[j] /= count;
	}

	d_best = -1;
	*idx = 0;	
	for(i = 0; i < c_size1; i++){
		d = var;
		for(j = 0; j < c_size1; j++)
			for(k = 0; k < c_size1; k++)
				for(l = 0; l < c_size1; l++)
					d += (POW2(val - (*c1)[k][l])
							-2 * (*c2)[k][l] * src_expected_val[j]
							+ POW2((*c2)[k][l]))
						*src_prob[j] * trans_prob(i,j,k,l);
		if(d_best < 0 || d < d_best){
			d_best = d;
			*idx = i;
		}
	}
	return d_best;
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
