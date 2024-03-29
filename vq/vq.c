#include "vq.h"

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

// Updates the centroids in a single iteration of the LBG-VQ scheme
// This function re-adjusts the codevectors based on the centroid condition
// This function does not do any splitting
// A training set should be provided
// **c_old should initially point to the current codebook
// **c_new should point to a vectorset the same size as c1
// *count should point to an integer array the same size as the codebook
// *count is used internally
// After computing the updating centroids, the new codebook is placed under *c_new
// The average distortion between the training set and the new codebook is returned
double centroid_update(vectorset *train, vectorset *c_old, vectorset *c_new, int *count){
	int i, j, nn;
	vectorset *tmp;
	double d_avg = 0;

	// Initialize counts and new codebook to 0
	memset(count, 0, c_new->size * sizeof(int));
	memset(c_new->v, 0, c_new->size * VECTOR_DIM * sizeof(float));

	// For each training vector, we find the codevector of minimal distortion
	// i.e. the codevector for that training vector's encoding region
	// We want to place the new codevector at the mean of the training vectors that map to it
	// That is sum of all code vectors / number of codevectors
	for(i = 0; i < train->size; i++){
		d_avg += nearest_neighbour( train->v[i], c_old, &nn);
		count[nn]++;
		for(j = 0; j < VECTOR_DIM; j++)
			c_new->v[nn][j] += train->v[i][j];
	}

	// Here we divide the new codevectors by the number of mapped vectors to get the mean
	for(i = 0; i < c_new->size; i++){
		if(count[i] > 0)
			for(j = 0; j < VECTOR_DIM; j++)
				c_new->v[i][j] /= count[i];
		//Note: if count[i] = 0, (no mapping training vectors), then codevector v[i] maps to origin
	}

	// Get average distortion by dividing by the total training set size	
	d_avg /= train->size;
	return d_avg;
}

// Performs the LBG VQ scheme on a given training set
// The training set should be provided, along with the number of times the codebook should split
// Recall that the total codebook size is therefore 2^(nsplits)
// A pointer to the new codebook is returned
vectorset *lbgvq(vectorset *train, int nsplits){
	vectorset *c_old, *c_new, *tmp;
	int i, j, k, max_cv, *counts;
	double d_old, d_new;

	max_cv = 1<<nsplits; // Final codebook size (2^nsplits)

	if((c_old = init_vectorset(max_cv))){
		if((c_new = init_vectorset(max_cv))){
			counts = (int*) malloc(max_cv*sizeof(int));
			if(counts){
				// Everything initialized properly
				//We first set the initial codevector at the centroid of the training data
				c_new->size = 1;
				for(i = 0; i < VECTOR_DIM; i++){
					c_new->v[0][i] = 0;
					for(j = 0; j < train->size; j++)
						c_old->v[0][i] += train->v[j][i];
					c_new->v[0][i] = c_old->v[0][i] / train->size;
				}
				
				// Iterate through the specified number of splits, doubling each time
				for(i = 1; i <= nsplits; i++){
					// Split each codevector, slightly displacing it
					for(j = 0; j < c_new->size; j++)
						for(k = 0; k < VECTOR_DIM; k++)
							c_new->v[j + c_new->size][k] = c_new->v[j][k] + CODE_VECTOR_DISPLACE;

					// We set the size of the codevectors since we don't use all of them
					c_old->size = 1<<i; // 1<<i = 2^i
					c_new->size = 1<<i;

					d_new = DBL_MAX; // We set this high to reset recorded distortion
					// Update codevectors until chenge in distortion is sufficiently small
					do{
						d_old = d_new;

						// Set old codebook to last codebook
						// Old codebook from last iteration holds new codebook
						tmp = c_old;
						c_old = c_new;
						c_new = tmp;

						d_new = centroid_update(train, c_old, c_new, counts);	
						// Remember, c1 points at the current codevectors after updating
						// c_new points at the codevectors from the previous iteration
						// counts is used internally in centroid update
					}while(d_old > (1 + LBG_EPS) * d_new);
				}
				destroy_vectorset(c_new);
				free(counts);
				return c_old;
			}
			destroy_vectorset(c_new);
		}
		destroy_vectorset(c_old);
	}
	return NULL;
}

