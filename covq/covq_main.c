#include "covq.h"

int main(int argc, char *argv[]) {
    int i;
    int tr_size, test_size;
    int vector_dim = 2;
    int nsplits = 4;
    double error_prob = 0.001;
    vectorset *train, *test, *c;
    double distortion;
    FILE *fp;
    // Codeword map for simulated annealing
    int *cw_map = malloc(sizeof(int) * (1 << nsplits));

    // initialize cw_map to the identity mapping
    for (i = 0; i < (1 << nsplits); i++) {
        cw_map[i] = i;
    }

    // Expects input of training set csv name and testing set csv name

    if (argc != 4) {
        fprintf(stderr, "Usage: covq tr_set.csv test_set.csv codebook_out.csv\n");
        exit(1);
    }

    /* Open tr_set.csv for reading */
    fp = fopen(argv[1], "r");

    tr_size = get_num_lines(fp);

    // allocate training vector set
    if (!(train = init_vectorset(tr_size, vector_dim))) {
        fprintf(stderr, "Could not allocate training vector array.\n");
        exit(1);
    }

    rewind(fp);
    // read in training set
    for (i = 0; i < train->size; i++) {
        get_next_csv_record(fp, train->v[i], train->dim);
    }
    fclose(fp);

    /* Reset random seed */
    srand(1234);

    /* Generate codebook from training set */
    c = bsc_covq(train, cw_map, nsplits, error_prob);
    destroy_vectorset(train);

    /* Write codebook to file */
    fp = fopen(argv[3], "w");
    print_vectorset(fp, c);
    fclose(fp);

    /* Open test_set.csv for reading */
    fp = fopen(argv[2], "r");

    test_size = get_num_lines(fp);

    // allocate testing vector set
    if (!(test = init_vectorset(test_size, vector_dim))) {
        fprintf(stderr, "Could not allocate test vector array.\n");
        exit(1);
    }

    rewind(fp);
    /* Read in test data */
    for (i = 0; i < test->size; i++) {
        get_next_csv_record(fp, test->v[i], test->dim);
    }
    fclose(fp);

    /* Run on test data */
    distortion = run_test(c, cw_map, test, error_prob);
    printf("test distortion = %f\n", distortion);

    // store test distortion in file
    fp = fopen("distortion_out.csv", "a");
    fprintf(fp, "%f,%f\n", error_prob, distortion);
    fclose(fp);

    free(cw_map);
    destroy_vectorset(test);
    destroy_vectorset(c);
    return 0;
}
