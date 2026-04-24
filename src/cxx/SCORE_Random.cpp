// SPDX-License-Identifier: GPL-3.0-or-later

//PRNG as detailed in Numerical Recipes in C 2nd version and implemented by L. A. Barrales-Mora, some explicit 32-bit datatypes where necessary
//credit to the authors of the additional PRNGs is given in the appropriate code sections

#include "SCORE_Random.h"

void randomClass::init( long sd )
{
	//initialize MersenneTwister
	seedMT = (uint32_t) sd;
	mt = (uint32_t*) new uint32_t[NMT];
	mti = NMT+1;
	sgenrand();
	warmupMT( 500000 );
}


void randomClass::sgenrand()
{
	//Initialization of Mersenne twister
	mt[0] = seedMT & 0xffffffff;
	for( mti=1; mti < NMT; mti++ )
		mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}


double randomClass::MersenneTwister()
{
/* Saito M, Matsumoto M. SIMD-oriented fast Mersenne twister: a 128-bit
   pseudorandom number generator, Monte-Carlo and Quasi-Monte Carlo Methods 2006; Springer 2008, 2:607
   DOI:10.1007/978-3-540-74496-2_36*/

//Original algorithm interval [0,1]; here modified for [0,1)

	uint32_t y;

	if( mti >= NMT )
	{
		int kk;

		if( mti == NMT+1 )
		{
			seedMT = (uint32_t) time(NULL);
			sgenrand();
		}

		for( kk=0; kk<NMT-MMT; kk++ )
		{
			y = ( mt[kk] & UPPER_MASK ) | ( mt[kk+1] & LOWER_MASK );
			mt[kk] = mt[kk+MMT] ^ ( y >> 1 ) ^ mag01[y & 0x1];
		}

		for(;kk<NMT-1;kk++)
		{
			y = ( mt[kk] & UPPER_MASK ) | ( mt[kk+1] & LOWER_MASK );
			mt[kk] = mt[kk+(MMT-NMT)] ^ (y >> 1) ^ mag01[y & 0x1];
		}

		y = ( mt[NMT-1] & UPPER_MASK ) | ( mt[0] & LOWER_MASK );
		mt[NMT-1] = mt[MMT-1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= TEMPERING_SHIFT_U(y);
	y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
	y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
	y ^= TEMPERING_SHIFT_L(y);

	double randr = ( (double) y / (uint32_t)0xffffffff );

	if( randr > RANDMAX )	return RANDMAX;

	return randr;
}
