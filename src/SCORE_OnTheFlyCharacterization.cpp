/* 
*	Copyright Markus Kühbach, 2014-2017
*	L. A. Barrales-Mora (quaternion library) and V. Mohles (I/O routines for reading the UDS file format)
*	contributed to the code.

*	SCORE is an MPI/OpenMP-parallel implementation of a cellular automaton model for the studying 
*	of microstructure evolution during the growth phase of static recrystallization in metallic alloys.
*	Its novelity is to solve an ensemble of independent simulation domains and to average their results
*	into an ensemble result. In comparison to the classical RVE-based approach this strategy enables 
*	studies with much higher statistical significance as orders of magnitude more grains can be studied
*	while these are solved at the same time in independent individual simulations which are thus executable
*	in parallel.
*	For this task, SCORE utilizes a two-layer data parallelism with a main layer of MPI-processes. 
*	Each of which solves for a queue of cellular automata domains. A second layer of OpenMP-thread 
*	parallelism accelerates the executing of each individual CA domain. The method is described in:

*	M. Kühbach, G. Gottstein, L. A. Barrales-Mora: A statistical ensemble cellular automaton 
*	microstructure model for primary recrystallization, Acta Materialia, Vol 107, 2016, p366
*	http://dx.doi.org/10.1016/j.actamat.2016.01.068

*	Further details, in particular to this implementation and the concept, are detailed in:
*	M. Kühbach: Efficient Recrystallization Microstructure Modeling by Utilizing Parallel Computation

*	The authors gratefully acknowledge the financial support from the Deutsche Forschungsgemeinschaft
*	(DFG) within the Reinhart Koselleck-Project (GO 335/44-1) and computing time grants kindly provided
*	by RWTH Aachen University and the FZ Jülich within the scope of the JARAHPC project JARA0076.


*	This file is part of SCORE.

*	SCORE is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.

*	SCORE is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.

*	You should have received a copy of the GNU General Public License
*	along with SCORE.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __SCORE_KERNEL_H_INCLUDED__
//#define __SCORE_KERNEL_H_INCLUDED__

#include "SCORE_Kernel.h"


void caHdl::log_initialization( void )
{
	//user defined recrystallized fraction of desired logpoints are inputted as normalized quantities, so multiply with boxdimensions
	double boxvolume = myCAGeometry.nboxvol_rdtdnd;

	for ( uint32_t d = 0; d < this->defragRXFront_atthisX.size(); ++d ) {
		defragRXFront_atthisX[d] = defragRXFront_atthisX[d] * boxvolume;
	}

	for ( uint32_t s = 0; s < this->output_atthisX.size(); ++s ) {
		output_atthisX[s] = output_atthisX[s] * boxvolume;
	}

	/* ##MK::we do no longer report these as counts but fraction thereby we can consistently scale
	 * into counts for area as well as volumina
	for ( uint32_t r = 0; r < this->rendering_atthisX.size(); ++r ) {
		rendering_atthisX[r] = rendering_atthisX[r] * boxvolume;
	}
	*/


	//initialize the color coding
	if ( outopt_localrenderhow == RENDERING_MS2D || outopt_localrenderhow == RENDERING_MS3D ) {
		if ( outopt_localrendercolor == RENDERING_COLOR_IPFZ )
			colorize_myoripool_ipfz();
	}


	if ( this->outopt_localgenDAMASKgeom == OUTPUT_DAMASK_GEOMETRYFILE_YES ) {
		//##MK::check status of this function...
		//write_damask_geom_h5( "NetworkInitialState" );
	}

	//when desiring percolation analysis create a 
	if ( percolation == PERCOLATION_ANALYZE_YES_SZDISTR ) {
		write_clustersizedistr_h5_create();
	}


	//implement intelligent precaching of logcontainers rxfrontstats and grainevo
}


double caHdl::log_rxareafraction( double zpos )
{
	//monitor how much area fraction is recrystallized in zpos, used to control output in Martin/Markus MMM2018 model
	//shared memory every thread processes it and will get the same result
	uint32_t imgx = myCAGeometry.nboxedge_rd;
	uint32_t imgy = myCAGeometry.nboxedge_td;
	uint32_t imgxy = imgx*imgy;
	uint32_t imgz = (uint32_t) ( zpos * ((double) myCAGeometry.nboxedge_nd));
	if ( imgz == myCAGeometry.nboxedge_nd ) //MK::global coordinates [0,nboxedge_i) along each direction!
		imgz--;

	//in which regions are the image data located? get region topology
	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //1 required, no domain split in x
	uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	//if ( npx != 1 ) { cout << "ERROR::Not outputting sections because npx != 1!" << endl; }

	//find the zlayer by scanning along the first npz region limits
	uint32_t zregion = 0;
	uint32_t zmi, zmx, zz;
	bool found = false;
	for ( zregion = 0; zregion < npz; zregion++ ) {
		zmi = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin; //global coordinates
		zmx = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax;

		if ( imgz >= zmi && imgz <= zmx ) {
			zz = imgz - zmi; //local grid coordinate z for all regions that include the z position imgz adnd participate in I/O
			found = true;
			break;
		}
	}

	uint32_t rx = 0;
	uint32_t dg = 0;

	if ( found == true ) {
		//the automaton comprises npz regions along z, x is aligned, and y as well, so simply read x*y slab stack in npy direction and drop such a global z section into the image
		uint32_t xper, yper;
		uint32_t r,region_corr, su_corr, gridvalue, oid;
		uint32_t ndg = this->mydefgpool.size();
		QUICKASSERT ( ndg == this->nmydefgpool );

		uint32_t xmi,xmx,ymi,ymx;
		for( uint32_t yregion = 0; yregion < npy; yregion++ ) { //collect data over regions
			r = (zregion*npy)+yregion;
			xper = this->regions[r]->myGeom.nreg_rd; //MK::extent in the r-th region not global SU coordinate
			yper = this->regions[r]->myGeom.nreg_td;
			xmi = this->regions[r]->myGeom.nreg_rdmin; //global coordinates delimitng memory region inclusively
			xmx = this->regions[r]->myGeom.nreg_rdmax;
			ymi = this->regions[r]->myGeom.nreg_tdmin;
			ymx = this->regions[r]->myGeom.nreg_tdmax;

			for ( uint32_t yy = 0; yy < yper; yy++ ) {
				uint32_t regyzoff = (xper * yy) + (xper * yper * zz);
				uint32_t yzoff = (ymi + yy)*imgx;
				for ( uint32_t xx = 0; xx < xper; xx++ ) {
					region_corr = xx + regyzoff;
					su_corr = xmi + xx + yzoff;

					gridvalue = regions[r]->mycellgrid[region_corr];

					if ( gridvalue <= CA_GRAINIDS_MAXIMUM ) { //therefore also != CURENTLY_INFECTED most likely either rx or defg
						if ( gridvalue >= ndg )
							rx++;
						else
							dg++;
					}
				} //scan through xline in region r
			} //scan +y the xlines stacked in region r

			//fill in remaining values inside this region, the currently infected cells whose status in the global grid is INFECTED
			uint32_t ncandidates = this->cellStatii.at(r)->size();
			vector<cellstatus>* candidates = this->cellStatii.at(r);
			for ( uint32_t cand = 0; cand < ncandidates; cand++ ) {
				//is the infected pixel in layer imgz?
				uint32_t zzz = candidates->at(cand).ixyz / (imgxy);
				if ( zzz != imgz ) { //most likely we're not...
					continue;
				}
				//in this plane, zzz == imgz, i.e. we are in the correct layer and require an orientation for a cell in state == CURRENTLY_INFECTED
				uint32_t rem = candidates->at(cand).ixyz - (imgxy * zzz);
				uint32_t yyy = rem / imgx;
				uint32_t xxx = rem - (imgx * yyy); //x,y,z are SU global coordinates and thus translate into PNG image coordinates
				uint32_t px = xxx + (yyy*imgx);

				gridvalue = candidates->at(cand).gid;
				if ( gridvalue >= ndg )
					rx++;
				else
					dg++;

			} //done checking all candidates of the region for contribution CURRENTLY_INFECTED pixel to the image
		} //next region stacked upon last one in +y
		//done with all regions along y who include the zsection at global SU coordinate imgz
	}

	uint32_t rxdg = rx + dg;
	if ( rxdg > 0 )
		return static_cast<double>(rx) / static_cast<double>(rxdg);
	else
		return 0.0;
}


