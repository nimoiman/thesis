#include <stdio.h>
#include "covq.h"
#define QLVLS 25
#define LINE_LEN 100


#define TRSET_FILE "training_set.csv"
#define ENCODER_X_FILE "encoder_x.csv"
#define ENCODER_Y_FILE "encoder_y.csv"
#define CODEVEC_X_FILE "x_ij.csv"
#define CODEVEC_Y_FILE "y_ij.csv"

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

int main( int argc, const char* argv[] ){
    int i,j;
    FILE * pFile;
    double qlvls[QLVLS];
    params_covq2 p;
    covq2 c;
    char line[LINE_LEN];
    int param_count;
    double distortion;
    double tprob;
    int tsize;

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

    tsize = p.trset_size;
    srand(1);
    for( tprob = 0.001; tprob < 0.5; tprob += 0.001 ){
        trans_prob_x = tprob;
        trans_prob_y = tprob;

        run( &c, &distortion, &p );
        printf("%lf, %lf\n", tprob, distortion);
    }

    /*
     * Set seed and run.
     * We provide the covq 2 parameters and run will return the codevectors and
     * encoder mapping in c.
     */
    srand(1);
    if( run( &c, &distortion, &p ) == 0 )
        // Stop if it didn't work
        free(p.trset_x);
        free(p.trset_y);
        return 0;
    // Make sure everything is in order
    assert_globals(&c, &p);

    /*
     * Print results to files
     */
    printf("Printing results to file...\n");
    fprintf_double("x_ij.csv", (double*) c.codevec_x, 1<<p.cwlen_x, 1<<p.cwlen_x);
    fprintf_double("y_ij.csv", (double*) c.codevec_y, 1<<p.cwlen_x, 1<<p.cwlen_x);
    fprintf_int("x_encoder.csv", c.encoder_x, 1, p.qlvls);
    fprintf_int("y_encoder.csv", c.encoder_y, 1, p.qlvls);

    for(i = 0; i < p.qlvls; i++)
        qlvls[i] = quant_to_vec(i, SRC_X, &p);
    fprintf_double("qlvls_x.csv", qlvls, 1, p.qlvls);

    for(i = 0; i < p.qlvls; i++)
        qlvls[i] = quant_to_vec(i, SRC_Y, &p);
    fprintf_double("qlvls_y.csv", qlvls, 1, p.qlvls);

    destroy_covq2_struct(&c);
    free(p.trset_x);
    free(p.trset_y);
    return 1;
}

