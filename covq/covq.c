#include "covq.h"

/* partition_index, count ought to be size of training set train
   
   for x in *train, ensures j := partition_index(x) minimizes:
   sum_k(Pr(k received|j sent) * MSE(x, codebook[j]))

   note: Pr(k received|j sent) = BSC_ERROR_PROB * hamming_distance(k, j),
   MSE(x, codebook[j]) = dist(x, codebook[j])
*/
double nearest_neighbour(vectorset *train, vectorset *codebook,
                         int *partition_index, int *count, int *cw_map) {
    int i, j, k;
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
                new_distance += channel_prob(k, j, BSC_ERROR_PROB,
                    log2(codebook->size), cw_map) * dist(train->v[i], codebook->v[j]);
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
    return total_distance / train->size;
}


/* count ought to be size of codebook: the count of vectors mapped to that
   codevector 
   partition_index is the codevector lookup for training set

   satisfies:
   codebook->v[i] = sum_j(Pr(i received|j sent) * partition_euclidean_centroid)
                    / sum_j(Pr(i received|j sent) * partition_probability)
   
   note: Pr(i received|j sent) = BSC_ERROR_PROB * hamming_distance(i, j)
 */
void update_centroids(vectorset *train, vectorset *codebook,
                      int *partition_index, int *count, int *cw_map) {
    int i, j, k, dim;
    double partition_euclidean_centroid[VECTOR_DIM], partition_probability;
    double numerator[VECTOR_DIM], denominator;

    for (i = 0; i < codebook->size; i++) {
        for (dim = 0; dim < VECTOR_DIM; dim++) {
            numerator[dim] = 0;
        }
        denominator = 0;
        for (j = 0; j < codebook->size; j++) {
            for (dim = 0; dim < VECTOR_DIM; dim++) {
                partition_euclidean_centroid[dim] = 0;
            }
            for (k = 0; k < train->size; k++) {
                if (partition_index[k] == j) {
                    for (dim = 0; dim < VECTOR_DIM; dim++) {
                        partition_euclidean_centroid[dim] += train->v[k][dim];
                    }
                }
            }
            for (dim = 0; dim < VECTOR_DIM; dim++) {
                partition_euclidean_centroid[dim] /= train->size;
            }

            partition_probability = (double) count[j] / train->size;

            for (dim = 0; dim < VECTOR_DIM; dim++) {
                numerator[dim] += channel_prob(i, j, BSC_ERROR_PROB,
                    log2(codebook->size), cw_map) * partition_euclidean_centroid[dim];
            }

            denominator += channel_prob(i, j, BSC_ERROR_PROB,
                log2(codebook->size), cw_map) * partition_probability;
        }
        for (dim = 0; dim < VECTOR_DIM; dim++) {
            codebook->v[i][dim] = numerator[dim] / denominator;
        }
    }
}



/* note: n_splits should be less than log_2(INT_MAX)=16 */
vectorset *bsc_covq(vectorset *train, int *cw_map, int n_splits) {
    double d_new, d_old;
    int i, j, k, *partition_index, *count;
    vectorset *codebook;

    if (!(partition_index = malloc(sizeof(int) * train->size))) {
        return NULL;
    }
    else if (!(count = malloc(sizeof(int) * (1 << n_splits)))) {
        free(partition_index);
        return NULL;
    }
    else if (!(codebook = init_vectorset(1 << n_splits))) {
        free(partition_index);
        free(count);
        return NULL;
    }

    /* Initialize codebook as single zero vector */
    codebook->size = 1;
    for (i = 0; i < VECTOR_DIM; i++) {
        codebook->v[0][i] = 0;
    }

    d_new = DBL_MAX; // iteration minimizes d_new

    // iterate through n_splits
    for (i = 0; i < n_splits; i++) {
        do {
            d_old = d_new;
            // satisfy nearest neighbour criterion on decoder's end
            d_new = nearest_neighbour(train, codebook, partition_index, count,
                cw_map);
            // satisfy centroid criterion on encoder's end
            update_centroids(train, codebook, partition_index, count, cw_map);

        } while (d_old > (1 + LBG_EPS) * d_new);

        /* Split each codevector, add splitted vector to codebook */

        for (j = 0; j < codebook->size; j++) {
            // iterate through vector components
            for (k = 0; k < VECTOR_DIM; k++) {
                // note that, since codebook->size is a power of 2,
                // and codebook->size > j,
                // hamming_distance(j, j + codebook) == 1 (i.e. minimal)
                codebook->v[j + codebook->size][k] = codebook->v[j][k]
                    + CODE_VECTOR_DISPLACE;
            }
        }
        // set codebook size
        codebook->size = 1 << (i+1); // 2^(i+1)
    }
    free(partition_index);
    free(count);
    return codebook;
}
