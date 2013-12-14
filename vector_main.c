#include "vq.h"

int main(int argc, char *argv[]){
	int i, j;
	int numtrain = 5;
	vectorset *train, *c;

	if((train = init_vectorset(numtrain))){
		for(i = 0; i < numtrain; i++)
			for(j = 0; j < VECTOR_DIM; j++)
				train->v[i][j] = i*j;
		print_vectorset(train);
		c = lbgvq(train, 2);
		print_vectorset(c);
	}
	else
		printf("Could not generate training data.\n");	
}
