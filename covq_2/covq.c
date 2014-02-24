#include "covq.h"

quant_lvls q_trset;
encoder encoder_x;
encoder encoder_y;
codevectors cv_x;
codevectors cv_y; 
codewords_x bin_cw_x;
codewords_y bin_cw_y;

/* Finds the index i of minimum expected distortion for a given value of x.
 * For a given x quantization level (q_lvl_x), find the index i (*index) that
 * would result in the lowest expected distortion. The expected distortion is
 * returned.
 * Asserts:
 * q_lvl_x is a valid quantization level
*/
double nearest_neighbour_x(int q_lvl_x, int *index, int init) {
	double prob_j[CODEBOOK_SIZE_Y], expected_val_j[CODEBOOK_SIZE_Y];
	double d, d_best, var_y, val_x, val_y;
	int i, j, k, l, num, count, q_lvl_y;    

    assert(IN_RANGE(q_lvl_x, 0, Q_LEVELS-1));

    count = 0;
    for(i = 0; i < CODEBOOK_SIZE_Y; i++){
        prob_j[i] = 0;
        expected_val_j[i] = 0;
    }

    var_y = 0;
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

    assert(d_best >= 0);
	return d_best;
}

/* Finds the index i of minimum expected distortion for a given value of y.
 * For a given y quantization level (q_lvl_y), find the index i (*index) that
 * would result in the lowest expected distortion. The expected distortion is
 * returned.
 * Asserts:
 * q_lvl_y is a valid quantization level
*/
double nearest_neighbour_y(int q_lvl_y, int *index, int init) {
	double prob_i[CODEBOOK_SIZE_X], expected_val_i[CODEBOOK_SIZE_X];
	double d, d_best, var_x, val_x, val_y;
	int i, j, k, l, num, count, q_lvl_x;

    assert(IN_RANGE(q_lvl_y, 0, Q_LEVELS-1));

    count = 0;
    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        prob_i[i] = 0;
        expected_val_i[i] = 0;
    }

    var_x = 0;
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

    assert(d_best >= 0);
	return d_best;
}

/* Finds the index of lowest expected distortion for a given source and quantization level.
 * Calls nearest_neighbour_x or nearest_neighbour_y depending on the value of src.
 */
double nearest_neighbour(int q_lvl, int *index, int init, int src){
    if(src == SRC_X)
        return nearest_neighbour_x(q_lvl,index,init);
    else
        return nearest_neighbour_y(q_lvl,index,init);
}

/* Uses the nearest neighbour condition to encode each quantization level for
 * source X and source Y. */
double nn_update(int init){
	double d_total, d;
	int q_lvl_x, q_lvl_y, num;
	
	for(q_lvl_x = 0; q_lvl_x < Q_LEVELS; q_lvl_x++){
		nearest_neighbour(q_lvl_x, encoder_x + q_lvl_x, init, SRC_X);
	}

    d_total = 0;
	for(q_lvl_y = 0; q_lvl_y < Q_LEVELS; q_lvl_y++){
		d = nearest_neighbour(q_lvl_y, encoder_y + q_lvl_y, init, SRC_Y);
		num = 0;
		for(q_lvl_x = 0; q_lvl_x < Q_LEVELS; q_lvl_x++)
			num += q_trset[q_lvl_x][q_lvl_y];
		d_total += num * d;
	}

	return d_total / trset_size;
}

/* Updates the codevectors for the X source using the centroid condition */
void centroid_update_x() {
	int count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	double sum[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	int num, i, j, k, l, q_val_x, q_val_y;
	double val_x, p, numer, denom;

    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            count[i][j] = 0;
            sum[i][j] = 0;
        }
    }

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

/* Updates the codevectors for the Y source using the centroid condition */
void centroid_update_y() {
	int count[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	double sum[CODEBOOK_SIZE_X][CODEBOOK_SIZE_Y];
	int num, i, j, k, l, q_val_x, q_val_y;
	double val_y, p, numer, denom;


    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            count[i][j] = 0;
            sum[i][j] = 0;
        }
    }

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

