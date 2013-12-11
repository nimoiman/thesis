#include "vq.h"

struct _vectorset{
	size_t size;
	double *v[VECTOR_DIM];
};

// Initializes a vectorset for a given size. Returns 1 on success, otherwise 0
// Remember to call destroy_vector when you are done
int init_vectorset(vectorset *vset, size_t size){
	int i;
	double *v;

	v = (double*) malloc(size*VECTOR_DIM*sizeof(double));
	if(v){
		vset->size = size;
		for(i = 0; i < VECTOR_DIM; i++)
			vset->v[i] = v + VECTOR_DIM*i;
		return 1;
	}
	return 0;
}

void destroy_vectorset(vectorset *vset){
	free(vset->v);
}
// Prints a vectorset to standard output
// One vector per row, tab delimited
void print_vectorset(vectorset *vset){
	int i, j;
	for(i = 0; i < vset->size; i++){
		for(j = 0; j < VECTOR_DIM; j++)
			printf("%f\t", vset->v[i][j]);
		printf("\n");
	}
}
	
// Distortion function. Returns the distortion between vector x and vector y.
double dist(double *x, double *y){
       int i;
       double d = 0;
       for(i = 0; i < VECTOR_DIM; i++)
	       d += pow(x[i] - y[i],2);
       return d;
}

// Finds the vector of minimal distortion within a given codebook
// Provide a vector x, codebook c
// parameter *idx is set to codebook index of codevector with minimal distortion
// The minimal distortion between x and the codevector is returned.
// This function just performs a linear search between the vector x and the codebook
// Makes use of the dist() function
double nearest_neighbour(double *x, vectorset *c, int *idx){
	int minarg, i;
	double d, bestsofar;
	bestsofar = dist(x, c->v[0]);
	minarg = 0;
	for(i = 1; i < c->size; i++){
		d = dist(x, c->v[i]);
		if(d < bestsofar){
			minarg = i;
			bestsofar = d;
		}
	}	
	*idx = minarg;
	return bestsofar;
}

// Updates the centroids in a signle iteration of the LBG-VQ scheme
// This function re-adjusts the codevectors based on the centroid condition
// This function does not do any splitting
// A training set should be provided
// **c1 should initially point to the current codebook
// **c2 should point to a vectorset the same size as c1
// *count should point to an integer array the same size as the codebook
// *count is used internally, and it passed to prevent excessive malloc and free calls on each function call
// After computing the updating centroids, the new codebook is placed under *c1
// The old codebook is placed under *c2
// The average distortion between the training set and the new codebook is returned
double centroid_update(vectorset *train, vectorset **c1, vectorset **c2, int *count){
	int i, j, nn;
	vectorset *tmp;
	double d_avg = 0;

	//We will place the new codebook in c2 and then swap c1 and c2 at the end
	
	// Initialize counts and new codebook to 0
	for(i = 0; i < (*c2)->size; i++){
		count[i] = 0;
		for(j = 0; j < VECTOR_DIM; j++)
		       (*c2)->v[i][j] = 0;	
	}

	// For each training vector, we find the codevector of minimal distortion
	// i.e. the codevector for that training vector's encoding region
	// We want to place the new codevector at the mean of the training vectors that map to it
	// That is sum of all code vectors / number of codevectors
	for(i = 0; i < train->size; i++){
		d_avg += nearest_neighbour( train->v[i], *c1, &nn);
		count[nn]++;
		for(j = 0; j < VECTOR_DIM; j++)
			(*c2)->v[nn][j] += train->v[i][j];
	}

	// Here we divide the new codevectors by the number of mapped vectors to get the mean
	for(i = 0; i < (*c2)->size; i++)
		for(j = 0; j < VECTOR_DIM; j++)
			(*c2)->v[i][j] /= count[i];	

	// Finally swap c1 and c2
	tmp = *c1;
	*c1 = *c2;
	*c2= tmp;
	// Get average distortion by dividing by the total training set size	
	d_avg /= train->size;
	return d_avg;
}

// Performs the LBG VQ scheme on a given training set
// The training set should be provided, along with the number of times the codebook should split
// Recall that the total codebook size is therefore 2^(nsplits)
// A pointer to the new codebook is returned
vectorset *lbgvq(vectorset *v, int nsplits){
	vectorset *c1, *c2, *tmp;
	int i, j, k, max_cv, *counts;
	double d_old, d_new;

	max_cv = 1<<nsplits; // Final codebook size

	init_vectorset(c1, max_cv); // Initialize the codebook
	if(c1){
		init_vectorset(c2, max_cv);
		if(c2){
			counts = (int*) malloc(max_cv*sizeof(int));
			if(counts){
				// Everything initialized properly

				//We first set the initial codevector at the centroid of the training data
				for(i = 0; i < VECTOR_DIM; i++){
					c1->v[0][i] = 0;
					for(j = 0; j < v->size; j++)
						c1->v[0][i] += v->v[j][i];
					c1->v[0][i] = c1->v[0][i] / v->size;
				}
				
				// Iterate through the specified number of splits, doubling each time
				for(i = 1; i <= nsplits; i++){
					// Split each codevector, slightly displacing it
					for(j = 0; j < c1->size; j++)
						for(k = 0; k < VECTOR_DIM; k++)
							c1->v[j + (1<<i)][k] = c1->v[j][k] + CODE_VECTOR_DISPLACE;
					
					d_new = DBL_MAX; // We set this high to reset recorded distortion
					// We set the size of the codevectors since we don't use all of them
					c1->size = 1<<i; // 1<<i = 2^i
					c2->size = 1<<i;
					// Update codevectors until chenge in distortion is sufficiently small
					do{
						d_old = d_new;
						d_new = centroid_update(v, &c1, &c2, counts);	
						// Remember, c1 points at the current codevectors after updating
						// c2 points at the codevectors from the previous iteration
						// counts is used internally in centroid update
					}while(d_old > (1 + LBG_EPS) * d_new);
				}
				free(c2);
				free(counts);
				return c1;
			}
			free(c2);
		}
		free(c1);
	}
	return NULL;
}

