// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __SCORE_KERNEL_H_INCLUDED__
//#define __SCORE_KERNEL_H_INCLUDED__

#include "SCORE_Kernel.h"

void caHdl::ompshar_sim_myCA_sitesaturatedNucleation( vector<rxgcommit>* sync )
{
	//this function expects the mycellgrid positions to be marked with mydefgpool ids not tmpdefg seedids!
	//CALLED FROM WITHIN PARALLEL REGIONS BECAUSE OF THAT JUVENILE NUCLEATION INTO MYCELLGRID
	//TAKE CARE THAT CONCURRENT WRITES ARE DISALLOWED
	uint32_t nucleateWhere, nucleateWhereRegion;
	uint32_t card = myCAGeometry.nboxedge_rd;
	uint32_t cardtd = myCAGeometry.nboxarea_rdtd;
	uint32_t z, rem, y, x;

	uint32_t tid = omp_get_thread_num();

	uint32_t xmi = regions[tid]->myGeom.nreg_rdmin;
	uint32_t xmx = regions[tid]->myGeom.nreg_rdmax;
	uint32_t ymi = regions[tid]->myGeom.nreg_tdmin;
	uint32_t ymx = regions[tid]->myGeom.nreg_tdmax;
	uint32_t zmi = regions[tid]->myGeom.nreg_ndmin;
	uint32_t zmx = regions[tid]->myGeom.nreg_ndmax;
	uint32_t xx = regions[tid]->myGeom.nreg_rd;
	uint32_t xxyy = regions[tid]->myGeom.nregarea_rdtd;
	bool toggle = false;

	//scan and place cooperatively the nuclei
	//MK::be aware that it is necessary to distinguish whether to place the nucleus in the inside or the border!

	for ( uint32_t nuc = 0; nuc < myrxgpool.size(); nuc++) {
		nucleateWhere = myrxgpool[nuc].nucsite; //MK::only thread-safe if JuvenileNucleation does not modify myrxgpool

		//scan all coordinates
		z = nucleateWhere / cardtd;
		rem = nucleateWhere - (cardtd * z);
		y = rem / card;
		x = rem - (card * y);

		//check who is with me, mycellgrid of all regions form a nonoverlapping exactly the domain
		//HERE THE THREADS FILTER OUT ONLY THOSE NUCLEI THAT ARE LOCATED IN THEIR OWN CELLGRID
		//AS THE CELLGRIDS ARE NONOVERLAPPING AND FILLING COMPLETELY THE DOMAIN ALL THREADS EXECUTE
		//IN PARALLEL THE SAME CODE BUT NEVER INTERFERE...

		if ( x < xmi ) continue; //for performance utilize that most likely the nucleus is outside my own region
		if ( x > xmx ) continue;
		if ( y < ymi ) continue;
		if ( y > ymx ) continue;
		if ( z < zmi ) continue;
		if ( z > zmx ) continue;

		//inside!, interpret global coordinate from CA domain into local coordinate in my CAregion
		nucleateWhereRegion = ((z - zmi) * xxyy) + ((y - ymi) * xx) + (x - xmi);

		//inside or outer shell? most likely inside
		toggle = false; //assume most likely case, nucleus is in the shell
		if ( x > xmi && x < xmx && y > ymi && y < ymx && z > zmi && z < zmx ) {
			toggle = true;
		}

		if ( ompshar_sim_myCA_infect_JuvenileNucleation( nuc, x, y, z, nucleateWhereRegion, toggle, ALMOST_FULLY_INFECTED ) == true ) {
			struct rxgcommit success;
			success.nrxgid = nuc;
			success.nucsite = SITE_ALREADY_CONSUMED; //nucleateWhere;
			success.tincub = t;
			sync->push_back( success);
		}
	}

	if ( myensHdl->myRank == MASTER ) {
		cout << "\t\tmyRank " << myensHdl->myRank << "; ThreadID " << tid << "; JobID " << this->jobid << " mydefgpool.size() " << mydefgpool.size() << " myrxgpool.size() " << myrxgpool.size() << " nucleate site-saturated." << endl;
	}
}


