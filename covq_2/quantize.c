#include "quantize.h"

/*
 * Initialize Uniform Quantizer
 * Allocates memory to store quantized points. Quantizer parameters should be
 * already set in q.
 */
int quantizer_init
(unif_quant *q, int L_X, int L_Y,
 double min_x, double max_x, double min_y, double max_y)
{
    q->L_X = L_X;
    q->L_Y = L_Y;
    q->min_x = min_x;
    q->max_x = max_x;
    q->min_y = min_y;
    q->max_y = max_y;
    q->npoints = 0;

    q->quant = (int*) calloc((size_t) L_X * L_Y, sizeof(int));

    if(q->quant == NULL){
        fprintf(stderr, "Could not allocate memory for uniform quantizer.\n");
        return 0;
    }

    return 1;
}

/*
 * Destroy Uniform Quantizer
 * Frees memory allocated by the uniform quantizer and clears quantizer
 * parameters.
 */
void quantizer_free(unif_quant *q)
{
    q->L_X = 0;
    q->L_Y = 0;
    q->npoints = 0;
    q->min_x = 0;
    q->max_x = 0;
    q->min_y = 0;
    q->max_y = 0;
    free(q->quant);
    q->quant = NULL;

}

/*
 * Convert value to quantizer bin index.
 * Takes a value and returns the quantizer bin for the given source (s). If the
 * value is out of range of the quantizer, then return -1.
 */
int val_to_quant(double val, enum source s, unif_quant *q)
{
    int qval;
    int nbins;
    double len;
    double min, max;
    
    if(s == src_X){
        min = q->min_x;
        max = q->max_x;
        nbins = q->L_X;
    }
    else{
        min = q->min_y;
        max = q->max_y;
        nbins = q->L_Y;
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

/* Convert quantizer bin index to value.
 * Takes a quantizer bin index and returns the mean value of the corresponding
 * quantizer bin. Note that there is no error checking.
 */
double quant_to_val(int qval, enum source s, unif_quant *q)
{
    double val;
    int nbins;
    double len;
    double min, max;
    
    if(s == src_X){
        min = q->min_x;
        max = q->max_x;
        nbins = q->L_X;
    }
    else{
        min = q->min_y;
        max = q->max_y;
        nbins = q->L_Y;
    }
    len = max - min;

    val = (qval + 0.5) / nbins;
    val = len * val;
    val = val + min;

    return val;
}

/*
 * Bins a point into the quantizer.
 * Takes a quantizer index pair for both sources and adds the point to the
 * quantizer. Returns 0 if the index pair is our of range of the quantizer,
 * otherwise returns 1.
 */
int quantizer_bin(int qx, int qy, unif_quant *q)
{
    if(qx >= 0 && qx < q->L_X && qy >= 0 && qy < q->L_Y){
        q->quant[(long) q->L_X*qy + qx]++;
        q->npoints++;
        return 1;
    }
    else
        return 0;
}

/*
 * Gets number of points for index pair.
 * Returns the number of points that fall into the quantizer bin given by the
 * provided index pair. If the index pair is out of range, return -1.
 */
int quantizer_get_count(int qx, int qy, unif_quant *q)
{
    if(qx >= 0 && qx < q->L_X && qy >= 0 && qy < q->L_Y)
        return q->quant[(long) q->L_X*qy + qx];
    else
        return -1;
}

/*
 * Tests the uniform quantizer.
 */
void quantizer_test()
{
    double val_x, val_y, val2;
    int qx, qy;
    int i, j;
    int count;

    unif_quant q;

    /*
     * Test init
     */
    quantizer_init( &q, 100, 1000, -1, 1, 3, 5);
    assert(q.quant != NULL);

    /*
     * Test conversion
     * Convert to qval and back. Ensure distance between reconstruction and
     * original value is less than half a bin size.
     */

    // Test in src_X
   for(i = 0; i < 100; i++){
       val_x = i / 100.0;
       val_x *= q.max_x - q.min_x;
       val_x += q.min_x;
       qx = val_to_quant(val_x, src_X, &q);
       val2 = quant_to_val(qx, src_X, &q);
       assert(fabs(val_x - val2) <=
               0.5001 * (q.max_x - q.min_x) / q.L_X );
   }

   // Test in src_Y
   for(i = 0; i < 100; i++){
       val_y = i / 100.0;
       val_y *= q.max_y - q.min_y;
       val_y += q.min_y;
       qy = val_to_quant(val_y, src_Y, &q);
       val2 = quant_to_val(qy, src_Y, &q);
       assert(fabs(val_y - val2) <=
               0.5001 * (q.max_y - q.min_y) / q.L_Y );
   }

   /*
    * Test binning
    * Place points into bins. Test with outliers as well.
    */

   //Test outliers in src_X
   for(i = 0; i < 100; i++){
       val_x = q.min_x - 0.1 - i; 
       qx = val_to_quant(val_x, src_X, &q);
       qy = val_to_quant((q.min_y + q.max_y) / 2, src_X, &q);
       assert(quantizer_bin(qx, qy, &q) == 0);
       assert(quantizer_get_count(qx, qy, &q) == -1);
       assert(q.npoints == 0);
   }

   //Test outliers in src_Y
   for(i = 0; i < 100; i++){
       val_y = q.min_y - 0.1 - i; 
       qy = val_to_quant(val_y, src_Y, &q);
       qx = val_to_quant((q.min_x + q.max_x) / 2, src_Y, &q);
       assert(quantizer_bin(qx, qy, &q) == 0);
       assert(quantizer_get_count(qx, qy, &q) == -1);
       assert(q.npoints == 0);
   }

   // Test inliers in src_X and src_Y
   for(i = 0; i < 10; i++){
       val_x = q.min_x + (q.max_x - q.min_x)/10 * i;
       qx = val_to_quant(val_x, src_X, &q);
       for(j = 0; j < 10; j++){
           val_y = q.min_y + (q.max_y - q.min_y)/10 * j;
           qy = val_to_quant(val_y, src_Y, &q);
           count = quantizer_get_count( qx, qy, &q);
           quantizer_bin( qx, qy, &q);
           assert(count + 1 == quantizer_get_count( qx, qy, &q));
       }
   }
   assert(q.npoints == 100);

   quantizer_free( &q );

   printf("quantizer test passed.\n");
}
