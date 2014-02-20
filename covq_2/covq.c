#include "covq.h"

double trans_prob(int i, int j, int k, int l) {
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

/* for a given source vector, returns the nearest code vector and respective
 * distortion with respect to the dist() function. The source quantization
 * level should be passed, along with a pointer which will point to the nearest
 * code vector index, and the source. */
double nearest_neighbour(int q_lvl, int *index, int src) {
    int src_count[CODEBOOK_SIZE_MAX];
    double src_sum[CODEBOOK_SIZE_MAX], rcv_prob[CODEBOOK_SIZE_MAX], rcv_avg[CODEBOOK_SIZE_MAX];
    double p, d, d_best, var, val, d_term, *dist;
    int i, j, k, l, src1, src2, num, c, count, i_best, c_size1, c_size2, *enc2;
    double (*c1)[FINAL_C_SIZE_Y], (*c2)[FINAL_C_SIZE_Y];

    // set primary and secondary source parameters
    if(src == SRC_X){
        c_size1 = C_SIZE_X;
        c_size2 = C_SIZE_Y;
        c1 = c_x;
        c2 = c_y;
        src1 = SRC_X;
        src2 = SRC_Y;
        enc2 = enc_y;
    }else{

    }
    
    // for src1 == SRC_X, compute:
    // E[Y^2 | X = x] (var)
    // number of points that map to each J index (src_count)
    // sum of points that map to each J index (src_sum)
    for(i = 0; i < Q_LEVELS; i++){
        if (src1 == SRC_X) {
            num = q_tr[q_lvl][i];
        }
        else {
            num = q_tr[i][q_lvl];
        }
        c = enc2[i];
        val = quant_to_vec(i, src2);

        count += num;
        src_count[c] += num;
        src_sum[c] += num * val;
        var += num * val * val;
    }
    var /= count;
    
    // for src1 == SRC_X, compute:
    // P(L = l | X = x) for all l (rcv_prob)
    // E[Y | L = l, X = x] for all l (rcv_avg)
    // TODO check above (rcv_avg)
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
    for(k = 0; k < c_size1; k++){
        dist[k] = 0;
        for(l = 0; l < c_size2; l++){
            d_term = (val-c1[k][l]) * (val-c1[k][l])
                + c2[k][l] * c2[k][l]
                - 2 * c2[k][l] * rcv_avg[l];
            dist[k] += rcv_prob[l] * d_term;
        }
    }

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

/* Updates the centroids for a given source by applying the centroid condition. */
void centroid_update(int src) {
    int count[C_SIZE_X][C_SIZE_Y];
    double sum[C_SIZE_X][C_SIZE_Y];
    int num, i, j, k, l, idx_x, idx_y;
    double c, val_x, d_x, d_y, d_total, p_x, p_y, numer, denom;

    for(i = 0; i < C_SIZE_X; i++){
        idx_x = enc_x[i];
        val_x = quant_to_vec(i, src);
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
