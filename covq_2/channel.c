#include "covq.h"

// Transition Probabilities for BSC
#define TRANS_PROB_X 0.01
#define TRANS_PROB_Y 0.01


/* Channel transition probability
 * Computes the channel transition probability P(k,l | i,j). That is, the
 * probability of receiving indexes k and l from source X and Y respectively,
 * given that index i was sent from source X and j was sent from source Y. Take
 * note that the channels do not need to be independent.
 * Asserts:
 * i, j, k, and l are valid codebook indexes.
 * return value is between 0 and 1.
 */
double channel_prob(int i, int j, int k, int l) {
    double p = 1;
    int n, diff;
    int i_cw, j_cw, k_cw, l_cw;

    //Assert indecies in range
    assert(IN_RANGE(i, 0, CODEBOOK_SIZE_X-1));
    assert(IN_RANGE(j, 0, CODEBOOK_SIZE_Y-1));
    assert(IN_RANGE(k, 0, CODEBOOK_SIZE_X-1));
    assert(IN_RANGE(l, 0, CODEBOOK_SIZE_Y-1));

    // Get binary transmission codewords for the indexes
    i_cw = bin_cw_x[i];
    j_cw = bin_cw_y[j];
    k_cw = bin_cw_x[k];
    l_cw = bin_cw_y[l];
    
    // Compute transition probability
    diff = i_cw ^ k_cw;
    for(n = 0; n < CODEWORD_LEN_X; n++)
        p *= (diff>>n & 1) ? TRANS_PROB_X: (1-TRANS_PROB_X);
    diff = j_cw ^ l_cw;
    for(n = 0; n < CODEWORD_LEN_Y; n++)
        p *= (diff>>n & 1) ? TRANS_PROB_Y: (1-TRANS_PROB_Y);
    
    //Assert valid probability
    assert(IN_RANGE(p, 0, 1));

    return p;
}