void caHdl::log_rxareaprofile( void )
{
	//monitor how much area fraction is recrystallized in zpos, used to control output in Martin/Markus MMM2018 model
	//shared memory every thread processes it and will get the same result

	//store intermediate results to avoid writing individual files for every time step
	uint32_t imgz = myCAGeometry.nboxedge_nd;
	uint32_t imgx = myCAGeometry.nboxedge_rd;
	uint32_t imgy = myCAGeometry.nboxedge_td;
	uint32_t imgxy = imgx*imgy;

	uint32_t* rxdg = NULL;
	rxdg = new uint32_t[2*imgz];
	QUICKASSERT ( rxdg != NULL );
	for( uint32_t i = 0; i < 2*imgz; ++i ) { //reset content to zero
		rxdg[i] = 0;
	}

	uint32_t ndg = this->mydefgpool.size();
	uint32_t nthreads = this->regions.size();
	for ( uint32_t r = 0; r < nthreads; r++ ) {
		uint32_t rx = this->regions[r]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
		uint32_t ry = this->regions[r]->myGeom.nreg_td;
		uint32_t rz = this->regions[r]->myGeom.nreg_nd;
		uint32_t rxy = rx*ry;

		//copy dislocation densities from the buffer
		uint32_t* thegrid = this->regions[r]->mycellgrid;
		uint32_t c = 0;
		uint32_t zroff, yzroff,zwoff;
		uint32_t zmi = this->regions[r]->myGeom.nreg_ndmin;
		uint32_t zmx = this->regions[r]->myGeom.nreg_ndmax;
		for ( uint32_t z = 0; z < rz; z++ ) {
			zroff = z*rxy;
			zwoff = zmi + z;
			for ( uint32_t y = 0; y < ry; y++ ) {
				yzroff = y*rx + zroff;
				for ( uint32_t x = 0; x < rx; x++ ) {
					c = thegrid[x+yzroff]; //read cell index, recrystallized?
					if ( c <= CA_GRAINIDS_MAXIMUM ) { //so not CURRENTLY_INFECTED
						if ( c >= ndg )
							rxdg[2*zwoff+0]++; //rx
						else
							rxdg[2*zwoff+1]++; //dg
					}
				} //scan +x line
			} //scan +y line
		} //scan +z lines

		//fill in remaining values inside this region, the currently infected cells whose status in the global grid is INFECTED
		uint32_t ncandidates = this->cellStatii.at(r)->size();
		vector<cellstatus>* candidates = this->cellStatii.at(r);
		for ( uint32_t cand = 0; cand < ncandidates; cand++ ) {
			//in which z layer is the infected pixel?
			uint32_t zzz = candidates->at(cand).ixyz / (imgxy);
			uint32_t gridvalue = candidates->at(cand).gid;
			if ( gridvalue >= ndg )
				rxdg[2*zzz+0]++;
			else
				rxdg[2*zzz+1]++;
		} //done checking all candidates of the region for contribution CURRENTLY_INFECTED pixel to the image

	} //for each thread

	myrxprofile.push_back( loginfo_rxareaprofile_ca(t, X, step, rxdg) );
}

struct rxdg
{
	uint32_t rx;
	uint32_t dg;
	rxdg() : rx(0), dg(0) {}
	rxdg(const uint32_t _rx, const uint32_t _dg) : rx(_rx), dg(_dg) {}
};


void caHdl::log_rxareaprofile( uint32_t threshold )
{
	//monitor how much area fraction is recrystallized in zpos used to control output in Martin/Markus MMM2018 model
	//corrected for excluding all rx grains whose sectioned area  is smaller than threshold

	//store intermediate results to avoid writing individual files for every time step
	uint32_t imgz = myCAGeometry.nboxedge_nd;
	uint32_t imgx = myCAGeometry.nboxedge_rd;
	uint32_t imgy = myCAGeometry.nboxedge_td;
	uint32_t imgxy = imgx*imgy;

	//accounting for every z layer
	uint32_t* rxdg = NULL;
	rxdg = new uint32_t[2*imgz];
	QUICKASSERT ( rxdg != NULL );
	for( uint32_t i = 0; i < 2*imgz; ++i ) { //reset content to zero
		rxdg[i] = 0;
	}

	uint32_t ndg = this->mydefgpool.size();
	uint32_t nthreads = this->regions.size();

	for( uint32_t zz = 0; zz < imgz; zz++ ) { //for every RDTD section
		//count for every recrystallized grain we see with how many pixel it is sectioned on the slice
		map<uint32_t,uint32_t> rxg_pixel_count;

		for ( uint32_t r = 0; r < nthreads; r++ ) {
			uint32_t rx = this->regions[r]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
			uint32_t ry = this->regions[r]->myGeom.nreg_td;
			uint32_t rz = this->regions[r]->myGeom.nreg_nd;
			uint32_t rxy = rx*ry;

			//copy dislocation densities from the buffer
			uint32_t* thegrid = this->regions[r]->mycellgrid;
			uint32_t c = 0;
			uint32_t zroff, yzroff,zwoff;
			uint32_t zmi = this->regions[r]->myGeom.nreg_ndmin;
			uint32_t zmx = this->regions[r]->myGeom.nreg_ndmax;
			for ( uint32_t z = 0; z < rz; z++ ) {
				//for every z section we account now the sectioned size of the RX grains as well and filter for threshold
				zroff = z*rxy;
				zwoff = zmi + z;
				//is there at all a contribution from this z layer?
				if ( zwoff != zz ) { //most likely case
					continue;
				}
				else { //there is a contribution to this layer
					for ( uint32_t y = 0; y < ry; y++ ) {
						yzroff = y*rx + zroff;
						for ( uint32_t x = 0; x < rx; x++ ) {
							c = thegrid[x+yzroff]; //read cell index, recrystallized?
							if ( c <= CA_GRAINIDS_MAXIMUM ) { //so not CURRENTLY_INFECTED
								if ( c >= ndg ) {
									uint32_t rxgid = c - ndg + 1;
									auto it = rxg_pixel_count.find( rxgid );
									if ( it != rxg_pixel_count.end() ) { //likely value exists
										it->second++;
									}
									else { //if not add it
										rxg_pixel_count.insert( pair<uint32_t,uint32_t>(rxgid, 1) );
									}
								}
								//else nothing to do we do not care for accounting of deformed volume
							}
						} //scan +x line
					} //scan +y line
				} //done checking contribution from this layer
			} //scan +z lines, ##MK::further checks not required

			//add contribution from active cells
			uint32_t ncandidates = this->cellStatii.at(r)->size();
                        //std::cout << "Logging function cell statii exists" << this->cellStatii.size() << "\n" << std::endl;
                        //std::cout << "What is at cell statii" << this->cellStatii.at(0) << "\n" << std::endl;
                        //std::cout << "Can we find size of cell statii" << this->cellStatii.at(0)->size() << "\n" << std::endl;
			vector<cellstatus>* candidates = this->cellStatii.at(r);
			for ( uint32_t cand = 0; cand < ncandidates; cand++ ) {
				//in which z layer is the infected pixel?
				uint32_t gridvalue = candidates->at(cand).gid;
				uint32_t zzz = candidates->at(cand).ixyz / (imgxy);
				if ( zzz != zz ) { //most likely case you are not on the slice
					continue;
				}
				else {
					if ( gridvalue >= ndg ) {
						uint32_t rxgid = gridvalue - ndg + 1;
						auto it = rxg_pixel_count.find( rxgid );
						if ( it != rxg_pixel_count.end() ) {
							it->second++;
						}
						else
							rxg_pixel_count.insert( pair<uint32_t,uint32_t>(rxgid,1) );
					}
					//else { we also here dont care for incompletely rxed stuff }
				} //done accounting for an infected cell in the slice
			} //done checking all candidates of the region for contribution CURRENTLY_INFECTED pixel to the image
		} //next thread

		//now that we know for z section zz how many grains are rx and how large their intersection with the slice is
		//accounting for total rx and deformed is trivial
		for( auto jt = rxg_pixel_count.begin(); jt != rxg_pixel_count.end(); ++jt ) {
			if ( jt->second >= threshold ) {
				rxdg[2*zz+0] += jt->second;
			}
			//else dont care, other stuff is assumed still not sufficiently rxed so just account via difference
		}
		//deformed remainder is pixel count of slice minus rx
		rxdg[2*zz+1] = imgx*imgy - rxdg[2*zz+0];

		//clear the map because for the next slice the size distribution is likely different pixelwise
		rxg_pixel_count.clear();
	}

	myrxprofile.push_back( loginfo_rxareaprofile_ca(t, X, step, rxdg) );
}

