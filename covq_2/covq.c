#include "covq.h"

/* Finds the index i of minimum expected distortion for a given value of x.
 * For a given x quantization level (q_lvl_x), find the index i (*index) that
 * would result in the lowest expected distortion. The expected distortion is
 * returned.
 * Asserts:
 * q_lvl_x is a valid quantization level
*/
double nearest_neighbour(int qlvl1, int *idx, int init, int src, covq2 *c, params_covq2 *p) {
	double prob2[MAX_CODEBOOK_SIZE], expected_val2[MAX_CODEBOOK_SIZE];
	double d, d_best, var2=0, val2, val1;
	int i, j, k, l, num, count=0, qlvl2;

    int *encoder2;
    int codebook_size1, codebook_size2;
    int src1, src2;
    double *codevec1, *codevec2;

    /*
     * Initialize source variables.
     * '1' suffice indicates the primary source and '2' indicates the secondary
     * source. That is, if src == SRC_X, '1' corresponds to SRC_X and '2'
     * corresponds to SRC_Y. If src == SRC_Y, '1' corresponds to SRC_Y and '2'
     * corresponds to SRC_X.
     */
    if(src == SRC_X){
        encoder2 = c->encoder_y;
        codebook_size1 = CODEBOOK_SIZE_X;
        codebook_size2 = CODEBOOK_SIZE_Y;
        codevec1 = c->codevec_x;
        codevec2 = c->codevec_y;
        src1 = SRC_X;
        src2 = SRC_Y;
    }
    else{
        encoder2 = c->encoder_x;
        codebook_size1 = CODEBOOK_SIZE_Y;
        codebook_size2 = CODEBOOK_SIZE_X;
        codevec1 = c->codevec_y;
        codevec2 = c->codevec_x;
        src1 = SRC_Y;
        src2 = SRC_X;
    }

    /*
     * Set variables to zero
     */
    for(i = 0; i < codebook_size2; i++){
        prob2[i] = 0;
        expected_val2[i] = 0;
    }

    // assert(IN_RANGE(qlvl1, 0, p->qlvls-1));
    
    /*
     * Compute var2, expected_val2, and prob2.
     * var2 should be the condition variance of source 2 given the value of
     * source 1. Expected_val2 should be the expected value of source 2
     * conditioned on the value of source 1 and the transmitted index for
     * source 2. Prob2 should equal the probability of transmitting an index
     * from source 2.
     */
    if(!init){
        for(qlvl2 = 0; qlvl2 < p->qlvls; qlvl2++){
            if(src == SRC_X)
                num = c->qtrset[TS_IDX(qlvl1,qlvl2)];
            else
                num = c->qtrset[TS_IDX(qlvl2,qlvl1)];
            j = encoder2[qlvl2]; // Encoding index
            val2 = quant_to_vec(qlvl2, src2, p); // value

            count += num;
            prob2[j] += num;
            expected_val2[j] += num * val1;
            var2 += num * POW2(val1);
        }
        if(count == 0) return -1;
        var2 /= count;
        for(j = 0; j < codebook_size2; j++){
            if(prob2[j] > 0)
                expected_val2[j] /= prob2[j];
            else
                expected_val2[j] = 0;
            prob2[j] /= count;
        }
    }
    else{
        for(qlvl2 = 0; qlvl2 < p->qlvls; qlvl2++){
            if(src == SRC_X)
                num = c->qtrset[TS_IDX(qlvl1,qlvl2)];
            else
                num = c->qtrset[TS_IDX(qlvl2,qlvl1)];
            val1 = quant_to_vec(qlvl2, src2, p); // value

            count += num;
            expected_val2[0] += num * val1;
            var2 += num * POW2(val1);
        }
        if(count == 0){
            return -1;    
        }
        var2 /= count;
        for(j = 0; j < codebook_size2; j++){
            expected_val2[j] = expected_val2[0] / count;
            prob2[j] = ((double) 1) / codebook_size2;
        }
    }

    /*
     * Find the best transmition index for qlvl1.
     * Loop through all transmission indexes (i) to find the one which
     * minimizes expected distortion at the receiver. The expected distortion
     * is computed for each transmission index and the best one is stored in
     * d_best. The best index is sent to *idx. We use the equation illustrated
     * in the paper to compute the distortion.
     */
	val1 = quant_to_vec(qlvl1, src, p);
    d_best = -1;
	for(i = 0; i < codebook_size1; i++){
		d = var2;
		for(j = 0; j < codebook_size2; j++){
			for(k = 0; k < codebook_size1; k++){
				for(l = 0; l < codebook_size2; l++){
                    if(src == SRC_X)
                        d += (POW2(val1 - codevec1[CV_IDX(k,l)])
                                -2 * codevec2[CV_IDX(k,l)] * expected_val2[j]
                                + POW2(codevec2[CV_IDX(k,l)]))
                            *prob2[j] * p->transition_prob(i,j,k,l,p,c);
                    else
                        d += (POW2(val1 - codevec1[CV_IDX(k,l)])
                                -2 * codevec2[CV_IDX(k,l)] * expected_val2[j]
                                + POW2(codevec2[CV_IDX(k,l)]))
                            *prob2[j] * p->transition_prob(i,j,k,l,p,c);
                }
            }
        }
		if(d_best < 0 || d < d_best){
			d_best = d;
			*idx = i;
		}
	}

    // assert(d >= -0.5);
	return d_best;
}

