#include "vq.h"

int main(int argc, char *argv[]){
	int numtrain = 100;
	vectorset train, *c;

	if(init_vectorset(&train, numtrain)){
		c = lbgvq(&train, 3);
		print_vectorset(c);
	}
	else
		printf("Could not generate training data.\n");	
}
