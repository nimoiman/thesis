#include "covq.h"

/* partition_index, count ought to be size of training set train
   
   for x in *train, ensures j := partition_index(x) minimizes:
   sum_k(Pr(k received|j sent) * MSE(x, codebook[j]))

   note: Pr(k received|j sent) = error_prob * hamming_distance(k, j),
   MSE(x, codebook[j]) = dist(x, codebook[j])
*/
double nearest_neighbour(vectorset *train, vectorset *codebook,
                         uint *partition_index, size_t *count, uint *cw_map,
                         double error_prob) {
    size_t i, j, k;
    double new_distance, best_distance, total_distance;
    // initialize partition indices to "arbitrary" codebook vector
    for (i = 0; i < train->size; i++) {
        partition_index[i] = 0;
    }

    for (i = 0; i < codebook->size; i++) {
        count[i] = (i == 0) ? train->size : 0;
    }
    total_distance = 0;
    for (i = 0; i < train->size; i++) {
        // index nearest neighbour for each training vector
        best_distance = DBL_MAX;
        for (j = 0; j < codebook->size; j++) {
            // linear search for nearest neighbour for now
            new_distance = 0;
            for (k = 0; k < codebook->size; k++) {
                new_distance += channel_prob(k, j, error_prob,
                    (uint) log2(codebook->size), cw_map) * dist(train->v[i],
                    codebook->v[j], train->dim);
            }
            if (new_distance < best_distance) {
                count[partition_index[i]] -= 1;
                partition_index[i] = j;
                count[j] += 1;
                best_distance = new_distance;
            }
        }

        total_distance += best_distance;
    }
    return total_distance / (train->size * train->dim);
}


/* count ought to be size of codebook: the count of vectors mapped to that
   codevector 
   partition_index is the codevector lookup for training set

   satisfies:
   codebook->v[i] = sum_j(Pr(i received|j sent) * partition_euclidean_centroid)
                    / sum_j(Pr(i received|j sent) * partition_probability)
   
   note: Pr(i received|j sent) = error_prob * hamming_distance(i, j)
 */
void update_centroids(vectorset *train, vectorset *codebook,
                      uint *partition_index, size_t *count, uint *cw_map,
                      double error_prob) {
    size_t i, j, k;
    uint dim;
    double *partition_euclidean_centroid, partition_probability;
    double *numerator, denominator;

    // allocate numerator, partition_euclidean_centroid
    numerator = calloc(train->dim, sizeof(double));
    partition_euclidean_centroid = calloc(train->dim, sizeof(double));

    for (i = 0; i < codebook->size; i++) {
        // zero numerator
        for (dim = 0; dim < train->dim; dim++) {
            numerator[dim] = 0;
        }
        denominator = 0;
        for (j = 0; j < codebook->size; j++) {
            for (dim = 0; dim < codebook->dim; dim++) {
                partition_euclidean_centroid[dim] = 0;
            }
            for (k = 0; k < train->size; k++) {
                if (partition_index[k] == j) {
                    for (dim = 0; dim < codebook->dim; dim++) {
                        partition_euclidean_centroid[dim] += train->v[k][dim];
                    }
                }
            }
            for (dim = 0; dim < train->dim; dim++) {
                partition_euclidean_centroid[dim] /= train->size;
            }

            partition_probability = ((double) count[j]) / train->size;
            assert(partition_probability >= 0);
            assert(partition_probability <= 1);

            double p_ij = channel_prob(i, j, error_prob, (uint) log2(codebook->size),
                cw_map);

            for (dim = 0; dim < train->dim; dim++) {
                numerator[dim] += p_ij * partition_euclidean_centroid[dim];
            }

            denominator += p_ij * partition_probability;
        }

        if (denominator > 0) {
            for (dim = 0; dim < codebook->dim; dim++) {
                codebook->v[i][dim] = numerator[dim] / denominator;
            }
        }
        else {
            for (dim = 0; dim < codebook->dim; dim++) {
                codebook->v[i][dim] = 0;
            }
        }
    }
    free(numerator);
    free(partition_euclidean_centroid);
}


/* note: n_splits should be less than log_2(INT_MAX)=16 */
double bsc_covq(vectorset *train, vectorset *codebook, uint *cw_map,
                uint n_splits, double error_prob, double lbg_eps,
                double code_vector_displace) {
    double d_new = 0, d_old = 0;
    uint i, j, k, *partition_index;
    size_t *count;

    if (!(partition_index = malloc(sizeof(uint) * train->size))) {
        return -1;
    }
    else if (!(count = malloc(sizeof(size_t) * (1 << n_splits)))) {
        free(partition_index);
        return -1;
    }

    /* Initialize codebook as single zero vector */
    codebook->size = 1;
    for (i = 0; i < codebook->dim; i++) {
        codebook->v[0][i] = 0;
    }

    // iterate through n_splits
    for (i = 0; i <= n_splits; i++) {
        d_new = nearest_neighbour(train, codebook, partition_index, count,
                cw_map, error_prob);
        // Ensure centroid condition met for annealing
        update_centroids(train, codebook, partition_index, count, cw_map,
                error_prob);
        // Perform simulated annealing
        anneal(codebook, count, cw_map, train->size, error_prob);

        do {
            d_old = d_new;
            // satisfy nearest neighbour criterion on decoder's end
            d_new = nearest_neighbour(train, codebook, partition_index, count,
                cw_map, error_prob);
            // satisfy centroid criterion on encoder's end
            update_centroids(train, codebook, partition_index, count, cw_map,
                error_prob);

        } while (d_old > (1 + lbg_eps) * d_new);

        if (i < n_splits) {
            /* Split each codevector, add splitted vector to codebook */
            for (j = 0; j < codebook->size; j++) {
                // iterate through vector components
                for (k = 0; k < codebook->dim; k++) {
                    // note that, since codebook->size is a power of 2,
                    // and codebook->size > j,
                    // hamming_distance(j, j + codebook) == 1 (i.e. minimal)
                    codebook->v[j + codebook->size][k] = codebook->v[j][k]
                        + code_vector_displace;
                }
            }
            if (i == 0) {
                fprintf(stderr, "Split ");
            }
            else {
                fprintf(stderr, ", ");
            }
            fprintf(stderr, "%u", i);
            if (i == n_splits - 1) {
                fprintf(stderr, "\n");
            }
            // set codebook size
            codebook->size = 1 << (i+1); // 2^(i+1)
        }
    }
    free(partition_index);
    free(count);
    return d_new;
}
