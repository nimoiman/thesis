#include "covq.h"

double nearest_neighbour1_x(int qx, int *idx, covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE];
    int MM = 0;
    double S[MAX_CODEBOOK_SIZE];

    /*
     * Zero Initialize arrays
     */
    for (int j = 0; j < v->N_Y; j++) {
        S[j] = 0;
        M[j] = 0;
    }

    double T = 0;
    for (int qy = 0; qy < v->q->L_Y; qy++) {
        double y = quant_to_val(qy, src_Y, v->q);
        int j = v->I_Y[qy];
        double m = quantizer_get_count(qx, qy, v->q);
        assert(m >= 0);

        MM += m;
        M[j] += m;
        S[j] += y * m;
        T += POW2(y) * m;
    }
    assert(T >= 0);

    if (MM == 0) {
        *idx = 0;
        return -1;
    }

    double d_best = -1;
    double x = quant_to_val(qx, src_X, v->q);
    for (int i = 0; i < v->N_X; i++) {
        double d = 0;
        for (int j = 0; j < v->N_Y; j++) {
            double x_ij = v->x_ij[CI(i,j)];
            double y_ij = v->y_ij[CI(i,j)];
            d += (POW2(x-x_ij)+POW2(y_ij))*M[j]-2*y_ij*S[j];
            assert(M[j] >= 0);
        }
        d = (T+d)/MM;
        if (d_best < 0 || d < d_best) {
            *idx = i;
            d_best = d;
        }
    }

    assert(d_best >= -0.5);
    return d_best;
}

double nearest_neighbour1_y(int qy, int *idx, covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE];
    int MM = 0;
    double S[MAX_CODEBOOK_SIZE];

    /*
     * Zero Initialize arrays
     */
    for (int i = 0; i < v->N_X; i++) {
        S[i] = 0;
        M[i] = 0;
    }

    double T = 0;
    for (int qx = 0; qx < v->q->L_X; qx++) {
        double x = quant_to_val(qx, src_X, v->q);
        int i = v->I_X[qx];
        int m = quantizer_get_count(qx, qy, v->q);
        assert(m >= 0);

        MM += m;
        M[i] += m;
        S[i] += x * m;
        T += POW2(x) * m;
    }
    assert(T >= 0);

    if(MM == 0){
        *idx = 0;
        return -1;
    }

    double d_best = -1;
    double y = quant_to_val(qy, src_Y, v->q);
    for (int j = 0; j < v->N_Y; j++) {
        double d = 0;
        for (int i = 0; i < v->N_X; i++) {
            double x_ij = v->x_ij[CI(i,j)];
            double y_ij = v->y_ij[CI(i,j)];
            d += (POW2(y-y_ij)+POW2(x_ij))*M[i]-2*x_ij*S[i];
            assert(M[i] >= 0);
        }
        d = (T+d)/MM;
        if (d_best < 0 || d < d_best) {
            *idx = j;
            d_best = d;
        }
    }

    assert(d_best >= -0.5);
    return d_best;
}

double nearest_neighbour2_x(int qx, int *idx, covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE];
    int MM = 0;
    double S[MAX_CODEBOOK_SIZE];
    double T = 0;

    /*
     * Zero Initialize arrays
     */
    for(int j = 0; j < v->N_Y; j++){
        S[j] = 0;
        M[j] = 0;
    }

   for(int qy = 0; qy < v->q->L_Y; qy++){
        double y = quant_to_val(qy, src_Y, v->q);
        int j = v->I_Y[qy];
        int m = quantizer_get_count(qx, qy, v->q);

        MM += m;
        M[j] += m;
        S[j] += y * m;
        T += POW2(y) * m;
    }

    if(MM == 0){
        *idx = 0;
        return -1;
    }

    double d_best = -1;
    double x = quant_to_val(qx, src_X, v->q);
    for(int i = 0; i < v->N_X; i++){
        double d = 0;
        for(int j = 0; j < v->N_Y; i++){
            for(int k = 0; k < v->N_X; k++){
                for(int l = 0; l < v->N_Y; l++){
                    double x_kl = v->x_ij[CI(k,l)];
                    double y_kl = v->y_ij[CI(k,l)];
                    double p = v->trans_prob(i,j,k,l,v->b_X,v->b_Y);
                    d += ((POW2(x-x_kl)+POW2(y_kl))*M[j]-2*y_kl*S[j])*p;
                }
            }
        }
        d = (T+d)/MM;
        if(d_best < 0 || d < d_best){
            *idx = i;
            d_best = d;
        }
    }

    return d_best;
}

double nearest_neighbour2_y(int qy, int *idx, covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE];
    int MM = 0;
    double S[MAX_CODEBOOK_SIZE];
    double T = 0;

    /*
     * Zero Initialize arrays
     */
    for(int i = 0; i < v->N_X; i++){
        S[i] = 0;
        M[i] = 0;
    }

    for(int qx = 0; qx < v->q->L_X; qx++){
        double x = quant_to_val(qx, src_X, v->q);
        int i = v->I_X[qx];
        int m = quantizer_get_count(qx, qy, v->q);

        MM += m;
        M[i] += m;
        S[i] += x * m;
        T += POW2(x) * m;
    }

    if(MM == 0){
        *idx = 0;
        return -1;
    }

    double d_best = -1;
    double y = quant_to_val(qy, src_Y, v->q);
    for(int j = 0; j < v->N_Y; j++){
        double d = 0;
        for(int i = 0; i < v->N_X; i++){
            for(int k = 0; k < v->N_X; k++){
                for(int l = 0; l < v->N_Y; l++){
                    double x_kl = v->x_ij[CI(k,l)];
                    double y_kl = v->y_ij[CI(k,l)];
                    double p = v->trans_prob(i,j,k,l,v->b_X,v->b_Y);
                    d += ((POW2(y-y_kl)+POW2(x_kl))*M[i]-2*x_kl*S[i])*p;
                }
            }
        }
        d = (T+d)/MM;
        if(d_best < 0 || d < d_best){
            *idx = i;
            d_best = d;
        }
    }

    return d_best;
}

