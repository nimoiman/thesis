#ifndef ANNEAL_H
#define ANNEAL_H

#ifdef __cplusplus
extern "C" {
#endif

// Simulated Annealing Constants
#define TEMP_INIT 10.0
#define COOLING_RATE 0.8
#define TEMP_FINAL 0.00025
#define PHI 5 // energy drops until temperature drop
#define PSI 200 // rejected swaps until temperature drop

#include <assert.h> /* assert */
#include "vector.h" /* vectorset */
#include "channel.h" /* BSC_ERROR_PROB */

void anneal(vectorset *codebook, size_t *count, uint *cw_map, size_t trset_size,
            double error_prob);

#ifdef __cplusplus
}
#endif

#endif // ANNEAL_H
