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

/* Channel transition probability
 * Computes the channel transition probability P(k,l | i,j). That is, the
 * probability of receiving indexes k and l from source X and Y respectively,
 * given that index i was sent from source X and j was sent from source Y. Take
 * note that the channels do not need to be independent.
 * Asserts:
 * i, j, k, and l are valid codebook indexes.
 * return value is between 0 and 1.
 */
double channel_prob(int i, int j, int k, int l, params_covq2 *p, covq2 *c){
    double prob = 1;
    int n, diff;
    int i_cw, j_cw, k_cw, l_cw;

    //Assert indecies in range
    assert(IN_RANGE(i, 0, CODEBOOK_SIZE_X-1));
    assert(IN_RANGE(j, 0, CODEBOOK_SIZE_Y-1));
    assert(IN_RANGE(k, 0, CODEBOOK_SIZE_X-1));
    assert(IN_RANGE(l, 0, CODEBOOK_SIZE_Y-1));

    // Get binary transmission codewords for the indexes
    i_cw = c->cwmap_x[i];
    j_cw = c->cwmap_y[j];
    k_cw = c->cwmap_x[k];
    l_cw = c->cwmap_y[l];
    
    // Compute transition probability
    diff = i_cw ^ k_cw;
    for(n = 0; n < p->cwlen_x; n++)
        prob *= (diff>>n & 1) ? trans_prob_x: (1-trans_prob_x);
    diff = j_cw ^ l_cw;
    for(n = 0; n < p->cwlen_y; n++)
        prob *= (diff>>n & 1) ? trans_prob_y: (1-trans_prob_y);
    
    //Assert valid probability
    assert(IN_RANGE(prob, 0, 1));

    return prob;
}

double eps_sweep(double eps0, double eps1, double eps_inc, params_covq2 *p, covq2 *c){
    double eps;
    double distortion;

    for(eps = eps0; eps > eps1; eps -= eps_inc){
        trans_prob_x = eps;
        trans_prob_y = eps;
        run(&distortion, c, p);
    }
    trans_prob_x = eps1;
    trans_prob_y = eps1;
    run(&distortion, c, p);

    return distortion;
}

int main( int argc, const char* argv[] ){
    int i,j;
    FILE * pFile;
    double qlvls[QLVLS];
    params_covq2 p;
    covq2 c;
    char line[LINE_LEN];
    int param_count;
    double distortion;
    double eps;

    /*
     * Set COVQ 2 Parameters
     */
    p.qlvls = QLVLS;
    p.qlen_x = 3;
    p.qlen_y = 3;
    p.cwlen_x = 3;
    p.cwlen_y = 3;
    p.trset_size = 0;
    p.transition_prob = &channel_prob;

    /*
     * Read training set from file.
     */
    pFile = fopen(TRSET_FILE, "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }
    p.trset_x = (double*) malloc(TRSET_SIZE_MAX * sizeof(double));
    p.trset_y = (double*) malloc(TRSET_SIZE_MAX * sizeof(double));
    if(!p.trset_x){
        fprintf(stderr, "Could not allocate memory for training set.\n");
        free(p.trset_x);
        free(p.trset_y);
        return 0;
    }
    while(fgets(line, LINE_LEN, pFile) != NULL){
        param_count = sscanf(line, "%lf, %lf", p.trset_x + p.trset_size, p.trset_y + p.trset_size);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            free(p.trset_x);
            free(p.trset_y);
            return 0;
        }
        p.trset_size++;
    }

    srand(1);
    for( eps = EPS_INIT; eps < EPS_FINAL; eps += EPS_INC ){
        init_covq2_struct( &c, &p );
        distortion = eps_sweep(SEPS_INIT, eps, SEPS_INC, &p, &c);
        destroy_covq2_struct( &c );
        printf("%lf, %lf\n", eps, distortion);
    }

    return 1;
}

