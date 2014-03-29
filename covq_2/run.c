#include "covq.h"
#define SPLIT_EPS 0.001
#define SPLIT_DELTA 0.00001

double initilization_stage_covq2(covq2 *v, unif_quant *q, int N_X_final, int N_Y_final)
{
    double D_avg, D_avg2;


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
    v->x_ij[0][0] = 0;
    v->x_ij[0][0] = 0;

    for(int qx = 0; qx < v->q->L_X; qx++){
        double x = quant_to_val(qx, src_X, v->q);
        for(int qy = 0; qy < v->q->L_Y; qy++){
            int m = quantizer_get_count(qx, qy, v->q);
            double y = quant_to_val(qy, src_Y, v->q);
            v->x_ij[0][0] += x * m;
            v->y_ij[0][0] += y * m;
        }
    }

    v->x_ij[0][0] /= v->q->npoints;
    v->y_ij[0][0] /= v->q->npoints;

    // Here we set the encoders to map onto the single codevector
    for(int qx = 0; qx < v->q->L_X; qx++)
        v->I_X[qx] = 0;

    for(int qy = 0; qy < v->q->L_Y; qy++)
        v->I_Y[qy] = 0;

    // Now we get the average distortion
    // Note that this is equal to the variance of the source.
    D_avg = dist1(v);

    /*
     * Step 3: Perform Splitting Algorithm for X
     */

    while( v->N_X < N_X_final ){

        // Perform split
        for(int i = 0; i < v->N_X; i++){
            for(int j = 0; j < v->N_Y; j++){
            double x_ij = v->x_ij[i][j];
            double y_ij = v->y_ij[i][j];

            v->x_ij[i][j] = (1+SPLIT_EPS)*x_ij;
            v->y_ij[i][j]= y_ij;
            
            v->x_ij[i+v->N_X][j] = (1-SPLIT_EPS)*x_ij;
            v->y_ij[i+v->N_X][j] = y_ij;
            }
        }

        // Get N_X = 2 * N_X
        v->N_X *= 2;

        do{
            // Old distortion
            D_avg2 = D_avg;

            D_avg = update1(v);
            // printf("N_X=%d,N_Y=%d,D_avg=%f,D_avg2=%f\n",v->N_X,v->N_Y,D_avg,D_avg2);
            // assert(D_avg < D_avg2);
        }while((D_avg2-D_avg) > SPLIT_DELTA * D_avg2);

    }

    /*
     * Step 4: Perform Splitting Algorithm for Y
     */

    while( v->N_Y < N_Y_final ){

        // Perform split
        for(int i = 0; i < v->N_X; i++){
            for(int j = 0; j < v->N_Y; j++){
            double x_ij = v->x_ij[i][j];
            double y_ij = v->y_ij[i][j];

            v->x_ij[i][j] = x_ij;
            v->y_ij[i][j] = (1+SPLIT_EPS)*y_ij;

            v->x_ij[i][j+v->N_Y] = x_ij;
            v->y_ij[i][j+v->N_Y] = (1+SPLIT_EPS)*y_ij;
            }
        }

        // Get N_Y = 2 * N_Y
        v->N_Y *= 2;

        do{
            // Old distortion
            D_avg2 = D_avg;

            D_avg = update1(v);
            // printf("N_X=%d,N_Y=%d,D_avg=%f,D_avg2=%f\n",v->N_X,v->N_Y,D_avg,D_avg2);
            // assert(D_avg < D_avg2);
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

