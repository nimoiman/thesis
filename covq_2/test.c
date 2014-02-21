#include "covq.h"
#undef NDEBUG
#include <assert.h>
#include <float.h>

#define ARR_ROWS 10
#define ARR_COLS 4

int trset_size;
double *trset_x, *trset_y;

void channel_test(void){
    int i,j,k,l;
    for(i = 0; i < CODEBOOK_SIZE_X; i++){
        for(j = 0; j < CODEBOOK_SIZE_Y; j++){
            for(k = 0; k < CODEBOOK_SIZE_X; k++){
                for(l = 0; l < CODEBOOK_SIZE_Y; l++){
                    printf("i=%d,j=%d,k=%d,l=%d,i_cw=%d,j_cw=%d,k_cw=%d,l_cw=%d,p(i,j,k,l)=%lf\n",i,j,k,l,bin_cw_x[i],bin_cw_y[j],bin_cw_x[k],bin_cw_y[l],channel_prob(i,j,k,l));
                }
            }
        }
    }
}
void print_test(void){
    int i, j;
    int int_arr[ARR_COLS];
    double double_arr[ARR_COLS];
    int int_arr_2D[ARR_ROWS][ARR_COLS];
    
    printf("testing print int array...\n");

    for( i = 0; i < ARR_COLS; i++)
        int_arr[i] = i+1;

    print_int_array(stdout, int_arr, ARR_COLS);
    printf("\n");
    
    for(i = 0; i < ARR_COLS; i++)
        double_arr[i]=((double)rand()/(double)RAND_MAX);
    
    printf("testing print double array...\n");
    print_double_array(stdout, double_arr, ARR_COLS);
    printf("\n");
    
    for( i = 0; i < ARR_ROWS; i++ )
        for( j = 0; j < ARR_COLS; j++ )
            int_arr_2D[i][j] = j;

    printf("testing print int matrix...\n");
    print_int_array_2d(stdout, (int*) int_arr_2D, ARR_ROWS, ARR_COLS);
    printf("\n");

}

void quantize_test() {
    int i;
    double q_centroid_x, q_centroid_y, v;
    double max_quantize_error_x, max_quantize_error_y, quantize_error_x,
        quantize_error_y;
    int *outlier = malloc(sizeof(int));
    int outlier_count = 12;
    trset_size = 16;

    double dummy_x[] = {0, -0.2, 0.3, 0.2, 0.1, 0.7, 0.5, 0.6, 0.3, 0.001,
        0.2, 0.33, 0.001, 0.2, 1.5, -0.5};

    double dummy_y[] = {-0.1, -5, -0.5, -0.005, 0.6, 0.001, 1.5, 0.001, 0.5, 0.6,
        0.7, 0.75, 0.777, 0.8, 0.923, 0.0};

    printf("quant: %d\n", vec_to_quant(dummy_x[0], NULL, SRC_X));

    trset_x = malloc(sizeof(double) * trset_size);
    trset_y = malloc(sizeof(double) * trset_size);

    printf("testing that quantize error is valid\n");
    max_quantize_error_x = ((double) Q_LENGTH_X)/(2*Q_LEVELS);
    max_quantize_error_y = ((double) Q_LENGTH_Y)/(2*Q_LEVELS);
    printf("max quantize error for X is %.3f\n", max_quantize_error_x);
    printf("max quantize error for Y is %.3f\n", max_quantize_error_y);
    for (i = 0; i < trset_size; i++) {
        trset_x[i] = dummy_x[i];
        trset_y[i] = dummy_y[i];
        v = trset_x[i];
        
        q_centroid_x = quant_to_vec(vec_to_quant(trset_x[i], outlier, SRC_X),
            SRC_X);
        if (*outlier) {
            printf("Input: (%.3f, %.3f), outlier detected.\n", trset_x[i],
                trset_y[i]);
            continue;
        }
        q_centroid_y = quant_to_vec(vec_to_quant(trset_y[i], outlier, SRC_Y),
            SRC_Y);
        if (*outlier) {
            printf("Input: (%.3f, %.3f), outlier detected.\n", trset_x[i],
                trset_y[i]);
            continue;
        }
        printf("Input: (%.3f, %.3f), Output: (%.3f, %.3f)\n", trset_x[i],
            trset_y[i], q_centroid_x, q_centroid_y);
        quantize_error_x = fabs(q_centroid_x - trset_x[i]);
        quantize_error_y = fabs(q_centroid_y - trset_y[i]);
        printf("Quantize error: (%.3f, %.3f)\n", quantize_error_x,
            quantize_error_y);
        assert(quantize_error_x <= max_quantize_error_x + DBL_EPSILON);
        assert(quantize_error_y <= max_quantize_error_y + DBL_EPSILON);
    }
    printf("passed.\n");

    printf("checking outlier count\n");
    assert(quantize() == outlier_count);
    printf("passed.\n");

    print_int_array_2d(stdout, (int*) q_trset, Q_LEVELS, Q_LEVELS);
}

void csv_io_test() {
    FILE *stream = fopen("lalala.csv", "w");
    fprintf(stream, "1.00,2.34\n1.23,2.56\n");
    fclose(stream);
    stream = fopen("lalala.csv", "r");
    read_csv_trset(stream);
    printf("\n");
    printf("X training set:\n");
    print_double_array(stdout, trset_x, trset_size);
    printf("\n");
    printf("Y training set:\n");
    print_double_array(stdout, trset_y, trset_size);
    printf("\n");
    fclose(stream);

}

int main( int argc, const char* argv[] ){
    // init_binary_codewords();
    // channel_test();
    // print_test();
    
    // csv_io_test();
}



