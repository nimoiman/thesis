#include "vq.h"
#include "channel.h"

int main(int argc, char *argv[]) {
	int i, j;
	int numtrain = 5;
	vectorset *train, *c;

	train = init_vectorset(numtrain); // allocate training vector set

	if (train) {
		/* Initialize training set to Gaussian(0,10) distribution */
		for (i = 0; i < numtrain; i++) {
			gaussian_channel(train->v[i], VECTOR_DIM, 0, 10);
		}
		/* Pass vectorset through Gaussian(0,1) noise */
		for (i = 0; i < numtrain; i++) {
			gaussian_channel(train->v[i], VECTOR_DIM, 0, 1);
		}

		printf("Training set:\n");
		print_vectorset(train);

		/* Generate codebook from training set */
		c = lbgvq(train, 2);
		printf("Generated Codebook:\n");
		print_vectorset(c);

	}
	else {
		printf("Could not generate training data.\n");
	}
}