/*
 * if ( gridvalue <= CA_GRAINIDS_MAXIMUM ) { //therefore also != CURENTLY_INFECTED most likely either rx or defg
		if ( gridvalue >= ndg ) { //account for pixel count of recrystallized grain area section
			uint32_t rxgid = gridvalue - ndg + 1;
			auto it = rxg_pixel_count.find( rxgid );
			if ( it != rxg_pixel_count.end() ) { //likely value exists
				it->second++;
			}
			else {
				rxg_pixel_count.insert( pair<uint32_t,uint32_t>(rxgid, 1) );
			}
		}
	}
 */






void caHdl::log_rxfrontstats( void )
{
	struct loginfo_rxfrontstats_ca rflog;

	rflog.localtime = this->t;
	rflog.localX = this->X;
	rflog.localmemory = this->myMemGuard;
	rflog.localPmax = this->myMobilityWeightMax;
	
	rflog.localstep = this->step;
	rflog.localSv = this->Sv;

	rflog.ntotalRXFront = DATA_NOT_LOGGED;
	rflog.nextSlotNeverActiveRXFront = DATA_NOT_LOGGED;
	rflog.nCurrentlyActive = DATA_NOT_LOGGED;
	rflog.ntotalFullRXList = DATA_NOT_LOGGED;
	rflog.nextSlotToFullRX = DATA_NOT_LOGGED;
	rflog.ntotalRecyclingList = DATA_NOT_LOGGED;
	rflog.nextSlotThatBecomesRecycled = DATA_NOT_LOGGED;
	rflog.firstNotRecycledYet = DATA_NOT_LOGGED;

	myrxfrontstatus.push_back ( rflog );
}


void caHdl::log_grainevo( void )
{
	//collect current microstructure state
	//MUST NOT BE CALLED BY MORE THAN ONE THREAD
	//##MK::further optimization potential by collecting only changes of the structure!

	uint32_t ndisjoint_grains = mydefgpool.size() + myrxgpool.size();

	//MK::in the old get_interp cell count there was a significant amount of time wasted with scanning mygrainevolution[i].localtime which is 40Byte just to find the internal
	//now this scanning is first performed on mygrainevolutionlocaltime and then only for the interesting data the cache misses are paid for
	mygrainevolutionlocaltime.push_back ( t );
	
	//temporary stack object invoking vector copy constructor of uninitialized grevo
	struct loginfo_grainevo_ca grevo;
	mygrainevolution.push_back ( grevo );

	uint32_t i = mygrainevolution.size() - 1;

	mygrainevolution[i].localtime = t;
	mygrainevolution[i].localX = X;
	mygrainevolution[i].locallogcnt = loginfo_grainevo_cnt;
	mygrainevolution[i].localtstep = step;
	mygrainevolution[i].localSv = Sv;
	mygrainevolution[i].nlocaldata = ndisjoint_grains;

	uint32_t* cellcount_bucket = NULL;
	cellcount_bucket = new uint32_t[ndisjoint_grains];
	QUICKASSERT ( cellcount_bucket != NULL );
	myMemGuard = myMemGuard + ( ndisjoint_grains * sizeof(uint32_t) );

	//log discrete volume of deformed grains
	uint32_t ndefg = mydefgpool.size();
	for ( uint32_t dg = 0; dg < ndefg; dg++ ) {
		cellcount_bucket[dg] = mydefgpool[dg].cellcount;

//cout << "DFG-LogOutput\t\t" << dg << "\t\t" << cellcount_bucket[dg] << endl;
	}

	//log discrete volume of rx grains
	for ( uint32_t rg = ndefg; rg < ndisjoint_grains; rg++ ) {
		cellcount_bucket[rg] = myrxgpool[rg-ndefg].cellcount;

//cout << "RXG-LogOutput\t\t" << rg << "\t\t" << cellcount_bucket[rg] << endl;
	}

	//here pointer to heap segment is carried over
	mygrainevolution[i].localdatabucket = cellcount_bucket;

#ifdef REPORTSTYLE_DEVELOPER
	cout << this->jobid << " logged grain evolution at step = " << this->step << " for the cnt = " << loginfo_grainevo_cnt << "-th time at desired X = " << UserDefLogPoint_X_Output[loginfo_grainevo_cnt] << endl;
#endif
	loginfo_grainevo_cnt++;
}


void caHdl::log_ca_physics( loginfo_ca_physicsP  container )
{
	container->nx = this->myCAGeometry.nboxedge_rd;
	container->ny = this->myCAGeometry.nboxedge_td;
	container->nz = this->myCAGeometry.nboxedge_nd;
	container->nxyz = this->myCAGeometry.nboxvol_rdtdnd;

	container->regx = this->regions[MASTER]->thePartitioning.nreg_rdx;
	container->regy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	container->regz = this->regions[MASTER]->thePartitioning.nreg_ndz;

	container->nboundarycells = this->SvDeformed;
	container->ndgrseeds = this->ndefgseeds;
	container->nrxgrseeds = this->nmyrxgpool;

	container->ndefmicrotexture = this->DefMicrotextureClasses;
	container->nnucleimicrotexture = this->NucleiMicrotextureClasses;
	container->defmicrotexture = this->DefMicrotexture;
	container->nucleimicrotexture = this->NucleiMicrotexture;
	container->storedenergy = this->StoredEnergy;
}


