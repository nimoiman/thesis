#include "covq.h"

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

double nearest_neighbour_x(int q_lvl_x, int *index, int init) {
	double prob_j[CODEBOOK_SIZE_Y], expected_val_j[CODEBOOK_SIZE_Y];
	double d, d_best, var_y, val_x, val_y;
	int i, j, k, l, num, count, q_lvl_y;

    count = 0;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++){
        prob_j[i] = 0;
        expected_val_j[i] = 0;
    }

    if(!init){
        for(q_lvl_y = 0; q_lvl_y < Q_LEVELS; q_lvl_y++){
            num = q_trset[q_lvl_x][q_lvl_y];
            j = encoder_y[q_lvl_y]; // Encoding index
            val_y = quant_to_vec(q_lvl_y, SRC_Y); // value

            count += num;
            prob_j[j] += num;
            expected_val_j[j] += num * val_y;
            var_y += num * POW2(val_y);
        }
        if(count == 0) return -1;
        var_y /= count;
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            if(prob_j[j] > 0)
                expected_val_j[j] /= prob_j[j];
            else
                expected_val_j[j] = 0;
            prob_j[j] /= count;
        }
    }
    else{
        for(q_lvl_y = 0; q_lvl_y < Q_LEVELS; q_lvl_y++){
            num = q_trset[q_lvl_x][q_lvl_y];
            val_y = quant_to_vec(q_lvl_y, SRC_Y); // value

            count += num;
            expected_val_j[0] += num * val_y;
            var_y += num * POW2(val_y);
        }
        if(count == 0) return -1;
        var_y /= count;
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            expected_val_j[j] = expected_val_j[0] / count;
            prob_j[j] = ((double) 1) / CODEBOOK_SIZE_Y;
        }
    }

	val_x = quant_to_vec(q_lvl_x, SRC_X);
	d_best = -1;
	*index = 0;	
	for(i = 0; i < CODEBOOK_SIZE_X; i++){
		d = var_y;
		for(j = 0; j < CODEBOOK_SIZE_Y; j++){
			for(k = 0; k < CODEBOOK_SIZE_X; k++){
				for(l = 0; l < CODEBOOK_SIZE_Y; l++){
                    d += (POW2(val_x - cv_x[k][l])
                            -2 * cv_y[k][l] * expected_val_j[j]
                            + POW2(cv_y[k][l]))
                        *prob_j[j] * channel_prob(i,j,k,l);
                }
            }
        }
		if(d_best < 0 || d < d_best){
			d_best = d;
			*index = i;
		}
	}
    printf("var_y=%lf, d_best x=%lf\n", var_y, d_best);
	return d_best;
}

double nearest_neighbour_y(int q_lvl_y, int *index, int init) {
	double prob_i[CODEBOOK_SIZE_Y], expected_val_i[CODEBOOK_SIZE_Y];
	double d, d_best, var_x, val_x, val_y;
	int i, j, k, l, num, count, q_lvl_x;

    count = 0;
    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        prob_i[i] = 0;
        expected_val_i[i] = 0;
    }

    if(!init){
        for(q_lvl_x = 0; q_lvl_x < Q_LEVELS; q_lvl_x++){
            num = q_trset[q_lvl_x][q_lvl_y];
            i = encoder_x[q_lvl_x]; // Encoding index
            val_x = quant_to_vec(q_lvl_x, SRC_X); // value

            count += num;
            prob_i[i] += num;
            expected_val_i[i] += num * val_x;
            var_x += num * POW2(val_x);
        }
        if(count == 0) return -1;
        var_x /= count;
        for(i = 0; i < CODEBOOK_SIZE_X; i++){
            if(prob_i[i] > 0)
                expected_val_i[i] /= prob_i[i];
            else
                expected_val_i[i] = 0;
            prob_i[i] /= count;
        }
    }
    else{
        for(q_lvl_x = 0; q_lvl_x < Q_LEVELS; q_lvl_x++){
            num = q_trset[q_lvl_x][q_lvl_y];
            val_x = quant_to_vec(q_lvl_x, SRC_X); // value

            count += num;
            expected_val_i[0] += num * val_x;
            var_x += num * POW2(val_x);
        }
        if(count == 0) return -1;
        var_x /= count;
        for(i = 0; i < CODEBOOK_SIZE_X; i++){
            expected_val_i[i] = expected_val_i[0] / count;
            prob_i[i] = ((double) 1) / CODEBOOK_SIZE_X;
        }
    }

	val_y = quant_to_vec(q_lvl_y, SRC_Y);
	d_best = -1;
	*index = 0;	
	for(j = 0; j < CODEBOOK_SIZE_Y; j++){
		d = var_x;
		for(i = 0; i < CODEBOOK_SIZE_X; i++){
			for(k = 0; k < CODEBOOK_SIZE_X; k++){
				for(l = 0; l < CODEBOOK_SIZE_Y; l++){
                    d += (POW2(val_y - cv_y[k][l])
                            -2 * cv_x[k][l] * expected_val_i[i]
                            + POW2(cv_x[k][l]))
                        *prob_i[i] * channel_prob(i,j,k,l);
                }
            }
        }
		if(d_best < 0 || d < d_best){
			d_best = d;
			*index = j;
		}
	}
    printf("dvar_x=%lf, best y=%lf\n", var_x, d_best);
	return d_best;
}

