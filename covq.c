#include "covq.h"

void print_int(int *array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        fprintf(stderr, "%d, ", array[i]);
    }
    fprintf(stderr, "\n");
}

void print_double(double *array, int size) {
    int i;
    for (i = 0; i < size; i++) {
        fprintf(stderr, "%f, ", array[i]);
    }
    fprintf(stderr, "\n");
}

/* partition_index, count ought to be size of training set train
   
   for x in *train, ensures j := partition_index(x) minimizes:
   sum_k(Pr(k received|j sent) * MSE(x, codebook[j]))

   note: Pr(k received|j sent) = BSC_ERROR_PROB * hamming_distance(k, j),
   MSE(x, codebook[j]) = dist(x, codebook[j])
*/
double nearest_neighbour(vectorset *train, vectorset *codebook, int *partition_index, int *count) {
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
        best_distance = 100;
        for (j = 0; j < codebook->size; j++) {
            // linear search for nearest neighbour for now
            new_distance = 0;

            for (k = 0; k < codebook->size; k++) {
                new_distance += BSC_ERROR_PROB * hamming_distance(k, j) * dist(train->v[i], codebook->v[j]);
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

double transition_probability(int i, int j, int length) {
    return pow(BSC_ERROR_PROB, hamming_distance(i, j)) * pow(1-BSC_ERROR_PROB, length - hamming_distance(i, j));
}

/* count ought to be size of codebook: the count of vectors mapped to that
   codevector 
   partition_index is the codevector lookup for training set

   satisfies:
   codebook->v[i] = sum_j(Pr(i received|j sent) * partition_euclidean_centroid)
                    / sum_j(Pr(i received|j sent) * partition_probability)
   
   note: Pr(i received|j sent) = BSC_ERROR_PROB * hamming_distance(i, j)
 */
void update_centroids(vectorset *train, vectorset *codebook, int *partition_index, int *count) {
    int i, j, k, dim;
    double partition_euclidean_centroid[VECTOR_DIM], partition_probability, numerator[VECTOR_DIM], denominator;

    // zero codebook for reconstruction
    for (i = 0; i < codebook->size; i++) {
        for (j = 0; j < VECTOR_DIM; j++) {
            codebook->v[i][j] = 0;
        }
    }

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

            // print_int(count, codebook->size);
            // fprintf(stderr, "j=%d\n", j);
            // fprintf(stderr, "count[j]=%d\n", count[j]);

            partition_probability = (double) count[j] / train->size;

            for (dim = 0; dim < VECTOR_DIM; dim++) {
                numerator[dim] += transition_probability(k, j, log2(codebook->size)) * partition_euclidean_centroid[dim];
            }
            // fprintf(stderr, "BSC_ERROR_PROB: %f\n", BSC_ERROR_PROB);
            // fprintf(stderr, "hamming dist: %d\n", hamming_distance(k, j));
            // fprintf(stderr, "partition_probability: %f\n", partition_probability);
            denominator += transition_probability(k, j, log2(codebook->size)) * partition_probability;
        }
        for (dim = 0; dim < VECTOR_DIM; dim++) {
            codebook->v[i][dim] = numerator[dim] / denominator;
        }
        // print_double(numerator, VECTOR_DIM);
        // fprintf(stderr, "denominator: %f\n", denominator);
    }
}



/* note: n_splits should be less than log_2(INT_MAX)=16 */
vectorset *bsc_covq(vectorset *train, int n_splits) {
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

    // for (i = 0; i < train->size; i++) {
    //     partition_index[i] = 0;
    // }

    // count[0] = train->size;

    d_new = DBL_MAX; // iteration minimizes d_new

    // iterate through n_splits
    for (i = 0; i < n_splits; i++) {
        do {
            d_old = d_new;
            // satisfy nearest neighbour criterion on decoder's end
            d_new = nearest_neighbour(train, codebook, partition_index, count);
            // fprintf(stderr, "After nn\n");
            // print_int(partition_index, train->size);
            // print_vectorset(stderr, codebook);
            // satisfy centroid criterion on encoder's end
            update_centroids(train, codebook, partition_index, count);
            // fprintf(stderr, "After centroids\n");
            // print_vectorset(stderr, codebook);
            // print_vectorset(stderr, codebook);


        } while (d_old > (1 + LBG_EPS) * d_new);

        /* Split each codevector, add splitted vector to codebook */

        for (j = 0; j < codebook->size; j++) {
            // iterate through vector components
            for (k = 0; k < VECTOR_DIM; k++) {
                codebook->v[j + codebook->size][k] = codebook->v[j][k] + CODE_VECTOR_DISPLACE;
            }
        }
        // set codebook size
        codebook->size = 1 << (i+1); // 2^(i+1)
        // fprintf(stderr, "just split\n");
        // print_vectorset(stderr, codebook);

        
    }
    free(partition_index);
    free(count);
    return codebook;
}
