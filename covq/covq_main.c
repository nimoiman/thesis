#include "covq.h"

void print_usage(FILE *fp) {
    fprintf(fp, "Usage: ./covq [OPTION]...\n");
    fprintf(fp, "\nOptions:\n");
    fprintf(fp, "  --train train.csv codebook_out.csv cw_map_out.csv\n");
    fprintf(fp, "  --test test_in.csv codebook_in.csv cw_map.csv test_out.csv\n");
    fprintf(fp, "  -e, --lbg-eps NUM [default=0.01]\n");
    fprintf(fp, "  -c, --cv-disp NUM [default=0.01]\n");
    fprintf(fp, "  -d, --dim NUM [default=2]\n");
    fprintf(fp, "  -n, --nsplits NUM [default=4]\n");
    fprintf(fp, "  -p, --bep NUM [default=0.001]\n");
    fprintf(fp, "  -s, --seed NUM [default=1234]\n");
    fprintf(fp, "  -h, --help\n");
}

void init_cmdline(struct cmdline *opts) {
    /* Set default values for parameters */
    opts->train = 0;
    opts->test = 0;
    opts->dim = 2;
    opts->nsplits = 4;
    opts->bep = 0.001;
    opts->seed = 1234;
    opts->lbg_eps = 0.01;
    opts->codevector_displace = 0.01;
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
                fprintf(stderr, "No dim provided with option %s.\n", argv[i]);
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
                fprintf(stderr, "No nsplits provided with option %s.\n", argv[i]);
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
                fprintf(stderr, "No BEP provided with option %s.\n", argv[i]);
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
                fprintf(stderr, "No random seed provided with option %s.\n", argv[i]);
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-e") || !strcmp(argv[i], "--lbg-eps")) {
            if (i + 1 < argc) {
                i += 1;
                opts.lbg_eps = atoi(argv[i]);
            }
            else {
                fprintf(stderr, "No lbg eps provided with option %s.\n", argv[i]);
                print_usage(stderr);
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--cv-disp")) {
            if (i + 1 < argc) {
                i += 1;
                opts.codevector_displace = atoi(argv[i]);
            }
            else {
                fprintf(stderr, "No codevector displacement provided with option %s.\n", argv[i]);
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


void free_retval(struct retval *rv, struct cmdline *opts) {
    if (opts->test) {
        destroy_vectorset(rv->test_out);
        destroy_vectorset(rv->test_in);
    }
    /* In every case, free codebook, cw_map */
    destroy_vectorset(rv->codebook);
    free(rv->cw_map);
    free(rv);
}

void single_2src(struct cmdline *opts_l, struct cmdline *opts_r,
                 struct retval *rv_l, struct retval *rv_r) {
    /* Run a single iteration of 2 source COSQ */
    run(opts_l, rv_l);
    run(opts_r, rv_r);

    /* Measure the joint PSNR, SNR */
    double distortion = 0;
    double power = 0;
    double mean = 0;
    double max_power = 0;
    double min_power = 0;
    for (size_t i = 0; i < rv_l->test_out->size; i++) {
        for (uint dim = 0; dim < opts_l->dim; dim++) {
            distortion += (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]) * (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]);
            distortion += (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]) * (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]);
            power += rv_l->test_in->v[i][dim] * rv_l->test_in->v[i][dim];
            power += rv_r->test_in->v[i][dim] * rv_r->test_in->v[i][dim];
            mean += rv_l->test_in->v[i][dim];
            mean += rv_r->test_in->v[i][dim];
        }
        if (power < min_power || i == 0) {
            min_power = power;
        }
        if (power > max_power || i == 0) {
            max_power = power;
        }
        
    }
    distortion /= (opts_l->dim * rv_l->test_out->size);
    power /= (opts_l->dim * rv_l->test_out->size);
    mean /= (opts_l->dim * rv_l->test_out->size);

    double snr = 10*log10((power - (mean*mean)) / distortion);
    double psnr = 10*log10((max_power - min_power) / distortion);
    printf("distortion=%f\n", distortion);
    FILE *fp = fopen("snr.out", "a");
    fprintf(fp, "%f\n", snr);
    fclose(fp);
    fp = fopen("psnr.out", "a");
    fprintf(fp, "%f\n", psnr);
    fclose(fp);
}

