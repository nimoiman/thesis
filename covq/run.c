#include "covq.h"

void run(struct cmdline *opts, struct retval *rv) {

    srand(opts->seed);

    /* Either training or testing, we need cw_map & codebook */
    /* Codeword map for simulated annealing */
    if (!(rv->cw_map = (uint*) malloc(sizeof(uint) * (((uint) 1) << opts->nsplits)))) {
        fprintf(stderr, "Could not allocate cw_map array.\n");
        free_retval(rv, opts);
        exit(1);
    }
    ;
    /* Allocate codebook */
    if (!(rv->codebook = init_vectorset(((size_t) 1) << opts->nsplits,
                                        opts->dim))) {
        fprintf(stderr, "Could not allocate training vector array.\n");
        free_retval(rv, opts);
        exit(1);
    }

    if (opts->train) {
        
        /* Initialize cw_map to the identity mapping */
        for (uint i = 0; i < (((uint) 1) << opts->nsplits); i++) {
            rv->cw_map[i] = i;
        }

        /* Open training set csv to find size of set */
        FILE *fp = fopen(opts->train_csv, "r");
        size_t tr_size = get_num_lines(fp);

        /* Allocate training vector set */
        vectorset *train;
        if (!(train = init_vectorset(tr_size, opts->dim))) {
            fprintf(stderr, "Could not allocate training vector array.\n");
            exit(1);
        }

        /* Read in training set */
        rewind(fp);
        for (size_t i = 0; i < train->size; i++) {
            get_next_csv_record(fp, train->v[i], train->dim);
        }
        fclose(fp);

        /* Generate codebook, cw_map from training set */
        rv->theor_distortion = bsc_covq(train, rv->codebook, rv->cw_map, opts->nsplits,
                                        opts->bep, opts->lbg_eps,
                                        opts->codevector_displace);
        destroy_vectorset(train);

        /* Write codebook to file */
        fp = fopen(opts->codebook_out, "w");
        print_vectorset(fp, rv->codebook);
        fclose(fp);

        /* Write cw_map to file */
        fp = fopen(opts->cw_map_out, "w");
        for (uint i = 0; i < (((uint) 1) << opts->nsplits); i++) {
            fprintf(fp, "%d%s", rv->cw_map[i],
                    (i < (((uint) 1) << opts->nsplits) - 1) ? IO_DELIM: "\n");
        }
        fclose(fp);

    }
    if (opts->test) {
        /* Check dimension & size of codebook */
        FILE *fp = fopen(opts->codebook_in, "r");
        uint cbook_dim = get_num_cols(fp);
        rewind(fp);
        size_t cbook_size = get_num_lines(fp);
        fclose(fp);

        /* Check dimension of cw_map */
        fp = fopen(opts->cw_map_in, "r");
        uint cw_map_dim = get_num_cols(fp);
        fclose(fp);
        printf("%s\n", opts->test_in_csv);

        /* Check dimension of test set */
        fp = fopen(opts->test_in_csv, "r");
        uint test_set_dim = get_num_cols(fp);
        fclose(fp);

        if (!(cbook_dim == test_set_dim && cw_map_dim == (uint) cbook_size)) {
            fprintf(stderr,
                    "The dimensions of codebook and cw_map do not match!\n");
            exit(1);
        }

        /* Read in cw_map from file */
        rv->cw_map = malloc(sizeof(int) * cw_map_dim);
        fp = fopen(opts->cw_map_in, "r");
        read_cw_map_csv(fp, rv->cw_map, cw_map_dim);
        fclose(fp);

        /* Read in testing set from file */
        fp = fopen(opts->test_in_csv, "r");
        size_t test_size = get_num_lines(fp);

        if (!(rv->test_in = init_vectorset(test_size, test_set_dim))) {
            fprintf(stderr, "Could not allocate testing vector array.\n");
            exit(1);
        }

        rewind(fp);
        for (size_t i = 0; i < rv->test_in->size; i++) {
            get_next_csv_record(fp, rv->test_in->v[i], rv->test_in->dim);
        }
        fclose(fp);

        /* Read in codebook from file */
        if (!(rv->codebook = init_vectorset(cbook_size, cbook_dim))) {
            fprintf(stderr,
                    "Could not allocate codebook vector array.\n");
            exit(1);
        }

        fp = fopen(opts->codebook_in, "r");
        for (size_t i = 0; i < rv->codebook->size; i++) {
            get_next_csv_record(fp, rv->codebook->v[i], rv->codebook->dim);
        }
        fclose(fp);

        /* Run on test data */
        rv->test_out = init_vectorset(rv->test_in->size, rv->test_in->dim);
        rv->empir_distortion = run_test(rv->test_in, opts->bep, rv->codebook,
            rv->cw_map, rv->test_out, &(rv->snr), &(rv->psnr), &(rv->sqnr));

        /* Write testing output to file */
        fp = fopen(opts->test_out_csv, "w");
        print_vectorset(fp, rv->test_out);
        fclose(fp);

        // fprintf(stderr, "Writing test channel output data to %s\n",
        //         opts->test_out_csv);
        // fp = fopen(opts->test_out_csv, "w");
        // print_vectorset(fp, test_out);
        // fclose(fp);

    }
}