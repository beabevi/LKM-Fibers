#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>
#include "numerical.h"

static __thread seed_type master_seed;

// A Uniform random number generator
double Random(void) {
	uint32_t *seed1;
	uint32_t *seed2;
	
	if(master_seed == 0) {
		master_seed = random();
	}

	seed1 = (uint32_t *)&master_seed;
	seed2 = (uint32_t *)((char *)&master_seed + (sizeof(uint32_t)));

	*seed1 = 36969u * (*seed1 & 0xFFFFu) + (*seed1 >> 16u);
	*seed2 = 18000u * (*seed2 & 0xFFFFu) + (*seed2 >> 16u);

	// The result is strictly between 0 and 1.
	return (((*seed1 << 16u) + (*seed1 >> 16u) + *seed2) + 1.0) * 2.328306435454494e-10;
}

// An exponential random number generator
double Expent(double mean) {
	if(mean < 0) {
		fprintf(stderr, "Expent() has been passed a negative mean value\n");
		exit(EXIT_FAILURE);
	}

	return -mean * log(1 - Random());
}
