#include <math.h>
#include <stdio.h>
#include "covq.h"

#define QLVLS 25
#define LINE_LEN 100

#define TRSET_FILE "training_set.csv"
#define ENCODER_X_FILE "encoder_x.csv"
#define ENCODER_Y_FILE "encoder_y.csv"
#define CODEVEC_X_FILE "x_ij.csv"
#define CODEVEC_Y_FILE "y_ij.csv"

#define EPS_INIT 0.001
#define EPS_FINAL 0.2
#define EPS_INC 0.001
#define SEPS_INIT 0.35
#define SEPS_INC 0.025

double trans_prob_x = 0.01;
double trans_prob_y = 0.01;

int B_X, B_Y;

/* Channel transition probability
 * Computes the channel transition probability P(k,l | i,j). That is, the
 * probability of receiving indexes k and l from source X and Y respectively,
 * given that index i was sent from source X and j was sent from source Y. Take
 * note that the channels do not need to be independent.
 * Asserts:
 * i, j, k, and l are valid codebook indexes.
 * return value is between 0 and 1.
 */
double trans_prob(int i, int j, int k, int l, int *b_X, int *b_Y)
{
    double prob = 1;
    int n, diff;
    int i_cw, j_cw, k_cw, l_cw;


    // Get binary transmission codewords for the indexes
    i_cw = b_X[i];
    j_cw = b_Y[j];
    k_cw = b_X[k];
    l_cw = b_Y[l];
    
    // Compute transition probability
    diff = i_cw ^ k_cw;
    for(n = 0; n < B_X; n++)
        prob *= (diff>>n & 1) ? trans_prob_x: (1-trans_prob_x);
    diff = j_cw ^ l_cw;
    for(n = 0; n < B_Y; n++)
        prob *= (diff>>n & 1) ? trans_prob_y: (1-trans_prob_y);
    
    return prob;
}

double run(int L_X, int L_Y, int N_X, int N_Y)
{
    FILE * pFile;
    covq2 v;
    unif_quant q;
    char line[LINE_LEN];
    int param_count;

    double x, y;
    int qx, qy;
    double T_X=0, S_X=0;
    double T_Y=0, S_Y=0;
    double signal_power;
    double quantized_noise_power;
    double SQNR;


    quantizer_init( &q, L_X, L_Y, -1, 1, -1, 1);

    /*
     * Read training set from file.
     */
    pFile = fopen(TRSET_FILE, "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }

    while(fgets(line, LINE_LEN, pFile) != NULL){
        param_count = sscanf(line, "%lf, %lf", &x, &y);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            return 0;
        }
        S_X += x;
        S_Y += y;
        T_X += POW2(x);
        T_Y += POW2(y);
        qx = val_to_quant(x, src_X, &q);
        qy = val_to_quant(y, src_Y, &q);
        quantizer_bin(qx, qy, &q);
    }

    S_X /= q.npoints;
    S_Y /= q.npoints;
    T_X /= q.npoints;
    T_Y /= q.npoints;
    signal_power = T_X + POW2(S_X) + T_Y + POW2(S_Y);

    quantized_noise_power = initilization_stage_covq2(&v, &q, N_X, N_Y);

    SQNR = 10 * log10(signal_power / quantized_noise_power);

    free_covq2(&v);
    quantizer_free(&q);

    return SQNR;
}

int main()
{
    int N_X, N_Y;
    int L_X, L_Y;
    double SQNR;
    int N_X_final, N_Y_final;

    N_X_final = 32;
    N_Y_final = 32;

    L_X = L_Y = 100;

    printf("\t");
    for(N_X = 1; N_X <= N_X_final; N_X *= 2)
        printf("%d\t\t", N_X);

    for(N_Y = 1; N_Y <= N_Y_final; N_Y *= 2){
        printf("\n%d", N_Y);
        for(N_X = 1; N_X <= N_X_final; N_X *= 2){
            SQNR = run(L_X, L_Y, N_X, N_Y);
            printf("\t%f",SQNR);
        }
    }

    printf("\n");

    return 1;
}

