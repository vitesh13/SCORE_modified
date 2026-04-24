// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __SCORE_RANDOM_H_INCLUDED__
#define __SCORE_RANDOM_H_INCLUDED__

#include <math.h>
#include <stdint.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <numeric>

//Mersenne Twister Parameters
#define NMT			624
#define MMT			397
#define MATRIX_A	0x9908b0df
#define UPPER_MASK	0x80000000
#define LOWER_MASK	0x7fffffff

#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

#define RANDMAX (1.0 - DBL_EPSILON)

#define DEFAULT_SEED	-46356


using namespace std;


class randomClass
{
public:
	randomClass(){
		mt = NULL;
	}
	~randomClass(){
		delete [] mt;
	};

	void init(long sd);
	double MersenneTwister();
	void sgenrand();
	void warmupMT( unsigned int n ) {
		double toss = 0.0;
		for ( unsigned int r = 0; r < n; r++ ) { toss = this->MersenneTwister(); }
	};

private:
	uint32_t seedMT;	//state set for the MersenneTwister
	uint32_t* mt;
	uint32_t mti;
	uint32_t mag01[2] = {0x0, MATRIX_A};
};
typedef randomClass *randomClassP;


#endif