/* Uses the nearest neighbour condition to encode each quantization level for
 * source X and source Y. */
double nn_update(int init, covq2 *c, params_covq2 *p){
	double d_total, d;
	int qlvl_x, qlvl_y, num;
	
	for(qlvl_x = 0; qlvl_x < p->qlvls; qlvl_x++){
		nearest_neighbour(qlvl_x, c->encoder_x + qlvl_x, init, SRC_X, c, p);
	}

    d_total = 0;
	for(qlvl_y = 0; qlvl_y < p->qlvls; qlvl_y++){
		d = nearest_neighbour(qlvl_y, c->encoder_y + qlvl_y, init, SRC_Y, c, p);
		num = 0;
		for(qlvl_x = 0; qlvl_x < p->qlvls; qlvl_x++)
			num += c->qtrset[TS_IDX(qlvl_x,qlvl_y)];
		d_total += num * d;
	}

	return d_total / p->trset_size;
}

/* Updates the codevectors for the X source using the centroid condition */
void centroid_update(int src, covq2 *c, params_covq2 *p) {
	int count[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
	double sum[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
	int num, i, j, k, l, qval1, qval2;
	double val1, prob, numer, denom;

    int *encoder1, *encoder2;
    int codebook_size1, codebook_size2;
    double *codevec1;

    /*
     * Set parameters based on src value
     */
    if(src == SRC_X){
        encoder1 = c->encoder_x;
        encoder2 = c->encoder_y;
        codebook_size1 = CODEBOOK_SIZE_X;
        codebook_size2 = CODEBOOK_SIZE_Y;
        codevec1 = c->codevec_x;
    }
    else{
        encoder1 = c->encoder_y;
        encoder2 = c->encoder_x;
        codebook_size1 = CODEBOOK_SIZE_Y;
        codebook_size2 = CODEBOOK_SIZE_X;
        codevec1 = c->codevec_y;
    }
    

    /*
     * Set count and sum to zero. We don't have to iterate across
     * MAX_CODEBOOK_SIZE since we don't use the whole array.
     */
    for(i = 0; i < codebook_size1; i++){
        for(j = 0; j < codebook_size2; j++){
            count[i][j] = 0;
            sum[i][j] = 0;
        }
    }

    /*
     * Compute count and sum. Count[i][j] should equal the total count of all
     * the training set pairs that map onto transmission indexes i and j.
     * Sum[i][j] should equal the total sum of all the training set pairs hat
     * map onto transmission indexes i and j.
     */
    for(qval1 = 0; qval1 < p->qlvls; qval1++){
        i = encoder1[qval1];
        val1 = quant_to_vec(qval1, src, p);
        for(qval2 = 0; qval2 < p->qlvls; qval2++){
            j = encoder2[qval2];
            if(src == SRC_X)
                num = c->qtrset[TS_IDX(qval1,qval2)];
            else
                num = c->qtrset[TS_IDX(qval2,qval1)];
            count[i][j] += num;
            sum[i][j] += num * val1;
        }
    }

    /*
     * Compute new centroid by placing them at the expected values of the
     * source conditioned on the received indexes k, l.
     */
	for(k = 0; k < codebook_size1; k++){
		for(l = 0; l < codebook_size2; l++){
			numer = 0;
			denom = 0;
			for(i = 0; i < codebook_size1; i++){
				for(j = 0; j < codebook_size2; j++){
                    if(src == SRC_X)
                        prob = p->transition_prob(i,j,k,l,p,c);
                    else
                        prob = p->transition_prob(j,i,l,k,p,c);
					numer += prob * sum[i][j];
					denom += prob * count[i][j];
				}
			}
			codevec1[CV_IDX(k,l)] = numer / denom;
		}
	}
}

