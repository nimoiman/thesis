#include <stdio.h>
#include "covq.h"
#define QLVLS 100
#define LINE_LEN 100

double trans_prob_x = 0.01;
double trans_prob_y = 0.01;

int main( int argc, const char* argv[] ){
    int i,j;
    FILE * pFile;
    double qlvls[QLVLS];
    params_covq2 p;
    covq2 *c;
    int trset_size = 0;
    double *trset_x, *trset_y;
    char line[LINE_LEN];
    int param_count;
    double distortion;

    /*
     * Set COVQ 2 Parameters
     */
    p.qlvls = QLVLS;
    p.qlen_x = 3;
    p.qlen_y = 3;
    p.cwlen_x = 3;
    p.cwlen_y = 3;

    /*
     * Read training set from file.
     */
    pFile = fopen("trainint_set.csv", "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }
    trset_x = (double*) malloc(TRSET_SIZE_MAX * sizeof(double));
    trset_y = (double*) malloc(TRSET_SIZE_MAX * sizeof(double));
    if(!trset_x){
        fprintf(stderr, "Could not allocate memory for training set.\n");
        free(trset_x);
        free(trset_y);
        return 0;
    }
    while(fgets(line, LINE_LEN, pFile) != NULL){
        param_count = sscanf(line, "%lf,%lf", trset_x + trset_size, trset_y + trset_size);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            free(trset_x);
            free(trset_y);
            return 0;
        }
    }

    /*
     * Set seed and run.
     * We provide the covq 2 parameters and run will return the codevectors and
     * encoder mapping in c.
     */
    srand(1);
    if( run( c, &distortion, &p ) == 0 )
        // Stop if it didn't work
        free(trset_x);
        free(trset_y);
        return 0;
    // Make sure everything is in order
    assert_globals(c, &p);

    /*
     * Print results to files
     */
    fprintf_double("x_ij.csv", (double*) c->codevec_x, 1<<p.cwlen_x, 1<<p.cwlen_x);
    fprintf_double("y_ij.csv", (double*) c->codevec_y, 1<<p.cwlen_x, 1<<p.cwlen_x);
    fprintf_int("x_encoder.csv", c->encoder_x, 1, p.qlvls);
    fprintf_int("y_encoder.csv", c->encoder_y, 1, p.qlvls);

    for(i = 0; i < p.qlvls; i++)
        qlvls[i] = quant_to_vec(i, SRC_X, &p);
    fprintf_double("qlvls_x.csv", qlvls, 1, p.qlvls);

    for(i = 0; i < p.qlvls; i++)
        qlvls[i] = quant_to_vec(i, SRC_Y, &p);
    fprintf_double("qlvls_y.csv", qlvls, 1, p.qlvls);

    destroy_covq2_struct(c);
    free(trset_x);
    free(trset_y);
    return 1;
}

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