double nearest_neighbour(int q_lvl, int *index, int init, int src){
    if(src == SRC_X)
        return nearest_neighbour_x(q_lvl,index,init);
    else
        return nearest_neighbour_y(q_lvl,index,init);
}

double nn_update(int init){
	double d_total, d;
	int i, j, num;
	
    printf("Encoding X...\n");
	for(i = 0; i < Q_LEVELS; i++){
		d = nearest_neighbour(i, encoder_x + i, init, SRC_X);
		num = 0;
		for(j = 0; j < Q_LEVELS; j++)
			num += q_trset[i][j];
		d_total += num * d;
	}

    printf("Encoding Y...\n");
	for(j = 0; j < Q_LEVELS; j++){
		d = nearest_neighbour(j, encoder_y + j, init, SRC_Y);
		num = 0;
		for(i = 0; i < Q_LEVELS; i++)
			num += q_trset[i][j];
		d_total += num * d;
	}

	return d_total / trset_size;
}

void centroid_update_x() {
	int count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	double sum[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	int num, i, j, k, l, q_val_x, q_val_y;
	double val_x, p, numer, denom;

    for(q_val_x = 0; q_val_x < Q_LEVELS; q_val_x++){
        i = encoder_x[q_val_x];
        val_x = quant_to_vec(q_val_x, SRC_X);
        for(q_val_y = 0; q_val_y < Q_LEVELS; q_val_y++){
            j = encoder_y[q_val_y];
            num = q_trset[q_val_x][q_val_y];
            count[i][j] += num;
            sum[i][j] += num * val_x;
        }
    }

	for(k = 0; k < CODEBOOK_SIZE_X; k++){
		for(l = 0; l < CODEBOOK_SIZE_Y; l++){
			numer = 0;
			denom = 0;
			for(i = 0; i < CODEBOOK_SIZE_X; i++){
				for(j = 0; j < CODEBOOK_SIZE_Y; j++){
                    p = channel_prob(i,j,k,l);
					numer += p * sum[i][j];
					denom += p * count[i][j];
				}
			}
			cv_x[k][l] = numer / denom;
		}
	}
}

void centroid_update_y() {
	int count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	double sum[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	int num, i, j, k, l, q_val_x, q_val_y;
	double val_y, p, numer, denom;

    for(q_val_y = 0; q_val_y < Q_LEVELS; q_val_y++){
        j = encoder_y[q_val_y];
        val_y = quant_to_vec(q_val_y, SRC_Y);
        for(q_val_x = 0; q_val_x < Q_LEVELS; q_val_x++){
            i = encoder_x[q_val_x];
            num = q_trset[q_val_x][q_val_y];
            count[i][j] += num;
            sum[i][j] += num * val_y;
        }
    }

	for(k = 0; k < CODEBOOK_SIZE_X; k++){
		for(l = 0; l < CODEBOOK_SIZE_Y; l++){
			numer = 0;
			denom = 0;
			for(i = 0; i < CODEBOOK_SIZE_X; i++){
				for(j = 0; j < CODEBOOK_SIZE_Y; j++){
                    p = channel_prob(i,j,k,l);
					numer += p * sum[i][j];
					denom += p * count[i][j];
				}
			}
			cv_y[k][l] = numer / denom;
		}
	}
}

/* Updates the centroids for a given source by applying the centroid condition. */
void centroid_update(int src) {
    if(src == SRC_X)
        centroid_update_x();
    else
        centroid_update_y();
}

