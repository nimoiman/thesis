#include "covq.h"
#undef NDEBUG
#include <assert.h>
#include <float.h>

int trset_size;
double *trset_x, *trset_y;

void print_test(void){
    int i;
    int int_arr[25];
    double double_arr[25];
    
    printf("testing print int array...\n");

    for( i = 0; i < 25; i++)
        int_arr[i] = i+1;

    print_int_array(stdout, int_arr, 25);
    printf("\n");
    
    for(i = 0; i < 25; i++)
        double_arr[i]=((double)rand()/(double)RAND_MAX);
    
    printf("testing print double array...\n");
    print_double_array(stdout, double_arr, 25);
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
    print_test();
    quantize_test();
    csv_io_test();
}



