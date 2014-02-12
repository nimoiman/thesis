#!/usr/bin/octave -qf
# Octave command line utility to output a multivariate Gaussian CSV file

pkg load statistics; # for mvnrnd()

global defaults = struct("data_size", 500, "cov_matrix", [1,0.5;0.5,1], "means", [0;0]);

function print_help()
    global defaults;
    fprintf(stderr, "Output a multivariate normal random sampling to a csv file\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "$ octave %s <OUTPUT_CSV> [DATA_SIZE] [COV_MATRIX] [MEANS]\n", program_name());
    fprintf(stderr, "\n");
    fprintf(stderr, "Parameters:\n");
    fprintf(stderr, "<OUTPUT_CSV>: filename of csv to store data\n");
    fprintf(stderr, "[DATA_SIZE]: number of sample points\n");
    fprintf(stderr, "    default: %d\n", defaults.data_size);
    fprintf(stderr, "[COV_MATRIX]: covariance matrix of the multivariate Gaussian random vector\n");
    fprintf(stderr, "    default: %s\n", mat2str(defaults.cov_matrix));
    fprintf(stderr, "[MEANS]: array of means of the multivariate Gaussian random vector\n");
    fprintf(stderr, "    default: %s\n", mat2str(defaults.means));
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "$ octave %s out_file.csv 200 '[1 0;0 1]' '[0;0]'", program_name());
endfunction

function [file_name, data_size, cov_matrix, means] = parse_args()
    global defaults;

    if (length(argv()) == 1 && argv(){1} == "help")
        print_help();
        exit(0);
    endif
    if (length(argv()) == 1)
        file_name = argv(){1};
        data_size = defaults.data_size;
        cov_matrix = defaults.cov_matrix;
        means = defaults.means;
    elseif (length(argv()) == 4)
        file_name = argv(){1};
        data_size = eval(argv(){2});
        cov_matrix = eval(argv(){3});
        means = eval(argv(){4});
    else
        print_help();
        exit(1);
    endif
endfunction

# BEGIN MAIN CODE

[file_name, data_size, cov_matrix, means] = parse_args();
data = mvnrnd(means, cov_matrix, data_size);
csvwrite(file_name, data);
