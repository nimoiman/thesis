#include "covq.h"

void print_usage(FILE *fp) {
    fprintf(fp, "Usage: ./covq [OPTION]...\n");
    fprintf(fp, "\nOptions:\n");
    fprintf(fp, "  --train train.csv codebook_out.csv cw_map_out.csv\n");
    fprintf(fp, "  --test test_in.csv codebook_in.csv cw_map.csv test_out.csv\n");
    fprintf(fp, "  -d, --dim NUM [default=2]\n");
    fprintf(fp, "  -n, --nsplits NUM [default=4]\n");
    fprintf(fp, "  -p, --bep NUM [default=0.001]\n");
    fprintf(fp, "  -s, --seed NUM [default=1234]\n");
    fprintf(fp, "  -h, --help\n");
}

struct cmdline {
    int train;
    char *train_csv;
    char *codebook_out;
    char *cw_map_out;

    int test;
    char *test_in_csv;
    char *codebook_in;
    char *cw_map_in;
    char *test_out_csv;

    int seed;
    uint dim;
    uint nsplits;
    double bep;
};

void init_cmdline(struct cmdline *opts) {
    /* Set default values for parameters */
    opts->train = 0;
    opts->test = 0;
    opts->dim = 2;
    opts->nsplits = 4;
    opts->bep = 0.001;
    opts->seed = 1234;
}

