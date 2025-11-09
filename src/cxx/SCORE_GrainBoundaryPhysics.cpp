// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __SCORE_KERNEL_H_INCLUDED__
//#define __SCORE_KERNEL_H_INCLUDED__

#include "SCORE_Kernel.h"

double caHdl::calc_mobilityweight ( uint32_t rgpoolid , uint32_t dgpoolid )
{
	if ( this->mobilitymodel == MOBILITYMODEL_ROLLETTHOLM ) {
		return ( this->calc_mobilityweight_rollettholm( rgpoolid, dgpoolid ) );
	}
	//else
	return ( this->calc_mobilityweight_sebaldgottstein( rgpoolid, dgpoolid ) );
}


double caHdl::calc_mobilityweight_sebaldgottstein ( uint32_t rgpid , uint32_t dgpid )
{
	//Sebald Gottstein model
	double q1[4], q2[4], theta;

	uint32_t rgori = myrxgpool[rgpid].caori;
	uint32_t dgori = mydefgpool[dgpid].caori;

	//now calculate on the fly...
	q1[0] = myoripool[rgori].q0;
	q1[1] = myoripool[rgori].q1;
	q1[2] = myoripool[rgori].q2;
	q1[3] = myoripool[rgori].q3;

	q2[0] = myoripool[dgori].q0;
	q2[1] = myoripool[dgori].q1;
	q2[2] = myoripool[dgori].q2;
	q2[3] = myoripool[dgori].q3;

	//P=weightedMob for all HAGB already defined
	double weightedMob = 0.0;

	theta = localmath.disori_angle_oriquat_cubic( q1, q2 );
	//assign categorical intrinsic boundary mobility to disorientation among two orientation
	//LAGB detected, overwrite default value
	if( theta <= MAXDISORI_LAGB2HAGB ) {
		weightedMob = -1.0;

		return weightedMob;
	}

	//obviously a HAGB but also eligible for GS? check proximity to 40deg111

	double qdis[4] = {1.0, 0.0, 0.0, 0.0};
	localmath.disori_fromoriquat_cubic_imm( q1, q2, qdis );
	//the quaternion that describes a 40deg111 misorientation
	double maxDev40_111 = MAXDISORI_TO_40DEG111;
	double _sqrt3 = 1 / sqrt( 3.0 );
	double oneNinth = 1.0 / 9.0;
	double m40_111[4] = { cos( oneNinth * _PI_ ), _sqrt3 * sin( oneNinth * _PI_ ), _sqrt3 * sin( oneNinth * _PI_ ), _sqrt3 * sin( oneNinth * _PI_ ) };

	double dev_40_111 = localmath.disori_angle_oriquat_cubic( qdis, m40_111 );

	if( dev_40_111 < maxDev40_111 ) {
		weightedMob = SQR( cos( 0.5 * _PI_ * dev_40_111 / maxDev40_111 ) );
	}

	return weightedMob;
}


double caHdl::calc_mobilityweight_rollettholm ( uint32_t rgpid, uint32_t dgpid )
{
	double q1[4], q2[4], theta;

	uint32_t rgori = myrxgpool[rgpid].caori;
	uint32_t dgori = mydefgpool[dgpid].caori;

	//now calculate on the fly...
	q1[0] = myoripool[rgori].q0;
	q1[1] = myoripool[rgori].q1;
	q1[2] = myoripool[rgori].q2;
	q1[3] = myoripool[rgori].q3;

	q2[0] = myoripool[dgori].q0;
	q2[1] = myoripool[dgori].q1;
	q2[2] = myoripool[dgori].q2;
	q2[3] = myoripool[dgori].q3;

	theta = localmath.disori_angle_oriquat_cubic( q1, q2 );

	//P value allows to calculate the mobility as often occurring in works by Rollett and Holm via P * mRHHAGB
	double P = 1.0 - (myPhysData.RH_LAGBHAGBcut * exp( -1.0 * myPhysData.RH_LAGBHAGBtrans * pow( (theta/MAXDISORI_LAGB2HAGB), myPhysData.RH_LAGBHAGBexponent ) ));

	return P;
}

#endif
