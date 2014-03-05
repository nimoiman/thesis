#include <assert.h>
#include <math.h>
#include "quantize.h"
#define Q_LEVELS 89
#define Q_MIN -1.0
#define Q_MAX 1.0

void test_init(){
    int i;
    uniform_quantizer q;

    q.bins = NULL;
    init_quantizer( &q, Q_LEVELS, Q_MIN, Q_MAX);

    assert(q.num == 0);
    assert(q.bins != NULL);
    assert(q.min == Q_MIN);
    assert(q.max == Q_MAX);

    for(i = 0; i < Q_LEVELS; i++)
        assert(q.bins[i] == 0);
}

void test_conversion(){
    double val = 0.3;
    double val2;
    double qval;
    int count = 0;
    int i, sum = 0;
    uniform_quantizer q;

    init_quantizer(&q, Q_LEVELS, Q_MIN, Q_MAX);

    for(val = Q_MIN; val < Q_MAX; val += (Q_MAX-Q_MIN)/100){
        qval = val_to_quant(val, &q);
        assert(0 <= qval);
        assert(qval <= Q_LEVELS - 1);

        val2 = quant_to_val(qval, &q);
        assert(fabs(val - val2) <= 0.5001 * (q.max - q.min) / q.levels );

        count++;
        bin_val(val, &q);
    }
    
    assert(count == q.num);

    for(i = 0; i < q.levels; i++)
       sum += q.bins[i];
    
    assert(sum == q.num);
}

int main( int argc, const char* argv[] ){
    uniform_quantizer q;

    test_init();
    test_conversion();

    printf("test passed.\n");
    return 1;
}

