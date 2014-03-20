#include "channel.h"
/* Defines functions which model / simulate a Gaussian channel
   (Binary Symmetric Channel) */

/* Uniform(0,1] */
double drand() {
    return (rand() + 1.0) / (RAND_MAX + 1.0);
}

double box_muller(double mean, double var) {
    double r_1, r_2; // independent Uniform (0,1)
    r_1 = drand();
    r_2 = drand();
    return sqrt(var)*(sqrt(-2*log(r_1))*cos(2*M_PI*r_2)) + mean;
}

void gaussian_channel(double *vector, uint dim, double mean, double var) {
    uint i;
    for (i = 0; i < dim; i++) {
        vector[i] += box_muller(mean, var);
    }
}

/* Return 1 on "success", 0 on "failure" */
int bernoulli(double p) {
    return (rand() < p * RAND_MAX) ? 1 : 0;
}

void binary_symmetric_channel(uint *index, double error_prob,
                              uint fixed_length) {
    uint i;
    char *ch_index = (char*) index; // go through byte at a time
    for (i = 0; i < fixed_length; i++) {
        if (bernoulli(error_prob)) {
            // flip the bit
            *ch_index ^= (1 << (i % 8));
        }
        if (i % 8 == 7) {
            // next byte
            ch_index++;
        }
    }
}

double _channel_prob(uint d, uint length, double error_prob) {
    // static const int n_buckets = 8;
    /* Caching assumes error_prob never changes */
    static struct {
        uint d;
        uint length;
        double result;
    } cache[8];
    /* Stupidly special case */
    if (length == 0) {
        return 1;
    }

    static uint last_read_k = 0;
    static uint last_written_k = 0;
    uint k = last_read_k;
    /* look in cache for result */
    do {
        if (cache[k].d == d && cache[k].length == length) {
            return cache[k].result;
        }
        k = (k + 1) % 8;
    } while (k != last_read_k);
    last_read_k = last_written_k = (last_written_k + 1) % 8;
    cache[last_written_k].d = d;
    cache[last_written_k].length = length;
    cache[last_written_k].result = pow(error_prob, d) * pow(1-error_prob,
        length - d);

    return cache[last_written_k].result;
}

/* Return probability of channel index cw_map[i] transitioning to cw_map[j] 
 * over BSC with epsilon = error_prob
 */
double channel_prob(uint i, uint j, double error_prob, uint length,
                    uint *cw_map) {
    // printf("prob(%u|%u)=%f\n", cw_map[i], cw_map[j], _channel_prob(cw_map[i], cw_map[j], length, error_prob));
    uint d = hamming_distance(cw_map[i], cw_map[j]);
    return _channel_prob(d, length, error_prob);
}

/* Nearest Neighbour encode vector, return channel index */
uint test_encode(double *vector, vectorset *codebook, uint *cw_map,
                double error_prob) {
    uint channel_index = 0;
    double d = DBL_MAX;
    for (uint i = 0; i < codebook->size; i++) {
        double d_new = 0;
        for (uint j = 0; j < codebook->size; j++) {
            d_new += channel_prob(j, i, error_prob, log2(codebook->size),
                cw_map) * dist(vector, codebook->v[j], codebook->dim);
        }
        if (d_new < d) {
            d = d_new;
            channel_index = cw_map[i];
        }
    }
    assert(channel_index < codebook->size);
    return channel_index;
}

/* Decode to the vector indexed by cw_map[received_index] */
uint test_decode(uint received_index, vectorset *codebook, uint *cw_map) {
    uint i;
    // find this channel index in cw_map, to find source index...
    for (i = 0; i < codebook->size; i++) {
        if (cw_map[i] == received_index) {
            // this is it
            break;
        }
    }
    return i;
}

double run_test(vectorset *test_set, double error_prob, vectorset *codebook,
                uint *cw_map, vectorset *test_out, double *snr, double *psnr,
                double *sqnr) {
    size_t i, k;
    uint channel_index;
    double distortion = 0;
    double e_x_sq = 0;
    double e_x = 0;
    int error_count = 0;
    int reconstruction_index = 0;
    double max = 0;
    double min = 0;
    double q_distortion = 0;

    for (i = 0; i < test_set->size; i++) {
        double this_energy = 0;
        for (uint dim = 0; dim < test_set->dim; dim++) {
            this_energy += test_set->v[i][dim]*test_set->v[i][dim];
        }
        double this_norm = sqrt(this_energy);

        for (uint dim = 0; dim < test_set->dim; dim++) {
            e_x_sq += this_energy;
            e_x += this_norm;
        }
        
        if (this_energy < min || i == 0) {
            min = this_energy;
        }
        else if (this_energy > max) {
            max = this_energy;
        }
        // encode source index to nearest neighbour
        channel_index = test_encode(test_set->v[i], codebook, cw_map,
            error_prob);

        // quantization noise (before channel)
        q_distortion += dist(test_set->v[i], 
            codebook->v[test_decode(channel_index, codebook, cw_map)],
            test_set->dim);

        // pass channel_index through BSC
        binary_symmetric_channel(&channel_index, error_prob,
            log2(codebook->size));

        // decode received index to codevector
        reconstruction_index = test_decode(channel_index, codebook, cw_map);

        for (k = 0; k < test_set->dim; k++) {
            test_out->v[i][k] = codebook->v[reconstruction_index][k];
        }

        if (channel_index != cw_map[reconstruction_index]) {
            error_count++;
        }

        // sum the end-to-end distortion
        distortion += dist(test_set->v[i], codebook->v[reconstruction_index],
            test_set->dim);
    }
    fprintf(stderr, "Finished running test set\n");

    // printf("error_count = %d\n", error_count);
    distortion /= (test_set->size * test_set->dim);
    q_distortion /= (test_set->size * test_set->dim);

    e_x_sq /= (test_set->size * test_set->dim);
    e_x /= (test_set->size * test_set->dim);

    *snr = 10 * log10((e_x_sq - (e_x * e_x)) / distortion);
    *psnr = 10 * log10(((max - min)*(max - min)) / distortion);
    *sqnr = 10 * log10((e_x_sq - (e_x * e_x)) / q_distortion);

    /* Write to output file */
    // fprintf(stderr, "Writing test channel output data to %s\n", test_out_csv);
    // fp = fopen(test_out_csv, "w");
    // print_vectorset(fp, test_out);
    // fclose(fp);

    return distortion;
}
