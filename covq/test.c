#include "anneal.h"
#include "vector.h"
#include "channel.h"

int main() {
    double error_prob = 0.001;
    size_t trset_size = 10000;
    size_t counted = 0;
    uint rate = 8;
    uint dim = 5;

    vectorset *codebook;
    if (!(codebook = init_vectorset((size_t) 1 << rate, dim))) {
        fprintf(stderr, "Could not allocate codebook vector array.\n");
        exit(1);
    }
    uint *cw_map = malloc(sizeof(uint) * codebook->size);
    uint *count = malloc(sizeof(uint) * trset_size);

    /* Initialize codebook set to Gaussian */
    for (size_t i = 0; i < codebook->size; i++) {
        for (uint d = 0; d < codebook->dim; d++) {
            codebook->v[i][d] = box_muller(0, 20);
        }
        cw_map[i] = i;
        count[i] = rand_lim(trset_size - counted);
        counted += count[i];
    }
    count[codebook->size - 1] += trset_size - counted;

    test_anneal(codebook, count, cw_map, trset_size, error_prob);
    destroy_vectorset(codebook);
    free(cw_map);
    free(count);
    return 0;
}