void iter_epsilon(struct cmdline *opts_l, struct cmdline *opts_r,
                  struct retval *rv_l, struct retval *rv_r) {
    /* Iterate through channel epsilons for 2 sources */
    for (double epsilon = 0.001; epsilon < 0.1; epsilon *= 1.2) {
        opts_l->bep = epsilon;
        opts_r->bep = epsilon;
        printf("epsilon=%f\n", epsilon);

        run(opts_l, rv_l);
        run(opts_r, rv_r);

        /* Measure the joint PSNR, SNR */
        double distortion = 0;
        double power = 0;
        double mean = 0;
        double max_power = 0;
        double min_power = 0;
        for (size_t i = 0; i < rv_l->test_out->size; i++) {
            for (uint dim = 0; dim < opts_l->dim; dim++) {
                distortion += (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]) * (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]);
                distortion += (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]) * (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]);
                power += rv_l->test_in->v[i][dim] * rv_l->test_in->v[i][dim];
                power += rv_r->test_in->v[i][dim] * rv_r->test_in->v[i][dim];
                mean += rv_l->test_in->v[i][dim];
                mean += rv_r->test_in->v[i][dim];
            }
            if (power < min_power || i == 0) {
                min_power = power;
            }
            if (power > max_power || i == 0) {
                max_power = power;
            }
            
        }
        distortion /= (opts_l->dim * rv_l->test_out->size);
        power /= (opts_l->dim * rv_l->test_out->size);
        mean /= (opts_l->dim * rv_l->test_out->size);

        double snr = 10*log10((power - (mean*mean)) / distortion);
        double psnr = 10*log10((max_power - min_power) / distortion);
        printf("distortion=%f\n", distortion);
        FILE *fp = fopen("snr.out", (epsilon == 0) ? "w":"a");
        fprintf(fp, "%f,%f\n", epsilon, snr);
        fclose(fp);
        fp = fopen("psnr.out", (epsilon == 0) ? "w":"a");
        fprintf(fp, "%f,%f\n", epsilon, psnr);
        fclose(fp);
    }
}

void iter_nsplits(struct cmdline *opts_l, struct cmdline *opts_r,
                  struct retval *rv_l, struct retval *rv_r) {
    /* Iterate through nsplits for two sources */
    for (uint rate_x = 0; rate_x < 7; rate_x++) {
        for (uint rate_y = 0; rate_y < 7; rate_y++) {
            opts_l->nsplits = rate_x;
            opts_r->nsplits = rate_y;

            run(opts_l, rv_l);
            run(opts_r, rv_r);

            /* Measure the joint PSNR, SNR */
            double distortion = 0;
            double power = 0;
            double mean = 0;
            double max_power = 0;
            double min_power = 0;
            for (size_t i = 0; i < rv_l->test_out->size; i++) {
                for (uint dim = 0; dim < opts_l->dim; dim++) {
                    distortion += (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]) * (rv_l->test_out->v[i][dim] - rv_l->test_in->v[i][dim]);
                    distortion += (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]) * (rv_r->test_out->v[i][dim] - rv_r->test_in->v[i][dim]);
                    power += rv_l->test_in->v[i][dim] * rv_l->test_in->v[i][dim];
                    power += rv_r->test_in->v[i][dim] * rv_r->test_in->v[i][dim];
                    mean += rv_l->test_in->v[i][dim];
                    mean += rv_r->test_in->v[i][dim];
                }
                if (power < min_power || i == 0) {
                    min_power = power;
                }
                if (power > max_power || i == 0) {
                    max_power = power;
                }
                
            }
            distortion /= (opts_l->dim * rv_l->test_out->size);
            power /= (opts_l->dim * rv_l->test_out->size);
            mean /= (opts_l->dim * rv_l->test_out->size);

            double snr = 10*log10((power - (mean*mean)) / distortion);
            double psnr = 10*log10((max_power - min_power) / distortion);
            printf("distortion=%f\n", distortion);
            FILE *fp = fopen("snr.out", "a");
            fprintf(fp, "%f%s", snr, (rate_y < 7-1) ? "\t":"\n");
            fclose(fp);
            fp = fopen("psnr.out", "a");
            fprintf(fp, "%f%s", psnr, (rate_y < 7-1) ? "\t":"\n");
            fclose(fp);
        }
    }
}