void caHdl::log_OUTPUT( void )
{
	//CALLED FROM WITHIN PARALLEL REGION BUT BY THE MASTER THREAD ONLY
	if ( this->outopt_localrxfront == OUTPUT_RXFRONTSTATS_YES ) {
		log_rxfrontstats();
	}

	//MK::NO PARALLEL RENDERING AT THE MOMENT
	//2D and 3D sections
	if ( renderingForThisCA == true ) {
		//##MK::old way of accounting was volume based i.e. Xcells / myCAGeometry.nboxvol_rdtdnd >= rendering_atthisX[loginfo_rendering_cnt] && loginfo_rendering_cnt < rendering_atthisX.size() ) {
		//##MK::Diehl/Kühbach way of accounting for MMM2018 January2019 study is that the inspection area needs sufficient RX fraction
		double Xarea_exp = log_rxareafraction( 0.0 );
		double Xarea_hlf = log_rxareafraction( 0.5 );

		double Xarea_use = Xarea_exp; //##MK::now fractions not counts! * myCAGeometry.nboxvol_rdtdnd;

		if ( omp_get_thread_num() == MASTER ) {
			cout << "Thread " << omp_get_thread_num() << " Xarea_exp = " << Xarea_exp << endl;
			cout << "Thread " << omp_get_thread_num() << " Xarea_hlf = " << Xarea_hlf << endl;
			cout << "Thread " << omp_get_thread_num() << " Xarea_use = " << Xarea_use << " loginfo_rendering_cnt " << loginfo_rendering_cnt << endl;
			cout << "Thread " << omp_get_thread_num() << " rendering[] = " << rendering_atthisX[loginfo_rendering_cnt] << " rendering_atthisX.size() " << rendering_atthisX.size() << endl;
		}

		if ( Xarea_use >= rendering_atthisX[loginfo_rendering_cnt] && loginfo_rendering_cnt < rendering_atthisX.size() ) {

			ompcrit_clarify_status_infectedcells( RXFRACTION_THRESHOLD );

			if ( outopt_localrenderhow == RENDERING_MS2D ) {
				if ( outopt_localrenderfileformat == RENDERING_FILEFORMAT_RAW ) {
					if ( outopt_localrendercolor == RENDERING_COLOR_IPFZ ) {
						ompcrit_write_zsection_looping_ipfz( Xarea_use, COLORIZE_RX_LEAVE_DEFORMED_BLACK );
						ompcrit_write_zsection_looping_ipfz( Xarea_use, COLORIZE_DEF_LEAVE_RX_BLACK );
						ompcrit_write_zsection_looping_ipfz( Xarea_use, COLORIZE_RX_RHOGREYSCALE_DEFORMED );
					}
					else
						cout << "ERROR::2D output in RAW format supports currently only IPFZ coloring!" << endl;
				}
				if ( outopt_localrenderfileformat == RENDERING_FILEFORMAT_HDF5 ) 
					cout << "ERROR::2D output with HDF5 currently not supported!" << endl;
			}

			if ( outopt_localrenderhow == RENDERING_MS3D ) {
				if ( outopt_localrenderfileformat == RENDERING_FILEFORMAT_RAW ) {
					if ( outopt_localrendercolor == RENDERING_COLOR_GRAINID )
						ompcrit_write_voxeldata_coloring_grainids( "GROWTH.3DGrainID" );
					else 
						cout << "ERROR::3D output in RAW format supports currently only grainid coloring!" << endl;
				}
				if ( outopt_localrenderfileformat == RENDERING_FILEFORMAT_HDF5 ) {
					if ( outopt_localrendercolor == RENDERING_COLOR_GRAINID )
						ompcrit_write_voxeldata_h5( RENDERING_COLOR_GRAINID, "/GROWTH3DGrainID", false, "dummy", this->t, this->X, this->step );
					else if ( outopt_localrendercolor == RENDERING_COLOR_IPFZ )
						ompcrit_write_voxeldata_h5( RENDERING_COLOR_IPFZ, "/GROWTH3DGrainID", false, "dummy", this->t, this->X, this->step );
					else if ( outopt_localrendercolor == RENDERING_COLOR_RHO )
						ompcrit_write_voxeldata_h5( RENDERING_COLOR_RHO, "/GROWTH3DGrainID", false, "dummy", this->t, this->X, this->step );
					else
						cout << "ERROR::3D output in HDF5 format supports currently only either grainid or IPFZ coloring!" << endl;
				}
			}

			if ( outopt_localartsemebsd == OUTPUT_SEMEBSD_YES ) {
				ompcrit_write_semebsd_looping( DEFAULT_SEMEBSD_MODE );
			}

			loginfo_rendering_cnt++;
		}
	}

	//single at the moment and data are stored not localized at the moment!
	//##MK::ideally time-resolved growth data of individual grains is spread on the threads to reduce overhead in get_interp_cellCount, not now
	if ( Xcells >= output_atthisX[loginfo_grainevo_cnt] && loginfo_grainevo_cnt < output_atthisX.size() ) {
		//##MK::if no solitary unit analyses are conducted this storing might not even be necessary, disable to save memory!
		log_grainevo();
		log_rxareaprofile();
	}
}


