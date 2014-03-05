#include "quantize.h"

enum quantizer_source{
    X,
    Y
};

int quantizer_init(uniform_quantizer *q, int nbins_x, int nbins_y,
        double min_x, double max_x, double min_y, double max_y){

    q->nbins_x = nbins_x;
    q->nbins_y = nbins_y;
    q->npoints = 0;
    q->min_x = min_x;
    q->max_x = max_x;
    q->min_y = min_y;
    q->max_y = max_y;
    q->quant = (int*) calloc((long) nbins_x * nbins_y, sizeof(int));
    
    if(q->quant == NULL){
        fprintf(stderr, "Could not allocate memory for uniform quantizer.\n");
        return 0;
    }

    return 1;
}

void quantizer_destroy(uniform_quantizer *q){

    q->nbins_x = 0;
    q->nbins_y = 0;
    q->npoints = 0;
    q->min_x = 0;
    q->max_x = 0;
    q->min_y = 0;
    q->max_y = 0;
    free(q->quant);
    q->quant = NULL;

}

int val_to_quant(double val, enum quantizer_source s, uniform_quantizer *q){
    int qval;
    int nbins;
    double len;
    double min, max;
    
    if(s == X){
        min = q->min_x;
        max = q->max_x;
        nbins = q->nbins_x;
    }
    else{
        min = q->min_y;
        max = q->max_y;
        nbins = q->nbins_y;
    }
    len = max - min;
        

    val = val - min; // 0 to q_max - q_min
    val = val / len; // 0 to 1
    val = nbins * val; // 0 to q->levels
    qval = (int) val; // 0 to q->levels - 1

    if( qval < 0 || qval > nbins - 1)
        return -1;
    else
        return qval;
}

double quant_to_val(int qval, enum quantizer_source s, uniform_quantizer *q){
    double val;
    int nbins;
    double len;
    double min, max;
    
    if(s == X){
        min = q->min_x;
        max = q->max_x;
        nbins = q->nbins_x;
    }
    else{
        min = q->min_y;
        max = q->max_y;
        nbins = q->nbins_y;
    }
    len = max - min;

    val = (qval + 0.5) / nbins;
    val = len * val;
    val = val + min;

    return val;
}

int quantizer_bin(double val_x, double val_y, uniform_quantizer *q){
    int qval_x, qval_y;

    qval_x = val_to_quant(val_x, X, q);
    qval_y = val_to_quant(val_y, Y, q);
    if(qval_x != -1 && qval_y != -1){
        q->quant[(long) q->nbins_x*qval_y + qval_x]++;
        q->npoints++;
        return 1;
    }
    else
        return 0;
}

int quantizer_get_count(double val_x, double val_y, uniform_quantizer *q){
    int qval_x, qval_y;

    qval_x = val_to_quant(val_x, X, q);
    qval_y = val_to_quant(val_y, Y, q);
    if(qval_x != -1 && qval_y != -1){
        return q->quant[(long) q->nbins_x*qval_y + qval_x];
    }
    else
        return 0;
}

void quantizer_test(){
    double val, val2;
    int qval;
    int i, j;
    int count;

    const int nbins_x = 100;
    const int nbins_y = 1000;
    const int min_x = -1;
    const int max_x = 1;
    const int min_y = 3;
    const int max_y = 5;

    uniform_quantizer q;

    /*
     * Test init
     */

    quantizer_init( &q, nbins_x, nbins_y, min_x, max_x, min_y, max_y);
    assert(q.nbins_x == nbins_x);
    assert(q.nbins_y == nbins_y);
    assert(q.min_x == min_x);
    assert(q.max_x == max_x);
    assert(q.min_y == min_y);
    assert(q.max_y == max_y);

    /*
     * Test conversion
     * Convert to qval and back. Ensure distance between reconstruction and
     * original value is less than half a bin size.
     */

   for(i = 0; i < 100; i++){
       val = i / 100.0;
       val *= q.max_x - q.min_x;
       val += q.min_x;
       qval = val_to_quant(val, X, &q);
       val2 = quant_to_val(qval, X, &q);
       assert(fabs(val - val2) <=
               0.5001 * (q.max_x - q.min_x) / q.nbins_x );
   }

   for(i = 0; i < 100; i++){
       val = i / 100.0;
       val *= q.max_y - q.min_y;
       val += q.min_y;
       qval = val_to_quant(val, Y, &q);
       val2 = quant_to_val(qval, Y, &q);
       assert(fabs(val - val2) <=
               0.5001 * (q.max_y - q.min_y) / q.nbins_y );
   }

   /*
    * Test binning
    * Place points into bins. Test with outliers as well.
    */

   for(i = 0; i < 100; i++){
       val = q.min_x - 0.1 - i; 
       assert(quantizer_bin(val, (q.min_y + q.max_y) / 2, &q) == 0);
       assert(quantizer_get_count(val, (q.min_y + q.max_y) / 2, &q) == 0);
       assert(q.npoints == 0);
   }

   for(i = 0; i < 100; i++){
       val = q.min_y - 0.1 - i; 
       assert(quantizer_bin((q.min_x + q.max_x) / 2, val, &q) == 0);
       assert(quantizer_get_count((q.min_x + q.max_x) / 2, val, &q) == 0);
       assert(q.npoints == 0);
   }

   for(i = 0; i < 10; i++){
       val = q.min_x + (q.max_x - q.min_x)/10 * i;
       for(j = 0; j < 10; j++){
           val2 = q.min_y + (q.max_y - q.min_y)/10 * j;
           count = quantizer_get_count( val, val2, &q);
           quantizer_bin( val, val2, &q);
           assert(count + 1 == quantizer_get_count( val, val2, &q));
       }
   }
   assert(q.npoints == 100);

   quantizer_destroy( &q );

   printf("quantizer test passed.\n");
}