bool caHdl::ompshar_sim_myCA_infect_JuvenileNucleation( uint32_t rxgpoolid, uint32_t xglo, uint32_t yglo, uint32_t zglo, uint32_t infectWhereRegion, bool inside, double rxfrac0 )
{
	//CALLED FROM WITHIN PARALLEL REGION!
	//works because the calling thread is only assigned one unique region
	caregionMemHdlP me = this->regions[omp_get_thread_num()];

	//get out here, cell is already infected, but in case of site-saturated nucleation no cell can be infected already
	if ( (me->mycellgrid[infectWhereRegion] >= me->reg_nmydefgpool) || (me->mycellgrid[infectWhereRegion] == CURRENTLY_INFECTED) ) {
		//##MK::synchronization like the following two lines violates thread-safety!
		//##MK::myrxgpool[rxgpoolid].nucsite = SITE_ALREADY_CONSUMED;	//not critical as no thread accesses the same elements of myrxg than another do never overlap! but false sharing... flush not necessary!
		//##MK::myrxgpool[rxgpoolid].tincub = NUCLEUS_NOT_PLACABLE; //MK::startingsite varible must not be changed!
		return false;
	}

	//obviously cell is free, hehe, go for it!
	uint32_t mydgid = me->mycellgrid[infectWhereRegion];
	//nucleation sites disjoint, mark cell as infected to prevent multiple reinfections, do right here to localize memory access
	me->mycellgrid[infectWhereRegion] = CURRENTLY_INFECTED;

	double PP = calc_mobilityweight( rxgpoolid, mydgid );

	uint32_t freeplace = UINT32T_MAX;
	if ( inside == true ) {
		freeplace = me->omp_getNextFreeSlotInRXFrontInside( CELLAPPEND );
		QUICKASSERT ( freeplace != INVALID_ADDRESS);

		me->myRXFrontInside[freeplace].activity = ACTIVE;
		me->myRXFrontInside[freeplace].infector = 26;
		me->myRXFrontInside[freeplace].ix = (short) xglo; //explicit cast to lower precision short allowed because 0 < iglo <= CA_MAXIMUM_DIMENSIONS << SHORT_RANGE
		me->myRXFrontInside[freeplace].iy = (short) yglo;
		me->myRXFrontInside[freeplace].iz = (short) zglo;
		me->myRXFrontInside[freeplace].rxFrac = rxfrac0;
		me->myRXFrontInside[freeplace].mydefgid = mydgid;	//cell carries consumed orientation along
		me->myRXFrontInside[freeplace].myrxgid = rxgpoolid;
		me->myRXFrontInside[freeplace].P = PP;
	}
	else {
		freeplace = me->omp_getNextFreeSlotInRXFrontBorder( CELLAPPEND );
		QUICKASSERT (freeplace != INVALID_ADDRESS);

		me->myRXFrontBorder[freeplace].activity = ACTIVE;
		me->myRXFrontBorder[freeplace].infector = 26;
		me->myRXFrontBorder[freeplace].ix = (short) xglo;
		me->myRXFrontBorder[freeplace].iy = (short) yglo;
		me->myRXFrontBorder[freeplace].iz = (short) zglo;
		me->myRXFrontBorder[freeplace].rxFrac = rxfrac0;
		me->myRXFrontBorder[freeplace].mydefgid = mydgid;	//cell carries consumed orientation along
		me->myRXFrontBorder[freeplace].myrxgid = rxgpoolid;
		me->myRXFrontBorder[freeplace].P = PP;
	}

	if ( PP >= me->reg_myMobilityWeightMax ) me->reg_myMobilityWeightMax = PP;
	me->reg_nmynuclei++;


	//MK::get_NextFreeSlotInRXFront() ASSURES freeplace <= nextSlotNeverActiveRXFront < ntotalRXFront!

//#pragma omp critical{cout << "Thr/Nuc = " << omp_get_thread_num() << "\t\t" << rxgpoolid << endl;}
#ifdef REPORTSTYLE_CELLCYCLES
	cout << this->jobid << "\t\tjuvenile infection;rxgpoolid;WhichCelltoInfect;x;y;z;rxfrac0;freeplace" << rxgpoolid << ";" << WhichCelltoInfect << ";" << x << ";" << y << ";" << z << ";" << rxfrac0 << ";" << freeplace << endl;
#endif
	return true;
}


#endif