struct cmdline parse_args(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(stderr);
        exit(1);
    }
    else {
        for (int i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
                print_usage(stderr);
                exit(0);
            }
        }
    }

    struct cmdline opts;
    init_cmdline(&opts);

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--train")) {
            opts.train = 1;
            if (i + 3 < argc) {
                i += 1;
                opts.train_csv = argv[i];
                i += 1;
                opts.codebook_out = argv[i];
                i += 1;
                opts.cw_map_out = argv[i];
            }
            else {
                fprintf(stderr, "Provide a training set csv, output codebook filename, and cw_map.csv\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "--test")) {
            opts.test = 1;
            if (i + 4 < argc) {
                i += 1;
                opts.test_in_csv = argv[i];
                i += 1;
                opts.codebook_in = argv[i];
                i += 1;
                opts.cw_map_in = argv[i];
                i += 1;
                opts.test_out_csv = argv[i];
            }
            else {
                fprintf(stderr, "Provide a testing set csv, input codebook file, cw_map.csv, test_out.csv\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dim")) {
            if (i + 1 < argc) {
                i += 1;
                opts.dim = atoi(argv[i]);
            }
            else {
                fprintf(stderr, "No dim provided.\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--nsplits")) {
            if (i + 1 < argc) {
                i += 1;
                opts.nsplits = atoi(argv[i]);
            }
            else {
                fprintf(stderr, "No dim provided.\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--bep")) {
            if (i + 1 < argc) {
                i += 1;
                opts.bep = atof(argv[i]);
            }
            else {
                fprintf(stderr, "No BEP provided.\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--seed")) {
            if (i + 1 < argc) {
                i += 1;
                opts.seed = atoi(argv[i]);
            }
            else {
                fprintf(stderr, "No random seed provided.\n");
                print_usage(stderr);
                exit(1);
            }
        }
        else {
            fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
            print_usage(stderr);
            exit(1);
        }
    }
    return opts;
}

int main(int argc, char *argv[]) {
    struct cmdline opts = parse_args(argc, argv);

    /* Set random seed */
    srand(opts.seed);
    

    if (opts.train) {
        // Codeword map for simulated annealing
        uint *cw_map = malloc(sizeof(uint) * (((uint) 1) << opts.nsplits));
        // initialize cw_map to the identity mapping
        for (uint i = 0; i < (((uint) 1) << opts.nsplits); i++) {
            cw_map[i] = i;
        }

        /* Open tr_set.csv for reading */
        FILE *fp = fopen(opts.train_csv, "r");
        size_t tr_size = get_num_lines(fp);

        vectorset *train;
        // allocate training vector set
        if (!(train = init_vectorset(tr_size, opts.dim))) {
            fprintf(stderr, "Could not allocate training vector array.\n");
            exit(1);
        }
        printf("allocated train\n");
        printf("ready to receive %zu*%u doubles\n", train->size, train->dim);

        rewind(fp);
        // read in training set
        for (size_t i = 0; i < train->size; i++) {
            get_next_csv_record(fp, train->v[i], train->dim);
        }
        fclose(fp);

        /* Generate codebook from training set */
        vectorset *codebook = bsc_covq(train, cw_map, opts.nsplits, opts.bep);
        destroy_vectorset(train);

        /* Write codebook to file */
        fp = fopen(opts.codebook_out, "w");
        print_vectorset(fp, codebook);
        fclose(fp);

        /* Write cw_map to file */
        fp = fopen(opts.cw_map_out, "w");
        for (uint i = 0; i < (((uint) 1) << opts.nsplits); i++) {
            fprintf(fp, "%d%s", cw_map[i], (i < (((uint) 1) << opts.nsplits) - 1)
                ? IO_DELIM: "\n");
        }
        fclose(fp);
        free(cw_map);
    }
    if (opts.test) {
        /* Check dimensions of (first lines of) input files */
        FILE *fp = fopen(opts.codebook_in, "r");
        uint cbook_dim = get_num_cols(fp);
        printf("cbook_dim=%u\n", cbook_dim);
        rewind(fp);
        size_t cbook_size = get_num_lines(fp);
        printf("cbook_size=%zu\n", cbook_size);
        fclose(fp);

        fp = fopen(opts.cw_map_in, "r");
        uint cw_map_dim = get_num_cols(fp);
        printf("cw_map_dim=%u\n", cw_map_dim);
        fclose(fp);

        fp = fopen(opts.test_in_csv, "r");
        uint test_set_dim = get_num_cols(fp);
        printf("test_set_dim=%u\n", test_set_dim);
        fclose(fp);

        if (!(cbook_dim == test_set_dim && cw_map_dim == (uint) cbook_size)) {
            fprintf(stderr, "The dimensions of codebook and cw_map do not match!\n");
            exit(1);
        }

        uint *cw_map = malloc(sizeof(int) * cw_map_dim);
        /* Read in cw_map from file */
        printf("Reading cw_map: %s\n", opts.cw_map_in);
        fp = fopen(opts.cw_map_in, "r");
        read_cw_map_csv(fp, cw_map, cw_map_dim);
        printf("just read cw_map\n");
        fclose(fp);

        /* Read in testing set from file */
        printf("Reading test_in_csv: %s\n", opts.test_in_csv);
        fp = fopen(opts.test_in_csv, "r");
        size_t test_size = get_num_lines(fp);

        vectorset *test;
        if (!(test = init_vectorset(test_size, test_set_dim))) {
            fprintf(stderr, "Could not allocate testing vector array.\n");
            exit(1);
        }
        printf("Allocated test set\n");
        printf("Ready to receive %zu*%u doubles\n", test->size, test->dim);
        rewind(fp);
        for (size_t i = 0; i < test->size; i++) {
            get_next_csv_record(fp, test->v[i], test->dim);
        }
        fclose(fp);

        /* Read in codebook from file */
        printf("Reading codebook: %s\n", opts.codebook_in);
        vectorset *codebook;
        if (!(codebook = init_vectorset(cbook_size, cbook_dim))) {
            fprintf(stderr, "Could not allocate codebook vector array.\n");
            exit(1);
        }

        fp = fopen(opts.codebook_in, "r");
        for (size_t i = 0; i < codebook->size; i++) {
            get_next_csv_record(fp, codebook->v[i], codebook->dim);
        }
        fclose(fp);

        /* Run on test data and write received output to file */
        printf("Running test data, writing into %s\n", opts.test_out_csv);
        double distortion = run_test(codebook, cw_map, test, opts.test_out_csv,
            opts.bep);
        printf("Test distortion = %f\n", distortion);

        free(cw_map);
        destroy_vectorset(test);
        destroy_vectorset(codebook);
    }
    return 0;
}