int main(int argc, char *argv[]) {

    /* Note this main function is hacked to run different simulations for
     * the two source scalar case i.e.
     * opts.dim is 1, opts.
     * Make sure the two test/train csvs are same size then call
     * and are names tr_left.csv,  tr_right.csv, test_left.csv test_right.csv
     * ./covq/covq */
    
    /* Two source parameters */
    // struct cmdline opts_l;
    // struct cmdline opts_r;
    // struct retval *rv_l = malloc(sizeof(struct retval));
    // struct retval *rv_r = malloc(sizeof(struct retval));

    // opts_l.train = 1;
    // opts_r.train = 1;
    // opts_l.train_csv = "tr_left.csv";
    // opts_r.train_csv = "tr_right.csv";
    // opts_l.codebook_out = "cb_out_l.csv";
    // opts_r.codebook_out = "cb_out_r.csv";
    // opts_l.cw_map_out = "cw_out_l.csv";
    // opts_r.cw_map_out = "cw_out_r.csv";
    // opts_l.lbg_eps = 0.001;
    // opts_r.lbg_eps = 0.001;
    // opts_l.codevector_displace = 0.001;
    // opts_r.codevector_displace = 0.001;

    // opts_l.test = 1;
    // opts_r.test = 1;
    // opts_l.test_in_csv = "test_left.csv";
    // opts_r.test_in_csv = "test_right.csv";
    // opts_l.codebook_in = "cb_out_l.csv";
    // opts_r.codebook_in = "cb_out_r.csv";
    // opts_l.cw_map_in = "cw_out_l.csv";
    // opts_r.cw_map_in = "cw_out_r.csv";
    // opts_l.test_out_csv = "test_out_l.csv";
    // opts_r.test_out_csv = "test_out_r.csv";

    // opts_l.seed = 1234;
    // opts_r.seed = 1234;
    // opts_l.dim = 1;
    // opts_r.dim = 1;
    // opts_l.nsplits = 4;
    // opts_r.nsplits = 4;
    // opts_l.bep = 0.001;
    // opts_r.bep = 0.001;

    /* Varying correlation */
    // single_2src(&opts, rv);
    // free_retval(rv_l, &opts_l);
    // free_retval(rv_r, &opts_r);


    /* Varying channel epsilon */
    // iter_epsilon(&opts_l, &opts_r, rv_l, rv_r)
    // free_retval(rv_l, &opts_l);
    // free_retval(rv_r, &opts_r);

    /* Varying bitrates */
    // iter_nsplits(&opts_l, &opts_r, rv_l, rv_r)
    // free_retval(rv_l, &opts_l);
    // free_retval(rv_r, &opts_r);


    /* BEGIN NORMAL (single source) CODE */
    struct retval *rv = malloc(sizeof(struct retval));
    struct cmdline opts = parse_args(argc, argv);
    /* Write to files */
    if (opts.test) {
        FILE *fp = fopen(opts.test_out_csv, "w");
        print_vectorset(fp, rv->test_out);
        fclose(fp);
    }
    free_retval(rv, &opts);

    return 0;
}
