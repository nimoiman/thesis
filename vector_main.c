#include <stdio.h>
#include "vq.h"
#include "channel.h"

int main(int argc, char *argv[]) {
	int i, j, idx;
	int numtrain = 500;
	int nsplits = 4;
	vectorset *train, *c;
	FILE *fp;

	if(argc != 3){
		fprintf(stderr, "Need to specify 2 inputs: training set and codebook file\n");
		exit(1);
	}
	
	train = init_vectorset(numtrain); // allocate training vector set

	if (train) {
		/* Reset random seed */
		srand(1234);
		/* Initialize training set to Gaussian(0,10) distribution */
		for (i = 0; i < numtrain; i++) {
			gaussian_channel(train->v[i], VECTOR_DIM, 0, 10);
		}
		/* Pass vectorset through Gaussian(0,1) noise */
		for (i = 0; i < numtrain; i++) {
			gaussian_channel(train->v[i], VECTOR_DIM, 0, 1);
		}

		/* Generate codebook from training set */
		
		c = lbgvq(train, nsplits);
		
		fp = fopen(argv[1], "w");
		if(fp == NULL){
			fprintf(stderr, "Unable to open traing set file %s\n", argv[1]);
			exit(1);
		}

		for(j = 0; j < VECTOR_DIM; j++)
			fprintf(fp, "v%d\t", j+1);
		fprintf(fp, "map\n");
		for(i = 0; i < train->size; i++){
			for(j = 0; j < VECTOR_DIM; j++)
				fprintf(fp, "%f\t", train->v[i][j]);
			nearest_neighbour(train->v[i], c, &idx);
			fprintf(fp, "%d\n", idx);
		}
		fclose(fp);

		fp = fopen(argv[2], "w");
		if(fp == NULL){
			fprintf(stderr, "Unable to open codebook file %s\n", argv[2]);
			exit(1);
		}

		print_vectorset(fp, c);
		fclose(fp);
		
		destroy_vectorset(c);
		destroy_vectorset(train);

	}
	else {
		printf("Could not generate training data.\n");
	}
}