void centroid1(covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    double S_X[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    double S_Y[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    int i, j;
    int qx, qy;
    int m;
    double x, y;

    /*
     * Zero Initialize arrays
     */
    for(i = 0; i < v->N_X; i++){
        for(j = 0; j < v->N_Y; j++){
            M[i][j] = 0;
            S_X[i][j] = 0;
            S_Y[i][j] = 0;
        }
    }

    /*
     * Compute M(i,j), S_X(i,j), and S_Y(i,j)
     */
    for(qx = 0; qx < v->q->L_X; qx++){
        x = quant_to_val(qx, src_X, v->q);
        for(qy = 0; qy < v->q->L_Y; qy++){
            y = quant_to_val(qy, src_Y, v->q);
            m = quantizer_get_count(qx, qy, v->q);
            i = v->I_X[qx];
            j = v->I_Y[qy];

            M[i][j] += m;
            S_X[i][j] += x*m;
            S_Y[i][j] += y*m;
        }
    }

    /*
     * compute x_ij, and y_ij
     */
    for(i = 0; i < v->N_X; i++){
        for(j = 0; j < v->N_Y; j++){
            if(M[i][j] > 0){
                v->x_ij[CI(i,j)] = S_X[i][j] / M[i][j];
                v->y_ij[CI(i,j)] = S_Y[i][j] / M[i][j];
            }
            else{
                v->x_ij[CI(i,j)] = 0;
                v->y_ij[CI(i,j)] = 0;
            }

        }
    }
}

void centroid2(covq2 *v)
{
    int M[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    double S_X[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    double S_Y[MAX_CODEBOOK_SIZE][MAX_CODEBOOK_SIZE];
    int i, j, k, l;
    int qx, qy;
    int m;
    double x, y;
    double p;
    double numer_x, numer_y;
    double denom;


    /*
     * Zero Initialize arrays
     */
    for(i = 0; i < v->N_X; i++){
        for(j = 0; j < v->N_Y; j++){
            M[i][j] = 0;
            S_X[i][j] = 0;
            S_Y[i][j] = 0;
        }
    }

    /*
     * Compute M(i,j), S_X(i,j), and S_Y(i,j)
     */
    for(qx = 0; qx < v->q->L_X; qx++){
        x = quant_to_val(qx, src_X, v->q);
        for(qy = 0; qy < v->q->L_Y; qy++){
            y = quant_to_val(qy, src_Y, v->q);
            m = quantizer_get_count(qx, qy, v->q);
            i = v->I_X[qx];
            j = v->I_Y[qy];

            M[i][j] += m;
            S_X[i][j] += x*m;
            S_Y[i][j] += y*m;
        }
    }

    /*
     * Compute x_kl (numer_x/denom), and y_kl (numer_y/denom)
     */
    for(k = 0; k < v->N_X; k++){
        for(l = 0; l < v->N_Y; l++){
            numer_x = 0;
            numer_y = 0;
            denom = 0;
            for(i = 0; i < v->N_X; i++){
                for(j = 0; j < v->N_Y; j++){
                    p = v->trans_prob(i,j,k,l,v->b_X,v->b_Y);
                    numer_x += S_X[i][j] * p;
                    numer_y += S_Y[i][j] * p;
                    denom += M[i][j] * p;
                }
            }

            v->x_ij[CI(k,l)] = numer_x / denom;
            v->y_ij[CI(k,l)] = numer_y / denom;
        }
    }
}

double update(covq2 *v, int t)
{
    int qx, qy;
    int i, j;
    double d;
    int m;
    double d_total = 0;

    /*
     * Apply Nearest Neighbour Condition for X
     */
    for(qx = 0; qx < v->q->L_X; qx++){
        if(t == 1)
            nearest_neighbour1_x(qx, &i, v);
        else
            nearest_neighbour2_x(qx, &i, v);
        v->I_X[qx] = i;
    }

    /*
     * Apply Nearest Neighbour Condition for Y
     * We compute the expected distortion so it can be returned. Keep in mind
     * that we compute it before we run the centroid condition (since it is
     * easier this way, and it shouldn't matter that much) so the true
     * distortion should be slightly lower. We compute the average distortion
     * by going over the marginal in Y.
     */
    for(qy = 0; qy < v->q->L_Y; qy++){
        m = 0;
        for(qx = 0; qx < v->q->L_X; qx++)
            m += quantizer_get_count(qx, qy, v->q);
        if(t == 1)
            d = nearest_neighbour1_y(qy, &j, v);
        else
            d = nearest_neighbour2_y(qy, &j, v);
        v->I_Y[qy] = j;
        d_total += d * m;
    }

    if(t == 1)
        centroid1(v);
    else
        centroid2(v);

    return d_total / v->q->npoints;
}

double update1(covq2 *v)
{
    return update(v, 1);
}

double update2(covq2 *v)
{
    return update(v, 2);
}

