#include <assert.h>
#include <math.h>
#include "quantize.h"
#define Q_BINS 89
#define Q_MIN -1.0
#define Q_MAX 1.0

void test(){
    int i;
    uniform_quantizer q;
    double val, val2;
    int qval;
    int count=0, sum=0;

    /*
     * Test Initialization
     */
    q.bins = NULL;
    init_quantizer( &q, Q_BINS, Q_MIN, Q_MAX);

    assert(q.npoints == 0);
    assert(q.nbins == Q_BINS);
    assert(q.bins != NULL);
    assert(q.min == Q_MIN);
    assert(q.max == Q_MAX);

    for(i = 0; i < Q_BINS; i++)
        assert(q.bins[i] == 0);

    /*
     * Test Conversion
     */

    for(val = Q_MIN; val < Q_MAX; val += (Q_MAX-Q_MIN)/100){
        qval = val_to_quant(val, &q);
        assert(0 <= qval);
        assert(qval <= Q_BINS - 1);

        val2 = quant_to_val(qval, &q);
        assert(fabs(val - val2) <= 0.5001 * (q.max - q.min) / q.nbins );

        count++;
        bin_val(val, &q);
    }
    
    assert(count == q.npoints);

    for(i = 0; i < q.nbins; i++)
       sum += q.bins[i];
    
    assert(sum == q.npoints);

}

int main( int argc, const char* argv[] ){
    uniform_quantizer q;

    test();

    printf("test passed.\n");
    return 1;
}

