#include "covq.h"
#define POW2(x) (x*x)

quant q_trset;
int encoder_x[Q_LEVELS];
int encoder_y[Q_LEVELS];
codevectors cv_x;
codevectors cv_y; 
int bin_cw_x[CODEBOOK_SIZE_X];
int bin_cw_y[CODEBOOK_SIZE_Y];

double channel_prob(int i, int j, int k, int l) {
    double p = 1;
    int n, diff;
    int i_cw, j_cw, k_cw, l_cw;
    i_cw = bin_cw_x[i];
    j_cw = bin_cw_y[j];
    k_cw = bin_cw_x[k];
    l_cw = bin_cw_y[l];
    
    diff = i_cw ^ k_cw;
    for(n = 0; n < CODEWORD_LEN_X; n++)
        p *= (diff>>n & 1) ? TRANS_PROB_X: (1-TRANS_PROB_X);
    diff = j_cw ^ l_cw;
    for(n = 0; n < CODEWORD_LEN_Y; n++)
        p *= (diff>>n & 1) ? TRANS_PROB_Y: (1-TRANS_PROB_Y);
    return p;
}

double nearest_neighbour(int q_lvl, int *index, int src) {
	double src_prob[CODEBOOK_SIZE_MAX], src_expected_val[CODEBOOK_SIZE_MAX];
	double d, d_best, src_var, val;
	int i, j, k, l, src1, src2, num, count, c_size1, c_size2, *enc2;
	codevectors *c1, *c2;

	val = quant_to_vec(q_lvl, src);

	// set primary and secondary source parameters
	if(src == SRC_X){
		c_size1 = CODEBOOK_SIZE_X;
		c_size2 = CODEBOOK_SIZE_Y;
		c1 = &cv_x;
		c2 = &cv_y;
		src1 = SRC_X;
		src2 = SRC_Y;
		enc2 = encoder_y;
	}else{
		c_size1 = CODEBOOK_SIZE_Y;
		c_size2 = CODEBOOK_SIZE_X;
		c1 = &cv_y;
		c2 = &cv_x;
		src1 = SRC_Y;
		src2 = SRC_X;
		enc2 = encoder_x;
	}
	
	for(i = 0; i < Q_LEVELS; i++){
		if(src1 == SRC_X)
			num = q_trset[q_lvl][i];
		else
			num = q_trset[i][q_lvl];
		j = enc2[i]; // Encoding index
		val = quant_to_vec(i, src2); // value

		count += num;
		src_prob[j] += num;
		src_expected_val[j] += num * val;
		src_var += num * val * val;
	}
	src_var /= count;
	for(j = 0; j < c_size2; j++){
		src_expected_val[j] /= src_prob[j];
		src_prob[j] /= count;
	}

	d_best = -1;
	*index = 0;	
	for(i = 0; i < c_size1; i++){
		d = src_var;
		for(j = 0; j < c_size1; j++)
			for(k = 0; k < c_size1; k++)
				for(l = 0; l < c_size1; l++)
					d += (POW2(val - (*c1)[k][l])
							-2 * (*c2)[k][l] * src_expected_val[j]
							+ POW2((*c2)[k][l]))
						*src_prob[j] * channel_prob(i,j,k,l);
		if(d_best < 0 || d < d_best){
			d_best = d;
			*index = i;
		}
	}
	return d_best;
}

double nn_update(){
	double d_total, d;
	int i, j, num;
	
	//TODO check dist. equation
	for(i = 0; i < Q_LEVELS; i++){
		d = nearest_neighbour(i, encoder_x + i, SRC_X);
		num = 0;
		for(j = 0; j < Q_LEVELS; j++)
			num += q_trset[i][j];
		d_total += num * d;
	}

	for(j = 0; j < Q_LEVELS; j++){
		d = nearest_neighbour(j, encoder_y + j, SRC_Y);
		num = 0;
		for(i = 0; i < Q_LEVELS; i++)
			num += q_trset[i][j];
		d_total += num * d;
	}

	return d_total / trset_size;
}

/* Updates the centroids for a given source by applying the centroid condition. */
void centroid_update(int src) {
	int count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	double sum[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	int num, i, j, k, l, q_val1, q_val2, src1, src2, *enc1, *enc2, c_size1, c_size2;
	double c, val, p, numer, denom;
    codevectors *cv1;

	if(src == SRC_X){
		c_size1 = CODEBOOK_SIZE_X;
		c_size2 = CODEBOOK_SIZE_Y;
		enc1 = encoder_x;
		enc2 = encoder_y;
        cv1 = &cv_x;
	}else{
		c_size1 = CODEBOOK_SIZE_Y;
		c_size2 = CODEBOOK_SIZE_X;
		enc1 = encoder_y;
		enc2 = encoder_x;
        cv1 = &cv_y;
	}
    
    for(q_val1 = 0; q_val1 < Q_LEVELS; q_val1++){
        i = enc1[q_val1];
        val = quant_to_vec(q_val1, src);
        for(q_val2 = 0; q_val2 < Q_LEVELS; q_val2++){
            j = enc2[q_val2];
            if(src1 == SRC_X)
                num = q_trset[q_val1][q_val2];
            else
                num = q_trset[q_val2][q_val1];
            count[i][j] += num;
            sum[i][j] += num * val;
        }
    }

	for(k = 0; k < c_size1; k++){
		for(l = 0; l < c_size2; l++){
			numer = 0;
			denom = 0;
			for(i = 0; i < c_size1; i++){
				for(j = 0; j < c_size2; j++){
                    p = channel_prob(i,j,k,l);
					numer += p * sum[i][j];
					denom += p * count[i][j];
				}
			}
			(*cv1)[k][l] = numer / denom;
		}
	}
}
