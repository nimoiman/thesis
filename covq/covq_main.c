#include "covq.h"

int main(int argc, char *argv[]) {
    int i, j;
    int tr_size, test_size;
    int nsplits = 7;
    vectorset *train, *c;
    double *d;
    FILE *fp;

    // Expects input of training set csv name and testing set csv name

    if(argc != 5) {
        fprintf(stderr, "Usage: covq tr_set.csv test_set.csv codebook_out.csv distortion_out.csv\n");
        exit(1);
    }

    /* Open tr_set.csv for reading */
    fp = fopen(argv[1], "r");

    tr_size = get_num_lines(fp);

    // allocate training vector set
    if (!(train = init_vectorset(tr_size))) {
        fprintf(stderr, "Could not allocate training vector array.\n");
        exit(1);
    }

    rewind(fp);
    // read in training set
    for (i = 0; i < tr_size; i++) {
        for (j = 0; j < VECTOR_DIM; j++) {
            get_next_csv_record(fp, train->v[i]);
        }
    }
    fclose(fp);

    /* Reset random seed */
    srand(1234);

    /* Generate codebook from training set */
    c = bsc_covq(train, nsplits);
    destroy_vectorset(train);

    /* Output codebook to file */
    fp = fopen(argv[2], "w");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open codebook file %s\n", argv[2]);
        exit(1);
    }
    print_vectorset(fp, c);
    fclose(fp);


    /* Open test_set.csv for reading */
    fp = fopen(argv[2], "r");

    test_size = get_num_lines(fp);

    // allocate testing vector set
    if (!(test = init_vectorset(test_size))) {
        fprintf(stderr, "Could not allocate test vector array.\n")
        exit(1);
    }

    rewind(fp);
    /* Read in test data */
    for (i = 0; i < test_size; i++) {
        for (j = 0; j < VECTOR_DIM; j++) {
            get_next_csv_record(fp, test->v[i]);
        }
    }
    fclose(fp);

    /* Run on test data */


    /* Output distortions to distortion_out.csv */

    destroy_vectorset(c);
    
}
