#include <math.h>
#include <stdio.h>
#include "covq.h"

#define LINE_LEN 100

#define TRSET_FILE "training_set.csv"
#define SIM_FILE "simulation_set.txt"

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
    int n, diff;
    int i_cw, j_cw, k_cw, l_cw;


    // Get binary transmission codewords for the indexes
    i_cw = b_X[i];
    j_cw = b_Y[j];
    k_cw = b_X[k];
    l_cw = b_Y[l];
    
    // Compute transition probability
    diff = i_cw ^ k_cw;
    for(n = 0; n < B_X; n++)
        prob *= (diff>>n & 1) ? trans_prob_x: (1-trans_prob_x);
    diff = j_cw ^ l_cw;
    for(n = 0; n < B_Y; n++)
        prob *= (diff>>n & 1) ? trans_prob_y: (1-trans_prob_y);
    
    return prob;
}

int get_range(char *filename, double *min_x, double *max_x, double *min_y, double *max_y)
{
    FILE *pFile;
    char line[LINE_LEN];
    int first = 1;

    pFile = fopen(filename, "r");
    if(!pFile){
        fprintf(stderr,"Couln't read get_range file.\n");
        return 0;
    }

    while(fgets(line, LINE_LEN, pFile) != NULL){
        double x, y;
        int param_count = sscanf(line, "%lf, %lf", &x, &y);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            fclose(pFile);
            return 0;
        }
        if(first){
            first = 0;
            *min_x = x;
            *max_x = x;
            *min_y = y;
            *max_y = y;
        }
        if(x < *min_x)
            *min_x = x;
        if(x > *max_x)
            *max_x = x;
        if(y < *min_y)
            *min_y = y;
        if(y > *max_y)
            *max_y = y;
    }
    fclose(pFile);
    return 1;
}

double min(double x, double y)
{
    return x < y ? x : y;
}

double max(double x, double y)
{
        return x > y ? x : y;
}

double run(int N_X, int N_Y, int L, char *filename)
{
    FILE *pFile, *outFile;
    covq2 v;
    unif_quant q;
    char line[LINE_LEN];
    int param_count;

    double x, y;
    int qx, qy;
    double T_X=0, S_X=0;
    double T_Y=0, S_Y=0;
    double signal_power;
    double quantized_noise_power;
    double total_noise;
    double SQNR;
    int i, j;
    double x_ij, y_ij;

    char train_file[LINE_LEN];
    char sim_file[LINE_LEN];
    char out_file[LINE_LEN];

    double L_X, L_Y;

    double min_x, max_x, min_y, max_y;
    double min_x_sim, max_x_sim, min_y_sim, max_y_sim;
    double min_x_train, max_x_train, min_y_train, max_y_train;

    sprintf(train_file, "%s.train", filename);
    sprintf(sim_file, "%s.sim", filename);
    sprintf(out_file, "%s.out", filename);

    get_range(train_file, &min_x_train, &max_x_train, &min_y_train, &max_y_train);
    get_range(sim_file, &min_x_sim, &max_x_sim, &min_y_sim, &max_y_sim);
    min_x = min(min_x_sim, min_x_train) - 0.0001;
    max_x = max(max_x_sim, max_x_train) + 0.0001;
    min_y = min(min_y_sim, min_y_train) - 0.0001;
    max_y = max(max_y_sim, max_y_train) + 0.0001;

    L_X = L_Y = L;

    quantizer_init( &q, L_X, L_Y, min_x, max_x, min_y, max_y);

    /*
     * Read training set from file.
     */
    pFile = fopen(train_file, "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }

    while(fgets(line, LINE_LEN, pFile) != NULL){
        param_count = sscanf(line, "%lf, %lf", &x, &y);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid training set format.\n");
            return 0;
        }

        qx = val_to_quant(x, src_X, &q);
        qy = val_to_quant(y, src_Y, &q);
        quantizer_bin(qx, qy, &q);
    }

    fclose(pFile);

    initilization_stage_covq2(&v, &q, N_X, N_Y);

    /*
     * Read simulation set from file.
     */
    pFile = fopen(sim_file, "r");
    if( !pFile ){
        fprintf(stderr, "Could not open training set file.\n");
        return 0;
    }

    /*
     * Open output file
     */
    outFile = fopen(out_file, "w");
    if( !pFile ){
        fprintf(stderr, "Could not open output file.\n");
        return 0;
    }
    
    total_noise = 0;
    while(fgets(line, LINE_LEN, pFile) != NULL){
        param_count = sscanf(line, "%lf, %lf", &x, &y);
        if( param_count != 2 ){
            fprintf(stderr, "Invalid simulation set format.\n");
            return 0;
        }

        /*
         * Quantize X and Y
         */
        qx = val_to_quant(x, src_X, &q);
        qy = val_to_quant(y, src_Y, &q);

        /*
         * Ignore point if outside of uniform quantizer range.
         */
        if( qx == -1 || qy == -1)
            continue;

        i = v.I_X[qx];
        j = v.I_Y[qy];

        x_ij = v.x_ij[j * v.N_X + i];
        y_ij = v.y_ij[j * v.N_X + i];

        fprintf(outFile, "%lf,%lf\n", x_ij, y_ij);


        /*
         * Compute SQNR
         */
        S_X += x;
        S_Y += y;
        T_X += POW2(x);
        T_Y += POW2(y);

        total_noise += POW2(x - x_ij) + POW2(y-y_ij);

    }

    fclose(pFile);

    S_X /= q.npoints;
    S_Y /= q.npoints;
    T_X /= q.npoints;
    T_Y /= q.npoints;
    signal_power = T_X + POW2(S_X) + T_Y + POW2(S_Y);

    quantized_noise_power = total_noise / q.npoints;

    SQNR = 10 * log10(signal_power / quantized_noise_power);

    free_covq2(&v);
    quantizer_free(&q);

    return SQNR;
}

// 
// void cov_run(int L)
// {
//     int N_final = 4;
//     int L_X = L;
//     int L_Y = L;
//     char train_file[100];
//     char sim_file[100];
// 
//     printf("L:%d\n",L);
//     for(int cov_idx = 0; cov_idx <= 100; cov_idx++){
//         sprintf(train_file, "train_%.2f.csv", 0.01*cov_idx);
//         sprintf(sim_file, "sim_%.2f.csv", 0.01*cov_idx);
// 
//         for(int N = 4; N <= N_final; N *= 2){
//             double SQNR = run(L_X, L_Y, N, N, sim_file);
//             printf("%lf ", SQNR);
//         }
//         printf("\n");
//     }
// }
// 

int image_run(){

    char name[LINE_LEN];

    for(int i=0; i <= 7; i++){
        for(int j=0; j <= 7; j++){
            sprintf(name, "%d%d", i,j);
            printf( "%lf\n", run(4, 4, 200, name) );
        }
    }
    return 1;
}
int main()
{
    int N_X_final, N_Y_final;

    N_X_final = 64;
    N_Y_final = 64;

    image_run();
    return 0;
}

