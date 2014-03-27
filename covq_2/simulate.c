#include "covq.h"
#define LINE_LEN 512

double simulate( sim_covq2 *s, params_covq2 *p, covq2 *c){
    int i,j,k,l;
    int count = 0;
    int q_lvl;
    double d_total = 0, x, y, x_hat, y_hat;

    for(int loop = 0; loop < s->simset_size; loop++){
        x = s->simset_x[loop];
        y = s->simset_y[loop];
        
        // Get transmission indexes
        q_lvl = quant_to_vec(x, SRC_X, p);
        nearest_neighbour(q_lvl, &i, 0, SRC_X, c, p);
        q_lvl = quant_to_vec(y, SRC_Y, p);
        nearest_neighbour(q_lvl, &j, 0, SRC_Y, c, p);
        
        // Transmit and decode
        s->channel_sim(i, j, &k, &l);
        x_hat = c->codevec_x[CV_IDX(k,l)];
        y_hat = c->codevec_x[CV_IDX(k,l)];

        // Measure distortion
        d_total += POW2(x-x_hat) + POW2(y-y_hat);
    }
    
    // return average distortion
        return d_total / count;
}

