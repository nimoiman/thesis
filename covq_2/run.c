#include "covq.h"
#define SPLIT_EPS 0.0001
#define SPLIT_DELTA 0.0001

double initilization_stage_covq2(covq2 *v, unif_quant *q, int N_X_final, int N_Y_final)
{
    int qx, qy;
    int m;
    double x_ij, y_ij;
    double x, y;
    double D_avg, D_avg2;
    int i, j;


    /*
     * Initialize covq2 structure. Assume quantizer initialized and filled
     * already (step 1).
     */
    malloc_covq2(v, N_X_final, N_Y_final, q->L_X, q->L_Y);
    v->q = q;
    
    /*
     * Step 2: initialize 1st codevector
     */

    // Set N_X = N_Y = 1, store final values.
    v->N_X = 1;
    v->N_Y = 1;

    // Set codevector
    x_ij = 0;
    y_ij = 0;

    for(qx = 0; qx < v->q->L_X; qx++){
        x = quant_to_val(qx, src_X, v->q);
        for(qy = 0; qy < v->q->L_Y; qy++){
            m = quantizer_get_count(qx, qy, v->q);
            y = quant_to_val(qy, src_Y, v->q);
            x_ij += x * m;
            y_ij += y * m;
        }
    }

    x_ij /= v->q->npoints;
    y_ij /= v->q->npoints;

    v->x_ij[CI(0,0)] = x_ij;
    v->y_ij[CI(0,0)] = y_ij;

    // Here we set the encoders to map onto the single codevector
    for(qx = 0; qx < v->q->L_X; qx++)
        v->I_X[qx] = 0;

    for(qy = 0; qy < v->q->L_Y; qy++)
        v->I_Y[qy] = 0;

    // Now we get the average distortion
    // Note that this is equal to the variance of the source.
    D_avg = 0;

    for(qx = 0; qx < v->q->L_X; qx++){
        x = quant_to_val(qx, src_X, v->q);
        for(qy = 0; qy < v->q->L_Y; qy++){
            m = quantizer_get_count(qx, qy, v->q);
            y = quant_to_val(qy, src_Y, v->q);
            D_avg += (POW2(x-x_ij) + POW2(y-y_ij))*m;
        }
    }

    D_avg /= v->q->npoints;

    /*
     * Step 3: Perform Splitting Algorithm for X
     */

    while( v->N_X < N_X_final ){

        // Perform split
        for(i = 0; i < v->N_X*v->N_Y; i++){
            x_ij = v->x_ij[i];
            y_ij = v->y_ij[i];

            v->x_ij[i] = (1+SPLIT_EPS)*x_ij;
            v->y_ij[i] = y_ij;
            
            v->x_ij[i + v->N_X*v->N_Y] = (1-SPLIT_EPS)*x_ij;
            v->y_ij[i + v->N_X*v->N_Y] = y_ij;
        }

        // Get N_X = 2 * N_X
        v->N_X *= 2;

        do{
            // Old distortion
            D_avg2 = D_avg;

            D_avg = update1(v);

        }while((D_avg2-D_avg) > SPLIT_DELTA * D_avg2);
    }

    /*
     * Step 4: Perform Splitting Algorithm for Y
     */

    while( v->N_Y < N_Y_final ){

        // Perform split
        for(i = 0; i < v->N_X*v->N_Y; i++){
            x_ij = v->x_ij[i];
            y_ij = v->y_ij[i];

            v->x_ij[i] = x_ij;
            v->y_ij[i] = (1+SPLIT_EPS)*y_ij;

            v->x_ij[i + v->N_X*v->N_Y] = x_ij;
            v->y_ij[i + v->N_X*v->N_Y] = (1-SPLIT_EPS)*y_ij;
        }

        // Get N_Y = 2 * N_Y
        v->N_Y *= 2;

        do{
            // Old distortion
            D_avg2 = D_avg;

            D_avg = update1(v);

        }while((D_avg2-D_avg) > SPLIT_DELTA * D_avg2);
    }

    return D_avg;
}

void channel_optimization_stage_covq2(covq2 *v)
{
    double D_avg, D_avg2;

    D_avg2 = -1;
    do{
        D_avg = D_avg2;
        D_avg2 = update2(v);
    }while(D_avg == -1 || (D_avg-D_avg2) > SPLIT_DELTA * D_avg2);

}

