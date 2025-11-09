// SPDX-License-Identifier: GNU General Public License v3.0 or later

#ifndef __SCORE_KERNEL_H_INCLUDED__
//#define __SCORE_KERNEL_H_INCLUDED__

#include "SCORE_Kernel.h"

double caHdl::kam2rho( double kam )
{
	//physical model to interpret kernel average misorientation (KAM) into an approximate continuum-scale dislocation density for the deformed grain
	//see Godfrey, A., Mishin, O., Yu, T., 2015 doi:10.1088/1757-899X/89/1/012003
	//Method D ( (3.0*G*b / (2.0*Delta)) * kam ); //Es in Pa

	//handling of temperature dependency in G and b..., this works only for isothermal annealing because the time = 0.0 and thus the first temperature from the annealing profile is taken which may be roomtemperature!
	//a given population of dislocations has a different internal elastic strain energy as G and b are temperature-dependent
	double b = this->get_burgersvector( 273.15+20.0 );		//Burgers vector
	double Delta = myensHdl->ebsdstepsize;					//stepsize in meter

	return ( (3.0 /(b*Delta))*kam ); //Es/(0.5Gbb) //1/m^2
	//KAM needs to be defined in radiant
}

double caHdl::get_shearmodulus( double T )
{
	//temperature dependent shear modulus if desired
	//see Nadal, M-H and Le Poac, P in J. of Appl. Phys. 93 2472 2003 for further details what to do close to the melting point
	//MK::for Alu
	//double CurrentG = myPhysData.G0 - (myPhysData.dGdt * ( T / myPhysData.Tmelt ));

	//overwritten for iron in accordance with Ledbetter, Reed, 1973
	//MK::for bcc-iron, thesis MK
	//return ( myPhysData.G0 * (1.0 - ((9e-10 * CUBE(T) - 7e-7*SQR(T) + 0.0003*T - 0.00028))) );

	//MK::for bcc-iron, Kuehbach, Diehl
	return myPhysData.G0 * ( -1.38e-9 * CUBE(T) + 1.21e-6 * SQR(T) - 4.58e-4 * T + 1.00 );
}


double caHdl::get_burgersvector( double T )
{
	//temperature dependent Burgers vector as a result of lattice elongation/contraction
	//double T = CurrentTemperature - TOFFSET;
	//MK::for Alu
	//double CurrentBurgersVector = myPhysData.bZeroCelsius * ( 1.0 + myPhysData.thermexp_C * ((myPhysData.thermexp_a * T) + ( myPhysData.thermexp_b * SQR(T) )) * 1e-6 );

	//MK::for bcc iron up to 800deg celsius
	return myPhysData.bZeroCelsius;
}


double caHdl::calculateBoundaryDisoriFast( uint_fast32_t seedup, uint_fast32_t seeddown )
{
	double q1[4], q2[4], theta;
	uint32_t upOri = mydefgpool[tmpdefgseeds[seedup].mydefgpoolid].caori;
	uint32_t downOri = mydefgpool[tmpdefgseeds[seeddown].mydefgpoolid].caori;

	//now calculate on the fly...
	q1[0] = myoripool[upOri].q0;
	q1[1] = myoripool[upOri].q1;
	q1[2] = myoripool[upOri].q2;
	q1[3] = myoripool[upOri].q3;

	q2[0] = myoripool[downOri].q0;
	q2[1] = myoripool[downOri].q1;
	q2[2] = myoripool[downOri].q2;
	q2[3] = myoripool[downOri].q3;

	theta = localmath.disori_angle_oriquat_cubic( q1, q2 );

	return theta;
}

#endif