void caHdl::log_OUTPUT_Diehl()
{
	//CALLED FROM WITHIN PARALLEL REGION BUT BY THE MASTER THREAD ONLY
	ompcrit_clarify_status_infectedcells( RXFRACTION_THRESHOLD );

	if ( this->outopt_localrxfront == OUTPUT_RXFRONTSTATS_YES ) {
		log_rxfrontstats();
	}

	//single at the moment and data are stored not localized at the moment!
	//##MK::ideally time-resolved growth data of individual grains is spread on the threads to reduce overhead in get_interp_cellCount, not now
	if ( Xcells >= output_atthisX[loginfo_grainevo_cnt] && loginfo_grainevo_cnt < output_atthisX.size() ) {
		//##MK::if no solitary unit analyses are conducted this storing might not even be necessary, disable to save memory!
                std::cout << "Is there output of rxarea profile ----??????" << std::endl;
		log_grainevo();
		log_rxareaprofile( 13 ); //accounts for all
	}

	//previous the outputting of CA sections was controlled by passing volume or area rx fractions above predefined thresholds
	//and then output was rendered for all defined sections
	//for Diehl/Kühbach, MMM2018 Special Issue paper though we monitor first the recrystallized fraction on specific planes
	//and output individually when these reach predefined thresholds

	//MK::NO PARALLEL RENDERING AT THE MOMENT
	//2D and 3D sections
	if ( renderingForThisCA == true ) {
		//##MK::Diehl/Kühbach way of accounting for MMM2018 January2019 study is that the inspection area needs sufficient RX fraction

		uint32_t z00_logcnt = loginfo_rendering_sectionbased_cnt.at(0);
		if ( z00_logcnt < rendering_atthisX.size() ) {

			double z00_area_13px = ompcrit_probe_semebsd_core( 0.0, 13 ); //returns fractions not counts!
			//cout << "z00_area_13px\t\t" << z00_area_13px << endl;

			if ( z00_area_13px >= rendering_atthisX[z00_logcnt] ) {
				if ( outopt_localrenderhow == RENDERING_MS2D &&
						outopt_localrenderfileformat == RENDERING_FILEFORMAT_RAW &&
							outopt_localrendercolor == RENDERING_COLOR_IPFZ ) {
							//MK::also rxg grains smaller than threshold are written out!
							ompcrit_write_zsection_looping_ipfz( z00_area_13px, COLORIZE_RX_LEAVE_DEFORMED_BLACK );
							ompcrit_write_zsection_looping_ipfz( z00_area_13px, COLORIZE_DEF_LEAVE_RX_BLACK );
							ompcrit_write_zsection_looping_ipfz( z00_area_13px, COLORIZE_RX_RHOGREYSCALE_DEFORMED );
				}
				if ( outopt_localartsemebsd == OUTPUT_SEMEBSD_YES ) {
	cout << "Reporting a zsection at 0.0 for z00_logcnt\t\t" << z00_logcnt << "\t\t" << z00_area_13px << endl;
					ompcrit_write_semebsd_core( 0.0, z00_area_13px, 13, DEFAULT_SEMEBSD_MODE );
				}
				loginfo_rendering_sectionbased_cnt.at(0)++;
			}
		}

		uint32_t z05_logcnt = loginfo_rendering_sectionbased_cnt.at(1);
		if ( z05_logcnt < rendering_atthisX.size() ) {

			double z05_area_13px = ompcrit_probe_semebsd_core( 0.5, 13 );
			//cout << "z05_area_13px\t\t" << z05_area_13px << endl;

			if ( z05_area_13px >= rendering_atthisX[z05_logcnt] ) {
				if ( outopt_localrenderhow == RENDERING_MS2D &&
						outopt_localrenderfileformat == RENDERING_FILEFORMAT_RAW &&
							outopt_localrendercolor == RENDERING_COLOR_IPFZ ) {
							ompcrit_write_zsection_looping_ipfz( z05_area_13px, COLORIZE_RX_LEAVE_DEFORMED_BLACK );
							ompcrit_write_zsection_looping_ipfz( z05_area_13px, COLORIZE_DEF_LEAVE_RX_BLACK );
							ompcrit_write_zsection_looping_ipfz( z05_area_13px, COLORIZE_RX_RHOGREYSCALE_DEFORMED );
				}
				if ( outopt_localartsemebsd == OUTPUT_SEMEBSD_YES ) {
	cout << "Reporting a zsection at 0.5 for z05_logcnt\t\t" << z05_logcnt << "\t\t" << z05_area_13px << endl;
					ompcrit_write_semebsd_core( 0.5, z05_area_13px, 13, DEFAULT_SEMEBSD_MODE );
				}
				loginfo_rendering_sectionbased_cnt.at(1)++;
			}
		}

		uint32_t z10_logcnt = loginfo_rendering_sectionbased_cnt.at(2);
		if ( z10_logcnt < rendering_atthisX.size() ) {

			double z10_area_13px = ompcrit_probe_semebsd_core( 1.0, 13 );
			//cout << "z10_area_13px\t\t" << z10_area_13px << endl;

			if ( z10_area_13px >= rendering_atthisX[z10_logcnt] ) {
				if ( outopt_localrenderhow == RENDERING_MS2D &&
					outopt_localrenderfileformat == RENDERING_FILEFORMAT_RAW &&
						outopt_localrendercolor == RENDERING_COLOR_IPFZ ) {
						ompcrit_write_zsection_looping_ipfz( z10_area_13px, COLORIZE_RX_LEAVE_DEFORMED_BLACK );
						ompcrit_write_zsection_looping_ipfz( z10_area_13px, COLORIZE_DEF_LEAVE_RX_BLACK );
						ompcrit_write_zsection_looping_ipfz( z10_area_13px, COLORIZE_RX_RHOGREYSCALE_DEFORMED );
				}
				if ( outopt_localartsemebsd == OUTPUT_SEMEBSD_YES ) {
	cout << "Reporting a zsection at 1.0 for z10_logcnt\t\t" << z10_logcnt << "\t\t" << z10_area_13px << endl;
					ompcrit_write_semebsd_core( 1.0, z10_area_13px, 13, DEFAULT_SEMEBSD_MODE );
				}
				loginfo_rendering_sectionbased_cnt.at(2)++;
			}
		} //done for this CA
	}
}


void caHdl::binarize_partiallyrxed_microstructure( vector<bool>* outp )
{
	//threshold the structure into deformed (false) and infected + fully rxed (true)
	//MK::outp vector is already allocated
	size_t n = this->myCAGeometry.nboxvol_rdtdnd;
	outp->assign( n, false ); //set entire structure as deformed

	uint32_t nthreads = this->regions.size();
	for ( uint32_t thr = 0; thr < nthreads; thr++ ) {
		uint32_t xmi = this->regions[thr]->myGeom.nreg_rdmin; //global SU coordinates all inclusive [imi, imx]
		uint32_t ymi = this->regions[thr]->myGeom.nreg_tdmin;
		uint32_t zmi = this->regions[thr]->myGeom.nreg_ndmin;

		uint32_t rx = this->regions[thr]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
		uint32_t ry = this->regions[thr]->myGeom.nreg_td;
		uint32_t rz = this->regions[thr]->myGeom.nreg_nd;
		uint32_t rxy = rx*ry;

		uint32_t gx = this->myCAGeometry.nboxedge_rd; //global SU extent
		uint32_t gxy = this->myCAGeometry.nboxarea_rdtd;

		//copy dislocation densities from the buffer
		uint32_t* thegrid = this->regions[thr]->mycellgrid;
		uint32_t reg_ndgp = this->regions[thr]->reg_nmydefgpool;
		uint32_t c = 0;
		uint32_t zroff, zwoff, yzroff, xyzwoff;
		for ( uint32_t z = 0; z < rz; z++ ) {
			zroff = z*rxy;
			zwoff = (z+zmi)*gxy;
			for ( uint32_t y = 0; y < ry; y++ ) {
				yzroff = y*rx + zroff;
				xyzwoff = xmi+(y+ymi)*gx + zwoff;
				for ( uint32_t x = 0; x < rx; x++ ) {
					c = thegrid[x+yzroff]; //read cell index, recrystallized?
					if ( c >= reg_ndgp ) 
						outp->at(x+xyzwoff) = true;
					if ( c == CURRENTLY_INFECTED )
						outp->at(x+xyzwoff) = true;
				} //scan +x line
			} //scan +y line
		} //scan +z lines
	} //for each thread
}


