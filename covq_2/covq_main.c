#include <math.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include "covq.h"

#define LINE_LEN 100

double trans_prob_x = 0.01;
double trans_prob_y = 0.01;

int B_X, B_Y;

/* Channel transition probability
 * Computes the channel transition probability P(k,l | i,j). That is, the
 * probability of receiving indexes k and l from source X and Y respectively,
 * given that index i was sent from source X and j was sent from source Y. Take
 * note that the channels do not need to be independent.
 * Asserts:
 * i, j, k, and l are valid codebook indexes.
 * return value is between 0 and 1.
 */
double trans_prob(int i, int j, int k, int l, int *b_X, int *b_Y)
{
    double prob = 1;

    // Get binary transmission codewords for the indexes
    int i_cw = b_X[i];
    int j_cw = b_Y[j];
    int k_cw = b_X[k];
    int l_cw = b_Y[l];

    // Compute transition probability
    int diff = i_cw ^ k_cw;
    for(int n = 0; n < B_X; n++)
        prob *= (diff>>n & 1) ? trans_prob_x: (1-trans_prob_x);
    diff = j_cw ^ l_cw;
    for(int n = 0; n < B_Y; n++)
        prob *= (diff>>n & 1) ? trans_prob_y: (1-trans_prob_y);

    return prob;
}

double run(int N_X, int N_Y, int L, double min_x, double max_x, double min_y, double max_y, const char *filename)
{

    char train_file[LINE_LEN];
    sprintf(train_file, "%s.train", filename);

    covq2 v;
    unif_quant q;
    // printf("initializing quant\n");

    quantizer_init( &q, L, L, min_x, max_x, min_y, max_y);

    /*
     * Read training set from file.
     */
    FILE *pFile = fopen(train_file, "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }

    char line[LINE_LEN];
    while(fgets(line, LINE_LEN, pFile) != NULL){
        double x, y;
        int param_count = sscanf(line, "%lf,%lf", &x, &y);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            return 0;
        }

        int qx = val_to_quant(x, src_X, &q);
        int qy = val_to_quant(y, src_Y, &q);
        quantizer_bin(qx, qy, &q);
    }

    fclose(pFile);
    // printf("finished reading training set\n");

    initilization_stage_covq2(&v, &q, N_X, N_Y);
    // printf("finished initializing stage\n");

    /*
     * Read simulation set from file.
     */
    char simFile_name[LINE_LEN];
    sprintf(simFile_name, "%s.sim", filename);
    FILE *simFile = fopen(simFile_name, "r");
    if( !simFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }

    /*
     * Open output file
     */
    char outFile_name[LINE_LEN];
    sprintf(outFile_name, "%s.out", filename);
    FILE *outFile = fopen(outFile_name, "w");
    if( !outFile ){
        fprintf(stderr, "Could not open output file.\n");
        fclose(outFile);
        return 0;
    }

    double err = 0;
    int num_sim = 0;

    while(fgets(line, LINE_LEN, simFile) != NULL){
        double x, y;
        int param_count = sscanf(line, "%lf,%lf", &x, &y);
        num_sim++;
        if( param_count != 2 ){
            fprintf(stderr, "Invalid simulation set format.\n");
            fclose(outFile);
            fclose(simFile);
            return 0;
        }

        /*
         * Quantize X and Y
         */
        int qx = val_to_quant(x, src_X, v.q);
        int qy = val_to_quant(y, src_Y, v.q);

        /*
         * Ignore point if outside of uniform quantizer range.
         */
        if( qx == -1 || qy == -1){
            fprintf(stderr, "Invalid Point!\n");
            fclose(outFile);
            fclose(simFile);
            return 0;
        }

        int i = v.I_X[qx];
        int j = v.I_Y[qy];

        double x_ij = v.x_ij[i][j];
        double y_ij = v.y_ij[i][j];

        fprintf(outFile, "%lf,%lf\n", x_ij, y_ij);


        err += POW2(x-x_ij) + POW2(y-y_ij);
    }

    err /= num_sim;

    fclose(outFile);
    fclose(simFile);
    quantizer_free(v.q);
    free_covq2(&v);

    return err;
}


int main(int argc, const char **argv)
{
    int N_X, N_Y;
    double min_x, max_x, min_y, max_y;

    if(argc != 8){
        fprintf(stderr, "Need 8 arguments: N_X, N_Y, min_x, max_x, min_y, max_y, filename\n");
        return 1;
    }

    N_X   = atoi(argv[1]);
    N_Y   = atoi(argv[2]);
    min_x = atof(argv[3]);
    max_x = atof(argv[4]);
    min_y = atof(argv[5]);
    max_y = atof(argv[6]);

    double err_best = DBL_MAX;
    int L_best;
    for(int L = 25; L <= 600; L+= 25){
        // printf("helloooo\n");
        double err = run(N_X, N_Y, L, min_x, max_x, min_y, max_y, argv[7]);
        if(err == 0){
            fprintf(stderr,"An error occured\n");
            return 1;
        }
        if(err < err_best){
            err_best = err;
            L_best = L;
        }
    }

    printf("%f\t\n",err_best);
    return 0;
}

