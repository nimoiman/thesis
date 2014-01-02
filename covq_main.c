#include <stdio.h>
#include "covq.h"

int main(int argc, char *argv[]) {
    int i, j;
    int numtrain = 500;
    int nsplits = 7;
    vectorset *train, *c;
    FILE *fp;

    if(argc != 3){
        fprintf(stderr, "Need to specify 2 inputs: training set and codebook file\n");
        exit(1);
    }
    
    // allocate training vector set
    if(!(train = init_vectorset(numtrain))) {
        fprintf(stderr, "Could not generate training data.\n");
        exit(1);
    }

    /* Reset random seed */
    srand(1234);

    /* Initialize training set to Gaussian(0,10) distribution */
    for (i = 0; i < numtrain; i++) {
        gaussian_channel(train->v[i], VECTOR_DIM, 0, 10);
    }
    
    fp = fopen(argv[1], "w");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open training set file %s\n", argv[1]);
        exit(1);
    }
    print_vectorset(fp, train);
    fclose(fp);

    /* Generate codebook from training set */
    c = bsc_covq(train, nsplits);

    fp = fopen(argv[2], "w");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open codebook file %s\n", argv[2]);
        exit(1);
    }
    print_vectorset(fp, c);
    fclose(fp);

    destroy_vectorset(c);
    destroy_vectorset(train);
}