void caHdl::log_PERCOLATION( void )
{
	//CALLED FROM WITHIN A PARALLEL REGION
	//MK::currently called by omp thread master only!
	struct loginfo_perc foo;
	double t0 = omp_get_wtime();
	double t1 = t0;
	double t2 = t0;

	if ( percolation != PERCOLATION_ANALYZE_NO ) {
		if ( this->myCAGeometry.nboxedge_rd != this->myCAGeometry.nboxedge_td || this->myCAGeometry.nboxedge_rd != this->myCAGeometry.nboxedge_nd ) {
			std::cout << "WARNING::Currently the Hoshen-Kopelman-based cluster labeling analysis works only for cubic domains!" << std::endl;
			return;
		}

		//test whether the non periodic structure percolates...
		bool NetworkPercolates = false;

		vector<bool>* binarized = NULL;
		binarized = new vector<bool>;
		QUICKASSERT( binarized != NULL );

		binarize_partiallyrxed_microstructure( binarized );

		//##MK::NUMA memory issues! this analyzer is at the moment in the memory of the master thread only!
		percAnalyzer* hk = new percAnalyzer;

		//pass binarized partially recrystallized microstructure to the analyzer to label cluster and check for percolation...
		hk->initialize( binarized, this->myCAGeometry.nboxedge_rd );
		delete binarized; binarized = NULL;

		t2 = omp_get_wtime();
		foo.ProfInitializing = t2 - t1;
		t1 = omp_get_wtime();

		hk->hoshen_kopelman();

		t2 = omp_get_wtime();
		foo.ProfHoshenKopeling = t2 - t1;
		t1 = omp_get_wtime();

		hk->compactify();

		t2 = omp_get_wtime();
		foo.ProfCompactifying = t2 - t1;
		t1 = omp_get_wtime();

		hk->checkLabeling();

		t2 = omp_get_wtime();
		foo.ProfCheckLabeling = t2 - t1;
		t1 = omp_get_wtime();

		NetworkPercolates = hk->percolates();

		t2 = omp_get_wtime();
		foo.ProfCheckPercolating = t2 - t1;
		t1 = omp_get_wtime();

		if ( percolation == PERCOLATION_ANALYZE_YES_SZDISTR ) {
			hk->determine_clustersize_distr();

			foo.LargestCluster = hk->getClusterSize();
			foo.NCluster = hk->getNCluster();
			foo.Percolation = NetworkPercolates;

			if ( foo.NCluster > 0 ) {
				//MK::pass cluster size distribution to CA class object as only he has access to the omp_lock handling of the hybrid parallel region
				unsigned int* distr = NULL;
				distr = new unsigned int[foo.NCluster];
				QUICKASSERT( distr != NULL );
				hk->handover_distribution( distr );
				write_clustersizedistr_h5( distr, foo.NCluster );
				delete [] distr;
			}

			t2 = omp_get_wtime();
			foo.ProfCharacterizing = t2 - t1;
			t1 = omp_get_wtime();
		}

		delete hk;

		t1 = omp_get_wtime();
		if ( NetworkPercolates == true ) {
			if ( loginfo_percolation_cnt == 0 ) { //MK::ones a recrystallizing volume percolates it will continue to do so, but we would like to output only once!
				write_damask_matconfig_ascii( "NetworkFirstPercolation" );
				write_damask_geom_ascii( "NetworkFirstPercolation" );
				//write_damask_geom_h5( "NetworkFirstPercolation" );
				loginfo_percolation_cnt++;
			}
		}
		t2 = omp_get_wtime() - t1;
	}

	//cout << "\t\tPercolationAnalysis step " << this->step << " " << ( omp_get_wtime() - t0 ) << " seconds" << endl;
	foo.ProfPercTotal = omp_get_wtime() - t0 - t2;
	this->myhk.push_back( foo );
}

//void caHdl::log_grain_IDs( double t ){
//     // write the grain IDs to a hdf5 file
//     // First need to get the grain IDs from different regions
//     // Following the code from EBSD Importing
//        int r, index;
//	uint32_t rxmi, rxmx, rymi, rymx, rzmi, rzmx, extentx, extenty;
//	//no domain partitioning into regions along x, but along y and z
//	uint32_t npy = this->regions[0]->thePartitioning.nreg_tdy; //MK::fixed array access with 0 works because there is always at least one region and all regions know the global partitioning
//	uint32_t npz = this->regions[0]->thePartitioning.nreg_ndz;
//	uint32_t ry, rz;
//	uint32_t seedid;
//        std::vector<int> grain_ID(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd);
//
//	for ( uint32_t z = 0; z < myCAGeometry.nboxedge_nd; z++ ) {
//		//in which xy slab of regions is this particular CA layer xy, z included?
//		for ( uint32_t rr = 0; rr < (1*npy*npz); rr = rr + npy ) { //mind that implicit region id 0 unfolds into ry=0,rz=0, if npy e.g. =3 then next zlayer in 3
//			if ( z >= this->regions[rr]->myGeom.nreg_ndmin && z <= this->regions[rr]->myGeom.nreg_ndmax ) { //works because linear cluster of regions along y has same dimensions
//				rz = rr / npy; break;
//			}
//		}
//
//		for ( uint32_t y = 0; y < myCAGeometry.nboxedge_td; y++ ) {
//			//in which slab yregion is the particular coordinate line y?
//			for ( uint32_t rr = 0+(rz*npy); rr < npy+(rz*npy); rr++ ) { //mind that caHdl->regions is organized as contiguous groups of regions along +y stacked in +z
//				if ( y >= this->regions[rr]->myGeom.nreg_tdmin && y <= this->regions[rr]->myGeom.nreg_tdmax ) {
//					ry = rr-(rz*npy); break;
//				}
//			}
//			//no region partitioning along x so now we know in which particular region to place the xline assignments
//			uint32_t here = ry+(rz*npy); //MK::mentality --- a pixel from the EBSD mapping becomes mapped on global CA volume that is partitioned into regions
//			uint32_t* thecells = this->regions[here]->mycellgrid;
//
//			//x,y,z are SU global coordinates, we have to translate into local coordinates...
//			rxmi = this->regions[here]->myGeom.nreg_rdmin;
//			rxmx = this->regions[here]->myGeom.nreg_rdmax;
//			rymi = this->regions[here]->myGeom.nreg_tdmin;
//			rymx = this->regions[here]->myGeom.nreg_tdmax; //local region cell coordinate inclusive limits
//			rzmi = this->regions[here]->myGeom.nreg_ndmin;
//			rzmx = this->regions[here]->myGeom.nreg_ndmax;
//			extentx = rxmx - rxmi + 1;
//			QUICKASSERT( extentx == myCAGeometry.nboxedge_rd );
//			extenty = rymx - rymi + 1;
//
//                        uint32_t ncandidates;
//                        cellP front = NULL;
//                        ncandidates = this->regions[here]->nextSlotNeverActiveRXFrontInside;
//                        //std::cout << "Slot never active rx front inside" << ncandidates << std::endl;
//                        front = this->regions[here]->myRXFrontInside;
//                        for (uint32_t c = 0; c < ncandidates; c++){
//                            //if (front[c].activity == ACTIVE){
//                            if (front[c].activity == ACTIVE && c == 1038){
//                            //if (c == 1){
//                               //if (here == 3){
//                               //std::cout << "region is" << here << std::endl;
//                               std::cout << "Deformation grain id --------------------------" << front[c].mydefgid << std::endl;
//                               std::cout << "Recrystallized grain id --------------------------" << front[c].myrxgid << std::endl;
//                               std::cout << "Candidate is --------------------------" << c << std::endl;
//                               //int ind;
//			       //ind = front[c].ix + (front[c].iy * myCAGeometry.nboxedge_rd) + (front[c].iz * (myCAGeometry.nboxedge_rd)*(myCAGeometry.nboxedge_td)); //ix, ... are short but positive! so short to uint32_t no problem
//                               //std::cout << "Index is" << ind << std::endl;
//                               //std::cout << "Front x is" << front[c].ix << std::endl;
//                               //std::cout << "Front y is" << front[c].iy << std::endl;
//                               //std::cout << "Front z is" << front[c].iz << std::endl;
//                               //}
//                            }
//                        }
//
//                        //std::cout << "check if cellStatii exists" << this->cellStatii.size() << std::endl;
//                        //std::cout << "see inside cellStatii" << this->cellStatii.at(0) << "\n" << std::endl;
//                        //std::cout << "Can we find size of cell statii" << this->cellStatii.at(0)->size() << std::endl;
//                        //uint32_t ncandidates = this->cellStatii.at(0)->size(); 
//                        //std::cout << "Hellooooooooooooooooo candidates" << std::endl;
//                        //for (uint32_t cand = 0; cand < ncandidates; cand ++){
//                        //      std::cout << "Going over each candidate here" << cand << "\n" << std::endl;
//                        //}
//
//			uint32_t yzoffset = (y-rymi)*extentx + (z-rzmi)*extentx*extenty;
//			for ( uint32_t x = 0; x < extentx; x++ ) { //setup region partitioning more efficiently
//				//ebsdgid = myensHdl->expData[px].gid; //the grain at this position, expGrains is contiguous with IDs on [1,N]
//                                index = x + y*myCAGeometry.nboxedge_rd + z*(myCAGeometry.nboxedge_rd)*(myCAGeometry.nboxedge_td);
// 
//                                grain_ID[index] = thecells[x+yzoffset];
//                                //if (thecells[x+yzoffset] == CURRENTLY_INFECTED){
//                                //   grain_ID[index] = 0;
//                                //   continue;
//                                //}
//                                //if (thecells[x+yzoffset] <= CA_GRAINIDS_MAXIMUM){
//                                //   grain_ID[index] = thecells[x+yzoffset] + 1;
//                                //   continue;
//                                //}
//				//Fill the array with the data from CA
//				//grain_ID[index] = thecells[x+yzoffset];
//                                //if (index==327606 & t == 0.0){
//                                //std::cout << "x value is " << x << std::endl;
//                                //std::cout << "y value is " << y << std::endl;
//                                //std::cout << "z value is " << z << std::endl;
//                                //std::cout << "The actual theCells value" << thecells[x+yzoffset] << std::endl;
//                                //std::cout << "maybe the grainID gets converted" << grain_ID[index] << std::endl;
//                                //}
//			} //xline completed
//		} //ylines done
//
//	//} //all zlayers were build
//
//
//}
//        //std::cout << "Going to reopen the HDF5 file --------- \n" << std::endl;
//        //std::cout << "The index 6 grainID is: " << grain_ID[6] << std::endl;
//        //std::cout << "If this is infected then should be zero ------------" << grain_ID[327606] << std::endl;
//
//        //if (grain_ID[6] == CELL_IS_A_PARTICLE){
//        //std::cout << "The cell is a particle " << grain_ID[6] << std::endl;
//        //}
//        //if (grain_ID[6] == CURRENTLY_INFECTED){
//        //std::cout << "This is infected and thus RXed" << grain_ID[6] << std::endl;
//        //}
//        //if (grain_ID[6] <= CA_GRAINIDS_MAXIMUM){
//        //   if (grain_ID[6] > this->mydefgpool.size()){
//        //      std::cout << "This is Rxed dont know how" << std::endl;
//        //   }
//        //}
//
//        // need to create the HDF5 file outside the parallel region (inside the parallel region might need some other work arounds)
//        string fname = "/home/georg.falkinger/Rolling_partial_RX/CA_example/simulation/postProc/CA_output.hdf5";
//        HDF5_write_CA_output_int( fname, t, grain_ID );
//
//}

void caHdl::log_grain_IDs( double t ){
     // write the grain IDs to a hdf5 file
     // First need to get the grain IDs from different regions
     // Following the code from EBSD Importing
        int r, index, ixyz;
	//no domain partitioning into regions along x, but along y and z
	uint32_t npy = this->regions[0]->thePartitioning.nreg_tdy; //MK::fixed array access with 0 works because there is always at least one region and all regions know the global partitioning
	uint32_t npz = this->regions[0]->thePartitioning.nreg_ndz;
        std::vector<int> grain_ID(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd);

        uint32_t ndg = this->mydefgpool.size();
        uint32_t nthreads = this->regions.size();
        uint32_t ncandidates = 0;
        cellP front = NULL;

	for ( uint32_t r = 0; r < nthreads; r++ ) {
	    uint32_t rx = this->regions[r]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
	    uint32_t ry = this->regions[r]->myGeom.nreg_td;
	    uint32_t rz = this->regions[r]->myGeom.nreg_nd;
            uint32_t rymin = this->regions[r]->myGeom.nreg_tdmin;
            uint32_t rzmin = this->regions[r]->myGeom.nreg_ndmin;
	    uint32_t rxy = rx*ry;
            uint32_t yzoffset;
	    uint32_t c = 0;

	    uint32_t* thegrid = this->regions[r]->mycellgrid;
   
            for ( uint32_t z = 0; z < rz; z++){
                for (uint32_t y = 0; y < ry; y++){
                    yzoffset = y*rx + z*ry*rx; 
                    for (uint32_t x = 0; x < rx; x++){
                       index = x + (y + rymin)*myCAGeometry.nboxedge_rd + (z + rzmin)*(myCAGeometry.nboxedge_rd)*(myCAGeometry.nboxedge_td);
                       c = thegrid[x + yzoffset];
                       if (c <= CA_GRAINIDS_MAXIMUM){ //not infected
                          grain_ID[index] = c;
                       }
                    
                       } 

                }

             } // z layer completed
             
             // assuming that myRXFrontInside & myRXFrontBorder have all the infected cells
             // analyze cells inside
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontInside;
	     front = this->regions[r]->myRXFrontInside;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       grain_ID[ixyz] = front[c].mydefgid;
                    }
                    grain_ID[ixyz] = ndg + front[c].myrxgid;
                 }
             }
   
             // analyse cells border
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontBorder;
	     front = this->regions[r]->myRXFrontBorder;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       grain_ID[ixyz] = front[c].mydefgid;
                    }
                    grain_ID[ixyz] = ndg + front[c].myrxgid;
                 }
             }
             
        } // loop for threads
        // need to create the HDF5 file outside the parallel region (inside the parallel region might need some other work arounds)
        string fname = "/home/georg.falkinger/Rolling_partial_RX/CA_example/simulation_check/postProc/CA_output.hdf5";
        HDF5_write_CA_output_int( fname, t, grain_ID );

}

void caHdl::log_grain_rho( double t ){
     // write the dislocation density of the grains to a hdf5 file
        int r, index, ixyz;
	//no domain partitioning into regions along x, but along y and z
	uint32_t npy = this->regions[0]->thePartitioning.nreg_tdy; //MK::fixed array access with 0 works because there is always at least one region and all regions know the global partitioning
	uint32_t npz = this->regions[0]->thePartitioning.nreg_ndz;
        std::vector<int> grain_ID(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd);
        std::vector<double> rho(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd);

        uint32_t ndg = this->mydefgpool.size();
        uint32_t nthreads = this->regions.size();
        uint32_t ncandidates = 0;
        cellP front = NULL;

	for ( uint32_t r = 0; r < nthreads; r++ ) {
	    uint32_t rx = this->regions[r]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
	    uint32_t ry = this->regions[r]->myGeom.nreg_td;
	    uint32_t rz = this->regions[r]->myGeom.nreg_nd;
            uint32_t rymin = this->regions[r]->myGeom.nreg_tdmin;
            uint32_t rzmin = this->regions[r]->myGeom.nreg_ndmin;
	    uint32_t rxy = rx*ry;
            uint32_t yzoffset;
	    uint32_t c = 0;

	    uint32_t* thegrid = this->regions[r]->mycellgrid;
   
            for ( uint32_t z = 0; z < rz; z++){
                for (uint32_t y = 0; y < ry; y++){
                    yzoffset = y*rx + z*ry*rx; 
                    for (uint32_t x = 0; x < rx; x++){
                       index = x + (y + rymin)*myCAGeometry.nboxedge_rd + (z + rzmin)*(myCAGeometry.nboxedge_rd)*(myCAGeometry.nboxedge_td);
                       c = thegrid[x + yzoffset];
                       if (c <= CA_GRAINIDS_MAXIMUM){ //not infected either RX or deformed
                          if (c >= ndg){ // recrystallized
                              rho[index] = RHO_RECRYSTALLIZED_MATERIAL; 
                          }
                          else { // deformed
                              rho[index] = mydefgpool[c].rho;
                          }
                       }
                    
                       } 

                }

             } // z layer completed
             
             // assuming that myRXFrontInside & myRXFrontBorder have all the infected cells
             // analyze cells inside
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontInside;
	     front = this->regions[r]->myRXFrontInside;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       //grain_ID[ixyz] = front[c].mydefgid;
                       rho[ixyz] = mydefgpool[front[c].mydefgid].rho;
                    }
                    //grain_ID[ixyz] = ndg + front[c].myrxgid;
                    rho[ixyz] = RHO_RECRYSTALLIZED_MATERIAL; 
                 }
             }
   
             // analyse cells border
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontBorder;
	     front = this->regions[r]->myRXFrontBorder;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       //grain_ID[ixyz] = front[c].mydefgid;
                       rho[ixyz] = mydefgpool[front[c].mydefgid].rho;
                    }
                    //grain_ID[ixyz] = ndg + front[c].myrxgid;
                    rho[ixyz] = RHO_RECRYSTALLIZED_MATERIAL; 
                 }
             }
             
        } // loop for threads

        // loop over the grain IDs to fill the rho array
        
        // need to create the HDF5 file outside the parallel region (inside the parallel region might need some other work arounds)
        string fname = "/home/georg.falkinger/Rolling_partial_RX/CA_example/simulation_check/postProc/CA_output.hdf5";
        HDF5_write_CA_output_double_rank_1( fname, t, rho );

}

void caHdl::log_grain_ori( double t ){
     // write the orientations (quaternions) of the grains to a hdf5 file
        int r, index, ixyz, oid;
	//no domain partitioning into regions along x, but along y and z
	uint32_t npy = this->regions[0]->thePartitioning.nreg_tdy; //MK::fixed array access with 0 works because there is always at least one region and all regions know the global partitioning
	uint32_t npz = this->regions[0]->thePartitioning.nreg_ndz;
        std::vector<int> grain_ID(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd);
        std::vector<std::vector<double>> ori_quats(myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td*myCAGeometry.nboxedge_nd, std::vector<double>(4));

        uint32_t ndg = this->mydefgpool.size();
        uint32_t nthreads = this->regions.size();
        uint32_t ncandidates = 0;
        cellP front = NULL;

	for ( uint32_t r = 0; r < nthreads; r++ ) {
	    uint32_t rx = this->regions[r]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
	    uint32_t ry = this->regions[r]->myGeom.nreg_td;
	    uint32_t rz = this->regions[r]->myGeom.nreg_nd;
            uint32_t rymin = this->regions[r]->myGeom.nreg_tdmin;
            uint32_t rzmin = this->regions[r]->myGeom.nreg_ndmin;
	    uint32_t rxy = rx*ry;
            uint32_t yzoffset;
	    uint32_t c = 0;

	    uint32_t* thegrid = this->regions[r]->mycellgrid;
   
            for ( uint32_t z = 0; z < rz; z++){
                for (uint32_t y = 0; y < ry; y++){
                    yzoffset = y*rx + z*ry*rx; 
                    for (uint32_t x = 0; x < rx; x++){
                       index = x + (y + rymin)*myCAGeometry.nboxedge_rd + (z + rzmin)*(myCAGeometry.nboxedge_rd)*(myCAGeometry.nboxedge_td);
                       c = thegrid[x + yzoffset];
                       if (c <= CA_GRAINIDS_MAXIMUM){ //not infected either RX or deformed
                          if (c >= ndg){ // recrystallized
                              //rho[index] = RHO_RECRYSTALLIZED_MATERIAL; 
                              oid = myrxgpool[c - ndg].caori;
                              ori_quats[index][0] = myoripool[oid].q0; 
                              ori_quats[index][1] = myoripool[oid].q1; 
                              ori_quats[index][2] = myoripool[oid].q2; 
                              ori_quats[index][3] = myoripool[oid].q3; 
                          }
                          else { // deformed
                              //rho[index] = mydefgpool[c].rho;
                              oid = mydefgpool[c].caori;
                              ori_quats[index][0] = myoripool[oid].q0; 
                              ori_quats[index][1] = myoripool[oid].q1; 
                              ori_quats[index][2] = myoripool[oid].q2; 
                              ori_quats[index][3] = myoripool[oid].q3; 
                          }
                       }
                    
                       } 

                }

             } // z layer completed
             
             // assuming that myRXFrontInside & myRXFrontBorder have all the infected cells
             // analyze cells inside
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontInside;
	     front = this->regions[r]->myRXFrontInside;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       //grain_ID[ixyz] = front[c].mydefgid;
                       oid = mydefgpool[front[c].mydefgid].caori;
                       ori_quats[ixyz][0] = myoripool[oid].q0;
                       ori_quats[ixyz][1] = myoripool[oid].q1;
                       ori_quats[ixyz][2] = myoripool[oid].q2;
                       ori_quats[ixyz][3] = myoripool[oid].q3;
                    }
                    //grain_ID[ixyz] = ndg + front[c].myrxgid;
                    //rho[ixyz] = RHO_RECRYSTALLIZED_MATERIAL; 
                    oid = myrxgpool[front[c].myrxgid].caori;
                    ori_quats[ixyz][0] = myoripool[oid].q0;
                    ori_quats[ixyz][1] = myoripool[oid].q1;
                    ori_quats[ixyz][2] = myoripool[oid].q2;
                    ori_quats[ixyz][3] = myoripool[oid].q3;
                 }
             }
   
             // analyse cells border
	     ncandidates = this->regions[r]->nextSlotNeverActiveRXFrontBorder;
	     front = this->regions[r]->myRXFrontBorder;
	     for ( uint32_t c = 0; c < ncandidates; c++ ) {
	         if ( front[c].activity == ACTIVE ) { //most likely case for densely populated list
                    ixyz = front[c].ix + front[c].iy*myCAGeometry.nboxedge_rd + front[c].iz*myCAGeometry.nboxedge_rd*myCAGeometry.nboxedge_td;
                    if (front[c].rxFrac < RXFRACTION_THRESHOLD){
                       //grain_ID[ixyz] = front[c].mydefgid;
                       oid = mydefgpool[front[c].mydefgid].caori;
                       ori_quats[ixyz][0] = myoripool[oid].q0;
                       ori_quats[ixyz][1] = myoripool[oid].q1;
                       ori_quats[ixyz][2] = myoripool[oid].q2;
                       ori_quats[ixyz][3] = myoripool[oid].q3;
                    }
                    //grain_ID[ixyz] = ndg + front[c].myrxgid;
                    oid = myrxgpool[front[c].myrxgid].caori;
                    ori_quats[ixyz][0] = myoripool[oid].q0;
                    ori_quats[ixyz][1] = myoripool[oid].q1;
                    ori_quats[ixyz][2] = myoripool[oid].q2;
                    ori_quats[ixyz][3] = myoripool[oid].q3;
                 }
             }
             
        } // loop for threads

        // loop over the grain IDs to fill the rho array
        
        // need to create the HDF5 file outside the parallel region (inside the parallel region might need some other work arounds)
        string fname = "/home/georg.falkinger/Rolling_partial_RX/CA_example/simulation_check/postProc/CA_output.hdf5";
        HDF5_write_CA_output_double_rank_2( fname, t, ori_quats );

}



#endif
