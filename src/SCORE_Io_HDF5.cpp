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

bool caHdl::hdf5_write_coordinates( string fname, string grpname )
{
	std::cout << "Setting up new HDF5 file and writing coordinates of rendering window..." << endl;

	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //region topology how many memory regions in x, y, and z?
	//uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	//uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	if ( npx != 1 ) { std::cout << "ERROR::No 3D output because npx != 1!" << std::endl; return false; }

	struct loginfo_xdmf xmf;
	xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
	xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
	xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;
	uint32_t nxyz = xmf.nx * xmf.ny * xmf.nz;

	//check if total size of the dataset can be written with HDF5 at once keeping in mind the underlying
	//MPII/O which cannot handle writes of >2GB!
	if ( (nxyz*3*sizeof(unsigned int)) > HDF5_MAXIMUM_SINGLE_WRITE ) {
		cout << "ERROR::HDF5 cannot write " << (nxyz*3*sizeof(unsigned int)) << " Bytes at once!" <<endl; 
		return false;
	}

	/*unsigned int** corr = NULL; //set up coordinate field
	corr = new unsigned int*[nxyz];
	uint32_t cxyz = 0;
	for ( cxyz = 0; cxyz < nxyz; cxyz++ ) 
		corr[cxyz] = new unsigned int[3];
	cxyz = 0; //fill coordinates
	for ( uint32_t z = 0; z < xmf.nz; z++ ) { 
		for ( uint32_t y = 0; y < xmf.ny; y++ ) {
			for ( uint32_t x = 0; x < xmf.nx; x++ ) {
				corr[cxyz][0] = x;
				corr[cxyz][1] = y;
				corr[cxyz][2] = z;
				cxyz++;
			}
		}
	}*/
	unsigned int* corr = new unsigned int[3*nxyz];
	unsigned int cxyz = 0;
	for ( uint32_t z = 0; z < xmf.nz; z++ ) { 
		for ( uint32_t y = 0; y < xmf.ny; y++ ) {
			for ( uint32_t x = 0; x < xmf.nx; x++ ) {
				corr[3*cxyz+0] = myrenderwindow.xmi + x;
				corr[3*cxyz+1] = myrenderwindow.ymi + y;
				corr[3*cxyz+2] = myrenderwindow.zmi + z;
				cxyz++;
			}
		}
	}

	//overwrite existing file and truncate its content!
	hid_t file, space, dset;
	herr_t status;

	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	hsize_t dims[2] = {nxyz, 3};
	space = H5Screate_simple (2, dims, NULL);
	dset = H5Dcreate(file, grpname.c_str(), H5T_STD_U32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	status = H5Dwrite(dset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, corr);

	status = H5Dclose(dset);
	status = H5Sclose(space);
	status = H5Fclose(file);

	/*for ( cxyz = 0; cxyz < nxyz; cxyz++ ) 
		delete [] corr[cxyz];
	delete [] corr;*/
	delete [] corr;

	return true;
}


bool caHdl::hdf5_write_voxelgrid_grainid( string fname, string grpname, string subgrpname )
{
	//generate anew or append to existing file
	std::cout << "Preparing the writing of grainIDs to the HDF5 file..." << endl;

	//region topology how many memory regions in x, y, and z?
	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //1 required
	uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	if ( npx != 1 ) { std::cout << "ERROR::No 3D output because npx != 1!" << std::endl; return false; }

	struct loginfo_xdmf xmf;
	xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
	xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
	xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;
	uint32_t nxyz = xmf.nx * xmf.ny * xmf.nz;

	//check if total size of the dataset can be written with HDF5 at once keeping in mind the underlying MPII/O which cannot handle writes of >2GB!
	if ( nxyz * sizeof(uint32_t) > HDF5_MAXIMUM_SINGLE_WRITE ) { cout << "ERROR::HDF5 cannot write " << (nxyz * sizeof(uint32_t)) << " Bytes at once!" <<endl; 	return false; }

	uint32_t* buffer = NULL;
	buffer = new uint32_t[nxyz];
	QUICKASSERT( buffer != NULL );

	//TARGET FORMAT IS UINT32 FOR PARAVIEW
	//CELL_IS_PARTICLE	0
	//CELL_IS_INFECTED	1
	//DEFORMED 			2+mydefgid //because there is defgid = 0
	//RECRYSTALLIZED	3+mydefgid.size()+myrxgid

	//the automaton comprises npz regions along z, x is aligned, and y as well, so simply read x*y slab stack in npy direction and drop such a global z section into rawdata
	//##MK::at the moment a safe but less efficient than an optimized buffering strategy of working sequentially through xy layers along z is performed
	//uint32_t zper;
	uint32_t yper, xper;
	uint32_t zstart, ystart, xstart, zlim, ylim, xlim;
	uint32_t rzmi, rymi, rxmi;
	//..lim are not ..mx!
	uint32_t r;
	uint32_t corr, gridvalue;

	uint32_t c = 0; //implicitly fill buffer arranging +x lines and stacked them along +y to ultimatively stack these xy slabs along +z
	for (uint32_t zregion = 0; zregion < npz; zregion++ ) { //utilize that the regions form a regular-stacked 3D region aggregate
		//we want to write only a portion of the data, so we have to check whether these limits are smaller than the region size
		//zper = this->regions[(zregion*npy)+0]->myGeom.nreg_nd; //is an extent in +z not a global coordinate in SU! utilize that all regions stacked along +y at a fixed z region coordinate have the same extent in z!
		//nreg_tdmax is a global coordinate in SU, how does it relate to zmx, the highest z global coordinate to render from SU
		zlim = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax > myrenderwindow.zmx) ? myrenderwindow.zmx : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax;
		zstart = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin < myrenderwindow.zmi) ? myrenderwindow.zmi : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;
		rzmi = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;

		for ( uint32_t zz = zstart; zz <= zlim; zz++ ) { //run over global SU coordinates, at one z coordinate however, npy regions are stacked on top of each other,i.e. in +y
			for(uint32_t yregion = 0; yregion < npy; yregion++ ) { //collect data over regions in the zlayer
				r = (zregion*npy)+yregion; //in which memory region are we now?

				yper = this->regions[r]->myGeom.nreg_td; //row of zregions have same height in y
				ylim = (this->regions[r]->myGeom.nreg_tdmax > myrenderwindow.ymx) ? myrenderwindow.ymx : this->regions[r]->myGeom.nreg_tdmax;
				ystart  = (this->regions[r]->myGeom.nreg_tdmin < myrenderwindow.ymi) ? myrenderwindow.ymi : this->regions[r]->myGeom.nreg_tdmin;
				rymi = this->regions[r]->myGeom.nreg_tdmin;

				xper = this->regions[r]->myGeom.nreg_rd; //and width in x
				xlim = (this->regions[r]->myGeom.nreg_rdmax > myrenderwindow.xmx) ? myrenderwindow.xmx : this->regions[r]->myGeom.nreg_rdmax; //inclusive global coordinates
				xstart = (this->regions[r]->myGeom.nreg_rdmin < myrenderwindow.xmi) ? myrenderwindow.xmi : this->regions[r]->myGeom.nreg_rdmin; 
				rxmi = this->regions[r]->myGeom.nreg_rdmin;

				for ( uint32_t yy = ystart; yy <= ylim; yy++ ) { //..mi and ..mx are global automaton coordinates!

					for ( uint32_t xx = xstart; xx <= xlim; xx++ ) { //utilize that npx == 1

						//transform into local memory region coordinate
						corr = (xx-rxmi) + (xper*(yy-rymi)) + (xper*yper*(zz-rzmi)); //xper*yper is size of one slab of cells in region r

						gridvalue = regions[r]->mycellgrid[corr];

						buffer[c] = INVALID_CELLASSIGNMENT;		//4096000003

						if ( gridvalue == CELL_IS_A_PARTICLE ) {	//0
							buffer[c] = 0; //RAW_PARTICLE;
							c++;
							continue;
						}
						if ( gridvalue == CURRENTLY_INFECTED ) {	//1
							buffer[c] = 1; //RAW_INFECTED;
							c++;
							continue;
						}
						//other cases already excluded
						if ( gridvalue <= CA_GRAINIDS_MAXIMUM ) {	//<=4096000002
							buffer[c] = 2 + gridvalue;
							c++;
							continue;
						}
					} //scan through xline
				} //scan +y the xlines stacked in region r
			} //next region stacked upon last one in +y
		} //next zslice in +zregions
	} //next region z with regions on stacked top of one another in y

	std::cout << "Writing to HDF5 file... " << fname << std::endl;

	hid_t file, space, dset, group; //, dcpl;
	herr_t status;

	//file exists?
	//ifstream h5file( fname.c_str() );
	//if(!h5file) { //create
	//	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	//}
	//else { //open readwrite of existing file
		file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );
	//}

	status = H5Eset_auto(NULL, NULL, NULL); //generate group if not existing
	status = H5Gget_objinfo(file, grpname.c_str(), 0, NULL);
	if(status < 0) { //group does not exist or other error, attempt to create it at least
		group = H5Gcreate2( file, grpname.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
		status = H5Gclose(group);
		if ( status < 0 ) { cout << "ERROR::HDF5 unable to create group " << grpname << endl; delete [] buffer; return false; }
	} //else group exists

	//generate subgroup grpname/subgroup
	//group = H5Gcreate2( file, subgrpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	//status = H5Gclose(group);

	//create the dimensions, space of the dataset and copy data from the SU volume
	hsize_t dims[3] = { xmf.nx, xmf.ny, xmf.nz };
	space = H5Screate_simple(3, dims, NULL);
	dset = H5Dcreate(file, subgrpname.c_str(), H5T_STD_U32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);//create the dataset with default properties

	status = H5Dwrite(dset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
	if ( status < 0 ) { cout << "ERROR::HDF5 unable to write buffer " << endl; delete [] buffer; return false; }

	status = H5Sclose(space);
	status = H5Dclose(dset);
	delete [] buffer; buffer = NULL;
	status = H5Fclose(file);
	return true;
}


bool caHdl::hdf5_write_voxelgrid_rho( string fname, string grpname, string subgrpname ) {
	std::cout << "Preparing the writing of dislocation density to the HDF5 file..." << endl;

	//region topology how many memory regions in x, y, and z?
	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //1 required
	uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	if ( npx != 1 ) { std::cout << "ERROR::No 3D output because npx != 1!" << std::endl; return false;}

	struct loginfo_xdmf xmf;
	xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
	xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
	xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;
	uint32_t nxyz = xmf.nx * xmf.ny * xmf.nz;

	//check if total size of the dataset can be written with HDF5 at once keeping in mind the underlying MPII/O which cannot handle writes of >2GB!
	if ( nxyz * sizeof(uint32_t) > HDF5_MAXIMUM_SINGLE_WRITE ) { cout << "ERROR::HDF5 cannot write " << (nxyz * sizeof(uint32_t)) << " Bytes at once!" <<endl; return false; }

	double* buffer = NULL;
	buffer = new double[nxyz];
	QUICKASSERT( buffer != NULL );
	for ( uint32_t i = 0; i < nxyz; i++ ) { 
		buffer[i] = RHO_RECRYSTALLIZED_MATERIAL; //##MK::currently no particles
	}

	//TARGET FORMAT IS DOUBLE FOR PARAVIEW
	//CELL_IS_PARTICLE	0.0
	//CELL_IS_INFECTED	RHO_RECRYSTALLIZED_MATERIAL
	//DEFORMED 			mydefgpool(gid).rho
	//RECRYSTALLIZED	RHO_RECRYSTALLIZED_MATERIAL, thus we only have to probe if gridvalue are < ndg

	//the automaton comprises npz regions along z, x is aligned, and y as well, so simply read x*y slab stack in npy direction and drop such a global z section into rawdata
	//##MK::at the moment a safe but less efficient than an optimized buffering strategy of working sequentially through xy layers along z is performed
	uint32_t yper, xper;
	uint32_t zstart, ystart, xstart, zlim, ylim, xlim;
	uint32_t rzmi, rymi, rxmi;
	//..lim are not ..mx!
	uint32_t r;
	uint32_t corr, gridvalue;
	uint32_t ndg = mydefgpool.size();

	uint32_t c = 0; //implicitly fill buffer arranging +x lines and stacked them along +y to ultimatively stack these xy slabs along +z
	for (uint32_t zregion = 0; zregion < npz; zregion++ ) { //utilize that the regions form a regular-stacked 3D region aggregate
		//we want to write only a portion of the data, so we have to check whether these limits are smaller than the region size
		//zper = this->regions[(zregion*npy)+0]->myGeom.nreg_nd; //is an extent in +z not a global coordinate in SU! utilize that all regions stacked along +y at a fixed z region coordinate have the same extent in z!
		//nreg_tdmax is a global coordinate in SU, how does it relate to zmx, the highest z global coordinate to render from SU
		zlim = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax > myrenderwindow.zmx) ? myrenderwindow.zmx : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax;
		zstart = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin < myrenderwindow.zmi) ? myrenderwindow.zmi : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;
		rzmi = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;

		for ( uint32_t zz = zstart; zz <= zlim; zz++ ) { //run over global SU coordinates, at one z coordinate however, npy regions are stacked on top of each other,i.e. in +y
			for(uint32_t yregion = 0; yregion < npy; yregion++ ) { //collect data over regions in the zlayer
				r = (zregion*npy)+yregion; //in which memory region are we now?

				yper = this->regions[r]->myGeom.nreg_td; //row of zregions have same height in y
				ylim = (this->regions[r]->myGeom.nreg_tdmax > myrenderwindow.ymx) ? myrenderwindow.ymx : this->regions[r]->myGeom.nreg_tdmax;
				ystart  = (this->regions[r]->myGeom.nreg_tdmin < myrenderwindow.ymi) ? myrenderwindow.ymi : this->regions[r]->myGeom.nreg_tdmin;
				rymi = this->regions[r]->myGeom.nreg_tdmin;

				xper = this->regions[r]->myGeom.nreg_rd; //and width in x
				xlim = (this->regions[r]->myGeom.nreg_rdmax > myrenderwindow.xmx) ? myrenderwindow.xmx : this->regions[r]->myGeom.nreg_rdmax; //inclusive global coordinates
				xstart = (this->regions[r]->myGeom.nreg_rdmin < myrenderwindow.xmi) ? myrenderwindow.xmi : this->regions[r]->myGeom.nreg_rdmin; 
				rxmi = this->regions[r]->myGeom.nreg_rdmin;

				for ( uint32_t yy = ystart; yy <= ylim; yy++ ) { //..mi and ..mx are global automaton coordinates!

					for ( uint32_t xx = xstart; xx <= xlim; xx++ ) { //utilize that npx == 1

						//transform into local memory region coordinate
						corr = (xx-rxmi) + (xper*(yy-rymi)) + (xper*yper*(zz-rzmi)); //xper*yper is size of one slab of cells in region r

						gridvalue = regions[r]->mycellgrid[corr];

						//likeliest either RX or DG, albeit for RX and CURRENTLY_INFECTED we output RHO_RECRYSTALLIZED_MATERIAL
						if ( gridvalue < ndg ) { //DEFORMED
							buffer[c] = get_rho( c );
						}

					} //scan through xline
				} //scan +y the xlines stacked in region r
			} //next region stacked upon last one in +y
		} //next zslice in +zregions
	} //next region z with regions on stacked top of one another in y

	std::cout << "Writing to HDF5 file... " << fname << std::endl;

	hid_t file, space, dset, group; //, dcpl;
	herr_t status;

	//file exists?
	//ifstream h5file( fname.c_str() );
	//if(!h5file) { //create
	//	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	//}
	//else { //open readwrite of existing file
		file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );
	//}

	status = H5Eset_auto(NULL, NULL, NULL); //generate group if not existing
	status = H5Gget_objinfo(file, grpname.c_str(), 0, NULL);
	if(status < 0) { //group does not exist or other error, attempt to create it at least
		group = H5Gcreate2( file, grpname.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
		status = H5Gclose(group);
		if ( status < 0 ) { cout << "ERROR::HDF5 unable to create group " << grpname << endl; delete [] buffer; return false; }
	} //else group exists

	//generate subgroup grpname/subgroup
	//group = H5Gcreate2( file, subgrpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	//status = H5Gclose(group);

	//create the dimensions, space of the dataset and copy data from the SU volume
	hsize_t dims[3] = { xmf.nx, xmf.ny, xmf.nz };
	space = H5Screate_simple(3, dims, NULL);
	dset = H5Dcreate(file, subgrpname.c_str(), H5T_IEEE_F64LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);//create the dataset with default properties

	status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
	if ( status < 0 ) { cout << "ERROR::HDF5 unable to write buffer " << endl; delete [] buffer; return false; }

	status = H5Sclose(space);
	status = H5Dclose(dset);
	delete [] buffer; buffer = NULL;
	status = H5Fclose(file);
	return true;
}


bool caHdl::mpiio_write_voxelgrid_ipfz( string fname )
{
	//generate anew or append to existing file
	std::cout << "Preparing the writing to the Binary file..." << endl;

	//region topology how many memory regions in x, y, and z?
	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //1 required
	uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	if ( npx != 1 ) { std::cout << "ERROR::No 3D output because npx != 1!" << std::endl;  }

	struct loginfo_xdmf xmf;
	xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
	xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
	xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;
	uint32_t nxy = xmf.nx * xmf.ny;
	//uint32_t nxyz = xmf.nx * xmf.ny * xmf.nz;

	//check if total size of the dataset can be written with HDF5 at once keeping in mind the underlying
	//MPII/O which cannot handle writes of >2GB!
	if ( nxy * (3*sizeof(unsigned char)) > MPIIO_MAXIMUM_SINGLE_WRITE ) {
		cout << "ERROR::MPI I/O cannot write " << (nxy * (3*sizeof(unsigned char))) << " Bytes at once!" <<endl; 
		return false;
	}

	unsigned char* buffer = NULL;
	buffer = new unsigned char[3*nxy];
	QUICKASSERT( buffer != NULL );

	//TARGET FORMAT IS UINT32 FOR PARAVIEW
	//CELL_IS_PARTICLE	BLACK;BLACK;BLACK
	//CELL_IS_INFECTED	BLACK;BLACK;BLACK
	//DEFORMED 			2+mydefgid //because there is defgid = 0
	//RECRYSTALLIZED	3+mydefgid.size()+myrxgid

	//the automaton comprises npz regions along +z, x is aligned, and y as well, so simply read x*y slab stack +y along +npy direction and drop such a global z section into rawdata
	//##MK::at the moment a safe but less efficient than an optimized buffering strategy of working sequentially through xy layers along z is performed
	//uint32_t zper;
	uint32_t yper, xper;
	uint32_t zstart, ystart, xstart, zlim, ylim, xlim;
	uint32_t rzmi, rymi, rxmi;
	//..lim are not ..mx!
	uint32_t r;
	uint32_t corr, gridvalue;

	//open MPI file
	MPI_File msFileHdl;
	MPI_Status msFileStatus;
	MPI_File_open(MPI_COMM_SELF, fname.c_str(), MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &msFileHdl);
	//MK:: with 64-bit architectures, offsets calculations can be performed as longs 2^64 bit thus on petabyte files...
	//in mpi.h MPI_Offset is defined as an __int64 which is long long, thus we can jump much more than 2^32 directly when unsigned int would be utilized
	__int64 totalOffset = 0;
	MPI_File_seek( msFileHdl, totalOffset, MPI_SEEK_SET );

	//uint32_t ndgr = mydefgpool.size();

	uint32_t c = 0; //implicitly fill buffer arranging +x lines and stacked them along +y to ultimatively stack these xy slabs along +z
	for (uint32_t zregion = 0; zregion < npz; zregion++ ) { //utilize that the regions form a regular-stacked 3D region aggregate
		//we want to write only a portion of the data, so we have to check whether these limits are smaller than the region size
		//zper = this->regions[(zregion*npy)+0]->myGeom.nreg_nd; //nreg_td is an extent not a global coordinate in SU!
		//nreg_tdmax is a global coordinate in SU, how does it relate to zmx, the highest z global coordinate to render from SU
		zlim = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax > myrenderwindow.zmx) ? myrenderwindow.zmx : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax;
		zstart = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin < myrenderwindow.zmi) ? myrenderwindow.zmi : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;
		rzmi = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;

		for ( uint32_t zz = zstart; zz <= zlim; zz++ ) { //run over global SU coordinates in the zregion-th region, at one z coordinate however, npy regions are stacked on top of each other,i.e. in +y
			for(uint32_t yregion = 0; yregion < npy; yregion++ ) { //collect data over yregions in the fixed zlayer == zz global SU coordinates
				r = (zregion*npy)+yregion; //in which memory region are we now?

				yper = this->regions[r]->myGeom.nreg_td; //row of zregions have same height in y
				ylim = (this->regions[r]->myGeom.nreg_tdmax > myrenderwindow.ymx) ? myrenderwindow.ymx : this->regions[r]->myGeom.nreg_tdmax;
				ystart  = (this->regions[r]->myGeom.nreg_tdmin < myrenderwindow.ymi) ? myrenderwindow.ymi : this->regions[r]->myGeom.nreg_tdmin;
				rymi = this->regions[r]->myGeom.nreg_tdmin;

				xper = this->regions[r]->myGeom.nreg_rd; //and width in x
				xlim = (this->regions[r]->myGeom.nreg_rdmax > myrenderwindow.xmx) ? myrenderwindow.xmx : this->regions[r]->myGeom.nreg_rdmax; //inclusive global coordinates
				xstart = (this->regions[r]->myGeom.nreg_rdmin < myrenderwindow.xmi) ? myrenderwindow.xmi : this->regions[r]->myGeom.nreg_rdmin; 
				rxmi = this->regions[r]->myGeom.nreg_rdmin;


				for ( uint32_t yy = ystart; yy <= ylim; yy++ ) { //..mi and ..mx are global automaton coordinates!

					for ( uint32_t xx = xstart; xx <= xlim; xx++ ) { //utilize that npx == 1

						//transform into local memory region coordinate
						corr = (xx-rxmi) + (xper*(yy-rymi)) + (xper*yper*(zz-rzmi)); //xper*yper is size of one slab of cells in region r

						gridvalue = regions[r]->mycellgrid[corr];

						buffer[3*c+RED] = BLACK;
						buffer[3*c+GREEN] = BLACK;
						buffer[3*c+BLUE] = BLACK;

						if ( gridvalue == CELL_IS_A_PARTICLE ) {
							c++;
							continue;
						}
						if ( gridvalue == CURRENTLY_INFECTED ) {
							c++;
							continue;
						}
						//other cases already excluded
						if ( gridvalue <= CA_GRAINIDS_MAXIMUM ) {
							//get grainid and color //deformed or rx?
							uint32_t oid = gridvalue < mydefgpool.size() ? mydefgpool.at(gridvalue).caori : myrxgpool.at(gridvalue-mydefgpool.size()).caori;

//cout << "xx/yy/zz/oid/myoripool/mydeg/myrxg = " << xx << ";" << yy << ";" << zz << ";" << oid << "\t" << myoripool.size() << ";" << mydefgpool.size() << ";" << myrxgpool.size() << endl;

							buffer[3*c+RED] = myoripool[oid].RGB_R; //localprng.MersenneTwister() * 255.0; //myoripool[oid].RGB_R;
							buffer[3*c+GREEN] = myoripool[oid].RGB_G; //localprng.MersenneTwister() * 255.0; //myoripool[oid].RGB_G;
							buffer[3*c+BLUE] = myoripool[oid].RGB_B; //localprng.MersenneTwister() * 255.0; //myoripool[oid].RGB_B;

							c++;
							continue;
						}
					} //scan through xline
				} //scan +y the xlines stacked in region r
			} //next region stacked in +yregion upon last one in +y

			//write MPI for each z layer then reset c to collect the next nxy values
			MPI_File_write(msFileHdl, buffer, 3*nxy, MPI_CHAR, &msFileStatus);
			c = 0;

		} //next +zslice in +zregions
	} //next zregion

	MPI_File_close(&msFileHdl);
	delete [] buffer;
	return true;
}

bool caHdl::hdf5_write_voxelgrid_ipfz( string fname, string grpname, string subgrpname )
{
	//generate anew or append to existing file
	std::cout << "Preparing the writing to the HDF5 file..." << endl;

	//region topology how many memory regions in x, y, and z?
	uint32_t npx = this->regions[MASTER]->thePartitioning.nreg_rdx; //1 required
	uint32_t npy = this->regions[MASTER]->thePartitioning.nreg_tdy;
	uint32_t npz = this->regions[MASTER]->thePartitioning.nreg_ndz;
	if ( npx != 1 ) { std::cout << "ERROR::No 3D output because npx != 1!" << std::endl;  }

	struct loginfo_xdmf xmf;
	xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
	xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
	xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;
	uint32_t nxyz = xmf.nx * xmf.ny * xmf.nz;

	//check if total size of the dataset can be written with HDF5 at once keeping in mind the underlying
	//MPII/O which cannot handle writes of >2GB!
	if ( nxyz * (3*sizeof(unsigned char)) > HDF5_MAXIMUM_SINGLE_WRITE ) {
		cout << "ERROR::HDF5 cannot write " << (nxyz * (3*sizeof(unsigned char))) << " Bytes at once!" <<endl; 
		return false;
	}

	unsigned char* buffer = NULL;
	buffer = new unsigned char[3*nxyz];
	QUICKASSERT( buffer != NULL );
	unsigned char* rb = NULL;
	unsigned char* gb = NULL;
	unsigned char* bb = NULL;
	rb = new unsigned char[nxyz];
	gb = new unsigned char[nxyz];
	bb = new unsigned char[nxyz];

	//TARGET FORMAT IS UINT32 FOR PARAVIEW
	//CELL_IS_PARTICLE	BLACK;BLACK;BLACK
	//CELL_IS_INFECTED	BLACK;BLACK;BLACK
	//DEFORMED 			2+mydefgid //because there is defgid = 0
	//RECRYSTALLIZED	3+mydefgid.size()+myrxgid

	//the automaton comprises npz regions along z, x is aligned, and y as well, so simply read x*y slab stack in npy direction and drop such a global z section into rawdata
	//##MK::at the moment a safe but less efficient than an optimized buffering strategy of working sequentially through xy layers along z is performed
	//uint32_t zper;
	uint32_t yper, xper;
	uint32_t zstart, ystart, xstart, zlim, ylim, xlim;
	uint32_t rzmi, rymi, rxmi;
	//..lim are not ..mx!
	uint32_t r;
	uint32_t corr, gridvalue;

	uint32_t c = 0; //implicitly fill buffer arranging +x lines and stacked them along +y to ultimatively stack these xy slabs along +z
	for (uint32_t zregion = 0; zregion < npz; zregion++ ) { //utilize that the regions form a regular-stacked 3D region aggregate
		//we want to write only a portion of the data, so we have to check whether these limits are smaller than the region size
		//zper = this->regions[(zregion*npy)+0]->myGeom.nreg_nd; //is an extent not a global coordinate in SU!
		//nreg_tdmax is a global coordinate in SU, how does it relate to zmx, the highest z global coordinate to render from SU
		zlim = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax > myrenderwindow.zmx) ? myrenderwindow.zmx : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmax;
		zstart = (this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin < myrenderwindow.zmi) ? myrenderwindow.zmi : this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;
		rzmi = this->regions[(zregion*npy)+0]->myGeom.nreg_ndmin;

		for ( uint32_t zz = zstart; zz <= zlim; zz++ ) { //run over global SU coordinates, at one z coordinate however, npy regions are stacked on top of each other,i.e. in +y
			for(uint32_t yregion = 0; yregion < npy; yregion++ ) { //collect data over regions in the zlayer
				r = (zregion*npy)+yregion; //in which memory region are we now?

				yper = this->regions[r]->myGeom.nreg_td;
				ylim = (this->regions[r]->myGeom.nreg_tdmax > myrenderwindow.ymx) ? myrenderwindow.ymx : this->regions[r]->myGeom.nreg_tdmax;
				ystart  = (this->regions[r]->myGeom.nreg_tdmin < myrenderwindow.ymi) ? myrenderwindow.ymi : this->regions[r]->myGeom.nreg_tdmin;
				rymi = this->regions[r]->myGeom.nreg_tdmin;

				xper = this->regions[r]->myGeom.nreg_rd; //and width in x
				xlim = (this->regions[r]->myGeom.nreg_rdmax > myrenderwindow.xmx) ? myrenderwindow.xmx : this->regions[r]->myGeom.nreg_rdmax; //inclusive global coordinates
				xstart = (this->regions[r]->myGeom.nreg_rdmin < myrenderwindow.xmi) ? myrenderwindow.xmi : this->regions[r]->myGeom.nreg_rdmin; 
				rxmi = this->regions[r]->myGeom.nreg_rdmin;

				for ( uint32_t yy = ystart; yy <= ylim; yy++ ) { //..mi and ..mx are global automaton coordinates!
					for ( uint32_t xx = xstart; xx <= xlim; xx++ ) { //utilize that npx == 1

						//transform into local memory region coordinate
						corr = (xx-rxmi) + (xper*(yy-rymi)) + (xper*yper*(zz-rzmi)); //xper*yper is size of one slab of cells in region r

						gridvalue = regions[r]->mycellgrid[corr];

						buffer[3*c+RED] = BLACK;
						buffer[3*c+GREEN] = BLACK;
						buffer[3*c+BLUE] = BLACK;
						rb[c] = BLACK;
						gb[c] = BLACK;
						bb[c] = BLACK;

						if ( gridvalue == CELL_IS_A_PARTICLE ) {
							c++;
							continue;
						}
						if ( gridvalue == CURRENTLY_INFECTED ) {
							c++;
							continue;
						}
						//other cases already excluded
						if ( gridvalue <= CA_GRAINIDS_MAXIMUM ) {
							//get grainid and color //deformed or rx?
							uint32_t oid = gridvalue < mydefgpool.size() ? mydefgpool.at(gridvalue).caori : myrxgpool.at(gridvalue).caori;

//cout << "xx/yy/zz/oid/myoripool/mydeg/myrxg = " << xx << ";" << yy << ";" << zz << ";" << oid << "\t" << myoripool.size() << ";" << mydefgpool.size() << ";" << myrxgpool.size() << endl;

							buffer[3*c+RED] = myoripool[oid].RGB_R;
							buffer[3*c+GREEN] = myoripool[oid].RGB_G;
							buffer[3*c+BLUE] = myoripool[oid].RGB_B;
							rb[c] = myoripool[oid].RGB_R;
							gb[c] = myoripool[oid].RGB_G;
							bb[c] = myoripool[oid].RGB_B;

							c++;
							continue;
						}
					} //scan through xline
				} //scan +y the xlines stacked in region r
			} //next +yregion stacked upon last one in +y
		} //next zslice in zregions
	} //next zregion z with regions on stacked top of one another in y

	std::cout << "Writing to HDF5 file... " << fname << std::endl;

	hid_t file, space, dset, group; //, dcpl;
	herr_t status;

	//file exists?
	//ifstream h5file( fname.c_str() );
	//if(!h5file) { //create
	//	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	//}
	//else { //open readwrite of existing file
		file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );
	//}

	status = H5Eset_auto(NULL, NULL, NULL); //generate group if not existing
	status = H5Gget_objinfo(file, grpname.c_str(), 0, NULL);
	if(status < 0) { //group does not exist or other error, attempt to create it at least
		group = H5Gcreate2( file, grpname.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
		status = H5Gclose(group);
		if ( status < 0 ) { cout << "ERROR::HDF5 unable to create group " << grpname << endl; delete [] buffer; return false; }
	} //else group exists

	//generate subgroup grpname/subgroup
	//group = H5Gcreate2( file, subgrpname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	//status = H5Gclose(group);

	//create the dimensions, space of the dataset and copy data from the SU volume
	hsize_t dims[2] = { xmf.nx*xmf.ny*xmf.nz , 1 }; //{ , 3 };
	space = H5Screate_simple(2, dims, NULL);
	dset = H5Dcreate(file, subgrpname.c_str(), H5T_STD_U8LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); //maps to H5Dcreate2

	status = H5Dwrite(dset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer );
	if ( status < 0 ) { cout << "ERROR::HDF5 unable to write buffer " << endl; delete [] buffer; H5Sclose(space); H5Dclose(dset); H5Fclose(file); return false; }

	status = H5Sclose(space);
	status = H5Dclose(dset);
	//delete [] buffer; buffer = NULL;
	//status = H5Fclose(file);

	string nm = subgrpname + "_r";
	hsize_t dim[2] = {nxyz,1};
	space = H5Screate_simple(2, dim, NULL);
	dset = H5Dcreate(file, nm.c_str(), H5T_STD_U8LE, space, H5P_DEFAULT , H5P_DEFAULT , H5P_DEFAULT );
	status = H5Dwrite(dset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, rb );
	status = H5Sclose(space);
	status = H5Dclose(dset);
	nm.clear();

	nm = subgrpname + "_g";
	space = H5Screate_simple(2, dim, NULL);
	dset = H5Dcreate(file, nm.c_str(), H5T_STD_U8LE, space, H5P_DEFAULT , H5P_DEFAULT , H5P_DEFAULT );
	status = H5Dwrite(dset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, gb );
	status = H5Sclose(space);
	status = H5Dclose(dset);
	nm.clear();

	nm = subgrpname + "_b";
	space = H5Screate_simple(2, dim, NULL);
	dset = H5Dcreate(file, nm.c_str(), H5T_STD_U8LE, space, H5P_DEFAULT , H5P_DEFAULT , H5P_DEFAULT );
	status = H5Dwrite(dset, H5T_NATIVE_UCHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, bb );
	status = H5Sclose(space);
	status = H5Dclose(dset);
	nm.clear();

	delete [] buffer;
	delete [] rb;
	delete [] gb;
	delete [] bb;

	status = H5Fclose(file);

	return true;
}


void caHdl::hdf5_write_xmffile_3dcorectmesh( string fname, string hfname, string gname )
{
	cout << "Writing HDF5 metadata into " << fname << endl;
	ofstream xdmf;
	xdmf.open ( fname.c_str() );

	//write XDMF header
	xdmf <<(char)60<<(char)63<<"xml version"<<(char)61<<(char)34<<"1.0"<<(char)34<< " "<<(char)63<<">\n";
	xdmf <<(char)60<<(char)33<<"DOCTYPE Xdmf SYSTEM "<<(char)34<<"Xdmf.dtd"<<(char)34<<" []>\n";
	xdmf <<(char)60<<"Xdmf xmlns:xi="<<(char)34<<"http:"<<(char)47<<(char)47<<"www.w3.org/2003/XInclude"<<(char)34<<" Version"<<(char)61<<(char)34<<"2.2"<<(char)34<<">\n";
	xdmf <<"\t<Domain>\n";
	
	string prgname = "SCORE_v" + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_BUILD) + "_TimeSeries";
	xdmf <<"\t\t<Grid Name="<<(char)34<<prgname<<(char)34<<" GridType="<<(char)34<<"Collection"<<(char)34<<" CollectionType="<<(char)34<<"Temporal"<<(char)34<<">\n";
	prgname.clear();

	//autogenerate dimensions and link names to heavy data, utilize that the individual snapshots are named gname/step
	prgname = "SCORE_v" + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_BUILD) + "_Result";
	//string corr = hfname + ":" + "/" + "coordinates";
	string grid;
	string dim;
	string attr;

	for ( unsigned int snp = 0; snp < this->renderingxdmf.size(); snp++ ) { //append individual snapshots
		//write always coordinates first
		grid += std::to_string(renderingxdmf.at(snp).nx) + " " + std::to_string(renderingxdmf.at(snp).ny) + " " + std::to_string(renderingxdmf.at(snp).nz);
		dim += std::to_string(renderingxdmf.at(snp).nx*renderingxdmf.at(snp).ny*renderingxdmf.at(snp).nz) + " 3";

		xdmf <<"\t\t\t<Grid Name=" <<(char)34<<prgname<<(char)34<<" GridType="<<(char)34<<"Uniform"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t<Time Type="<<(char)34<<"Single"<<(char)34<<" Value="<<(char)34<<"   "<< setprecision(6) << (double) snp+1<<(char)34<<" />\n";
			xdmf <<"\t\t\t\t<Topology TopologyType="<<(char)34<<"3DCoRectMesh"<<(char)34<<" Dimensions="<<(char)34<<grid<<(char)34<<"></Topology>\n";
			xdmf <<"\t\t\t\t<Geometry Type="<<(char)34<<"Origin_DXDYDZ"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t\t<DataItem Format="<<(char)34<<"XML"<<(char)34<<" Dimensions="<<(char)34<<"3"<<(char)34<<" NumberType="<<(char)34<<"Float"<<(char)34<<" Precision="<<(char)34<<"3"<<(char)34<<">0.0 0.0 0.0</DataItem>\n";
			xdmf <<"\t\t\t\t\t<DataItem Format="<<(char)34<<"XML"<<(char)34<<" Dimensions="<<(char)34<<"3"<<(char)34<<" NumberType="<<(char)34<<"Float"<<(char)34<<" Precision="<<(char)34<<"3"<<(char)34<<">1.0 1.0 1.0</DataItem>\n";
			xdmf <<"\t\t\t\t</Geometry>\n";

			dim.clear();
			dim += std::to_string(renderingxdmf.at(snp).nx*renderingxdmf.at(snp).ny*renderingxdmf.at(snp).nz);
			//if ( renderingxdmf.at(snp).cmodel == RENDERING_COLOR_GRAINID )
			//	attr = hfname + ":" + gname + "/grainid_" + std::to_string(renderingxdmf.at(snp).step);
			//else { //if ( renderingxdmf.at(snp).cmodel == RENDERING_COLOR_IPFZ )
			//	attr = hfname + ":" + gname + "/ipfz_" + std::to_string(renderingxdmf.at(snp).step);
			//}

			if ( renderingxdmf.at(snp).cmodel == RENDERING_COLOR_GRAINID ) {
				attr = hfname + ":" + gname + "/" + std::to_string(renderingxdmf.at(snp).step);

				xdmf <<"\t\t\t\t<Attribute Name="<<(char)34<<"GrainID+2"<<(char)34<<" AttributeType="<<(char)34<<"Scalar"<<(char)34<<" Center="<<(char)34<<"Cell"<<(char)34<<">\n";
				xdmf <<"\t\t\t\t\t<DataItem Dimensions="<<(char)34<<renderingxdmf.at(snp).nx<<" "<<renderingxdmf.at(snp).ny<<" "<<renderingxdmf.at(snp).nz<<(char)34<<" Format="<<(char)34<<"HDF"<<(char)34<<" NumberType="<<(char)34<<"UInt"<<(char)34<<" Precision="<<(char)34<<"4"<<(char)34<<">\n";
				xdmf <<"\t\t\t\t\t"<<attr<<"</DataItem>\n";
				xdmf <<"\t\t\t\t</Attribute>\n";
			}
			if ( renderingxdmf.at(snp).cmodel == RENDERING_COLOR_RHO ) {
				attr = hfname + ":" + gname + "/" + std::to_string(renderingxdmf.at(snp).step);

				xdmf <<"\t\t\t\t<Attribute Name="<<(char)34<<"RhoDensity"<<(char)34<<" AttributeType="<<(char)34<<"Scalar"<<(char)34<<" Center="<<(char)34<<"Cell"<<(char)34<<">\n";
				xdmf <<"\t\t\t\t\t<DataItem Dimensions="<<(char)34<<renderingxdmf.at(snp).nx<<" "<<renderingxdmf.at(snp).ny<<" "<<renderingxdmf.at(snp).nz<<(char)34<<" Format="<<(char)34<<"HDF"<<(char)34<<" NumberType="<<(char)34<<"Float"<<(char)34<<" Precision="<<(char)34<<"8"<<(char)34<<">\n";
				xdmf <<"\t\t\t\t\t"<<attr<<"</DataItem>\n";
				xdmf <<"\t\t\t\t</Attribute>\n";
			}

			//add further attributes with different Attribute Names if desired here...

			dim.clear();
			attr.clear();
			grid.clear();

		xdmf << "\t\t\t</Grid>\n";
	}

	//finalize
	xdmf << "\t\t</Grid>" << endl;
	xdmf << "\t</Domain>\n";
	xdmf << "</Xdmf>";
	xdmf.flush();
	xdmf.close();

	//delete descriptive meta data information to the HDF5 file
	renderingxdmf.clear();
}


void caHdl::raw_write_xmffile_3dcorectmesh( string fname, string binprefix )
{
	cout << "Writing RAW metadata into " << fname << endl;
	ofstream xdmf;
	xdmf.open ( fname.c_str() );

	//write XDMF header
	xdmf <<(char)60<<(char)63<<"xml version"<<(char)61<<(char)34<<"1.0"<<(char)34<< " "<<(char)63<<">\n";
	xdmf <<(char)60<<(char)33<<"DOCTYPE Xdmf SYSTEM "<<(char)34<<"Xdmf.dtd"<<(char)34<<" []>\n";
	xdmf <<(char)60<<"Xdmf xmlns:xi="<<(char)34<<"http:"<<(char)47<<(char)47<<"www.w3.org/2003/XInclude"<<(char)34<<" Version"<<(char)61<<(char)34<<"2.2"<<(char)34<<">\n";
	xdmf <<"\t<Domain>\n";
	
	string prgname = "SCORE_v" + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_REVISION) + "_TimeSeries";
	xdmf <<"\t\t<Grid Name="<<(char)34<<prgname<<(char)34<<" GridType="<<(char)34<<"Collection"<<(char)34<<" CollectionType="<<(char)34<<"Temporal"<<(char)34<<">\n";
	prgname.clear();

	//autogenerate dimensions and link names to heavy data, utilize that the individual snapshots are named gname/step
	prgname = "SCORE_v" + std::to_string(VERSION_MAJOR) + "." + std::to_string(VERSION_MINOR) + "." + std::to_string(VERSION_REVISION) + "_Result";
	string grid;
	string dim;
	string attr;

	for ( unsigned int snp = 0; snp < this->renderingxdmf.size(); snp++ ) { //append individual snapshots
		//write always coordinates first
		grid += std::to_string(renderingxdmf.at(snp).nx) + " " + std::to_string(renderingxdmf.at(snp).ny) + " " + std::to_string(renderingxdmf.at(snp).nz);
		dim += std::to_string(renderingxdmf.at(snp).nx*renderingxdmf.at(snp).ny*renderingxdmf.at(snp).nz) + " 3";

		xdmf <<"\t\t\t<Grid Name=" <<(char)34<<prgname<<(char)34<<" GridType="<<(char)34<<"Uniform"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t<Time Type="<<(char)34<<"Single"<<(char)34<<" Value="<<(char)34<<"   "<< setprecision(6) << (double) snp+1<<(char)34<<" />\n";
			xdmf <<"\t\t\t\t<Topology TopologyType="<<(char)34<<"3DCoRectMesh"<<(char)34<<" Dimensions="<<(char)34<<grid<<(char)34<<"></Topology>\n";
			xdmf <<"\t\t\t\t<Geometry Type="<<(char)34<<"Origin_DXDYDZ"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t\t<DataItem Format="<<(char)34<<"XML"<<(char)34<<" Dimensions="<<(char)34<<"3"<<(char)34<<" NumberType="<<(char)34<<"Float"<<(char)34<<" Precision="<<(char)34<<"3"<<(char)34<<">0.0 0.0 0.0</DataItem>\n";
			xdmf <<"\t\t\t\t\t<DataItem Format="<<(char)34<<"XML"<<(char)34<<" Dimensions="<<(char)34<<"3"<<(char)34<<" NumberType="<<(char)34<<"Float"<<(char)34<<" Precision="<<(char)34<<"3"<<(char)34<<">1.0 1.0 1.0</DataItem>\n";
			xdmf <<"\t\t\t\t</Geometry>\n";

			dim.clear();
			dim += std::to_string(renderingxdmf.at(snp).nx*renderingxdmf.at(snp).ny*renderingxdmf.at(snp).nz);
			attr = binprefix + std::to_string(renderingxdmf.at(snp).step) + ".raw";

			xdmf <<"\t\t\t\t<Attribute Name="<<(char)34<<"IPFZ"<<(char)34<<" AttributeType="<<(char)34<<"Vector"<<(char)34<<" Center="<<(char)34<<"Cell"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t\t<DataItem Dimensions="<<(char)34<<renderingxdmf.at(snp).nx<<" "<<renderingxdmf.at(snp).ny<<" "<<renderingxdmf.at(snp).nz<<" 3"<<(char)34<<" Format="<<(char)34<<"Binary"<<(char)34<<" NumberType="<<(char)34<<"UChar"<<(char)34<<" Precision="<<(char)34<<"1"<<(char)34<<">\n";
			xdmf <<"\t\t\t\t\t"<<attr<<"</DataItem>\n";
			xdmf <<"\t\t\t\t</Attribute>\n";

			//add further attributes with different Attribute Names if desired here...

			dim.clear();
			attr.clear();
			grid.clear();

		xdmf << "\t\t\t</Grid>\n";
	}

	//finalize
	xdmf << "\t\t</Grid>" << endl;
	xdmf << "\t</Domain>\n";
	xdmf << "</Xdmf>";
	xdmf.flush();
	xdmf.close();

	//delete descriptive meta data information to the HDF5 file
	renderingxdmf.clear();
}


void caHdl::ompcrit_write_voxeldata_h5( unsigned char colormodel, string grpnm, bool xdmf, string xmfsuffix, double now, double xfraction, unsigned int stp )
{
	//CALLED FROM WITHIN PARALLEL REGION
	string h5fname = "SCORE." + std::to_string( this->myensHdl->simid ) + "." + "VoxelData.h5";
	string sbgrpn;
	if ( colormodel == RENDERING_COLOR_GRAINID )
		sbgrpn = grpnm + "/grainid_" + std::to_string(stp);
	else if ( colormodel == RENDERING_COLOR_IPFZ )
		sbgrpn = grpnm + "/ipfz_" + std::to_string(stp);
	else if ( colormodel == RENDERING_COLOR_RHO ) 
		sbgrpn = grpnm + "/rho_" + std::to_string(stp);
	else
		return;

	bool h5status = true;
	omp_set_lock(&h5lock);
	if ( colormodel == RENDERING_COLOR_GRAINID )
		h5status = hdf5_write_voxelgrid_grainid( h5fname, grpnm, sbgrpn );
	else if ( colormodel == RENDERING_COLOR_IPFZ ) {
		//h5status = hdf5_write_voxelgrid_ipfz( h5fname, grpnm, sbgrpn );
		string binfname = "SCORE." + std::to_string(myensHdl->simid) + ".VoxelData.Step." + std::to_string(renderingxdmf.size()) + ".raw"; //std::to_string(stp) + ".raw";
		bool status = mpiio_write_voxelgrid_ipfz( binfname );
	}
	else if ( colormodel == RENDERING_COLOR_RHO )
		h5status = hdf5_write_voxelgrid_rho( h5fname, grpnm, sbgrpn );
	else
		cout << "ERROR::Unsupported color model!" << endl;

	omp_unset_lock(&h5lock);

	//Metadata management
	if ( h5status == true ) {
		struct loginfo_xdmf xmf;
		xmf.time = now;
		xmf.X = xfraction;

		xmf.nx = myrenderwindow.xmx - myrenderwindow.xmi + 1;
		xmf.ny = myrenderwindow.ymx - myrenderwindow.ymi + 1;
		xmf.nz = myrenderwindow.zmx - myrenderwindow.zmi + 1;

		if (colormodel == RENDERING_COLOR_GRAINID ) {
			xmf.step = stp;
			xmf.cmodel = RENDERING_COLOR_GRAINID;
		}
		else if ( colormodel == RENDERING_COLOR_RHO ) {
			xmf.step = stp;
			xmf.cmodel = RENDERING_COLOR_RHO;
		}
		else {
			xmf.step = renderingxdmf.size();
			xmf.cmodel = RENDERING_COLOR_IPFZ;
		}

		renderingxdmf.push_back( xmf );

		//finalize xmf file? write metadata file and clear renderingxdmf
		if ( xdmf == true ) {
			if ( renderingxdmf.size() > 0 ) {
				string xmffname = "SCORE." + std::to_string( this->myensHdl->simid ) + ".VoxelData.xdmf";

				if ( colormodel == RENDERING_COLOR_GRAINID || colormodel == RENDERING_COLOR_RHO ) {
					hdf5_write_xmffile_3dcorectmesh( xmffname, h5fname, grpnm );
					return;
				}
				if ( colormodel == RENDERING_COLOR_IPFZ ) {
					string binprfx = "SCORE." + std::to_string( this->myensHdl->simid ) + ".VoxelData.Step.";
					raw_write_xmffile_3dcorectmesh( xmffname, binprfx );
					return;
				}
			}
			else {
				cout << "ERROR::No XDMF snapshot data to write!" << endl;
			}
		}
	} //end metadatahandling
}

#define DIM0			4
#define DIM1			7
void caHdl::h5_return_dislocationdensity( float* buffer, uint32_t buffersize )
{
	//fills the buffer with the dislocation density
	for ( uint32_t c = 0; c < buffersize; c++ ) {
		buffer[c] = 0.0;
	}

	uint32_t nthreads = this->regions.size();
	for ( uint32_t thr = 0; thr < nthreads; thr++ ) {
		uint32_t xmi = this->regions[thr]->myGeom.nreg_rdmin; //global SU coordinates all inclusive [imi, imx]
		uint32_t ymi = this->regions[thr]->myGeom.nreg_tdmin;
		uint32_t zmi = this->regions[thr]->myGeom.nreg_ndmin;

		uint32_t rx = this->regions[thr]->myGeom.nreg_rd; //local coordinates [0, ri) so ri exclusive
		uint32_t ry = this->regions[thr]->myGeom.nreg_td;
		uint32_t rz = this->regions[thr]->myGeom.nreg_nd;
		uint32_t rxy = rx*ry;

		uint32_t gx = this->myCAGeometry.nboxedge_rd;
		uint32_t gxy = this->myCAGeometry.nboxarea_rdtd;

		//copy dislocation densities from the buffer
		uint32_t* thegrid = this->regions[thr]->mycellgrid;
		uint32_t c = 0;
		for ( uint32_t z = 0; z < rz; z++ ) {
			uint32_t zroff = z*rxy;
			uint32_t zwoff = (z+zmi)*gxy;
			for ( uint32_t y = 0; y < ry; y++ ) {
				uint32_t yzroff = y*rx + zroff;
				uint32_t xyzwoff = xmi+(y+ymi)*gx + zwoff;
				for ( uint32_t x = 0; x < rx; x++ ) {
					c = thegrid[x+yzroff]; //read cell index
					if ( c == CURRENTLY_INFECTED || c >= this->regions[thr]->reg_nmydefgpool ) { //recrystallized
						buffer[x+xyzwoff] = RHO_RECRYSTALLIZED_MATERIAL;
//cout << "Thr/x/y/z/xmi/ymi/zmi/buffer\t\t" << thr << ";" << x << ";" << y << ";" << z << ";" << xmi << ";" << ymi << ";" << zmi << ";" << buffer[x+xyzwoff] << endl;
//if ( buffer[x+xyzwoff] < 1.0 ) cout << "c/reg" << c << ";" << this->regions[thr]->reg_nmydefgpool << endl;
						continue;
					}
					//still deformed
					buffer[x+xyzwoff] = get_rho( c ); //conversion to float has accuracy issues but should be sufficient for our pu
//cout << "Thr/x/y/z/xmi/ymi/zmi/buffer\t\t" << thr << ";" << x << ";" << y << ";" << z << ";" << xmi << ";" << ymi << ";" << zmi << ";" << buffer[x+xyzwoff] << endl;
//if ( buffer[x+xyzwoff] < 1.0 ) cout << "c/reg" << c << ";" << this->regions[thr]->reg_nmydefgpool << endl;
				}
			}
		}
	} // next thread region
}

void caHdl::hdf5_dummy( string fnsuffix )
{
	//https:\//support.hdfgroup.org/HDF5/examples/api18-c.html
	hid_t file, space, dset, group; //, dcpl;
	herr_t status;

	//create a new file using the default properties
	string fname = "SCORE." + std::to_string(this->myensHdl->simid) + "." + fnsuffix + ".h5";
std::cout << "Writing a HDF5 file " << fname << std::endl;

	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

	//create DAMASK dataset groups in the file
	//for further references see M. Diehl, P. Eisenlohr, C. Zhang, J. Nastola, P. Shanthraj and F. Roters:
	//"A flexible and efficient output file format for grain scale multiphysics simulations"
	//Integrating Materials and Manufacturing Innovation 2016

	//generate group
	group = H5Gcreate2( file, "/geometry", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose(group);

	group = H5Gcreate2( file, "/mapping", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose(group);

	group = H5Gcreate2( file, "/mapping/cells", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose(group);

	group = H5Gcreate2( file, "/mapping/cells/constitutive", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose(group);

	group = H5Gcreate2( file, "/mapping/cells/constitutive/plasticity", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose( group );

	uint32_t nxyz = this->myCAGeometry.nboxvol_rdtdnd;

	for ( int slipsystem = 0; slipsystem < 1; slipsystem++ ) {
		string grpname = "/mapping/cells/constitutive/plasticity/" + std::to_string(slipsystem);

		//create the dimensions, space of the dataset and copy data from the SU volume
		hsize_t dims[2] = { nxyz, 1 };

		float* rho = NULL;
		rho = new float[nxyz];
		QUICKASSERT( rho != NULL );

		h5_return_dislocationdensity( rho, nxyz );

		space = H5Screate_simple(2, dims, NULL);

		//create the dataset with default properties
		dset = H5Dcreate(file, grpname.c_str(), H5T_IEEE_F32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

		//write dataset
		status = H5Dwrite(dset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rho );

		status = H5Sclose (space);
		status = H5Dclose (dset);

		delete [] rho; rho = NULL;
	}

	status = H5Fclose(file);

	/*totalview
	hsize_t dims[2] = {DIM0, DIM1};
	int wdata[DIM0][DIM1];
	hsize_t i, j;

	for (i=0; i<DIM0; i++)
		for (j=0; j<DIM1; j++)
			wdata[i][j] = i * j - j;

	//create dataspace setting maximum size to NULL sets the maximum size to be the current size
	space = H5Screate_simple(2, dims, NULL);

	//create the dataset with default properties
	dset = H5Dcreate(file, "dummy", H5T_STD_I32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	//write dataset
	status = H5Dwrite(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata[0]);

	//clean-up
	status = H5Dclose(dset);
	status = H5Sclose(space);
	status = H5Fclose(file);
	*/
}

void caHdl::write_damask_geom_h5( string suffix )
{
	//###implement
	//CAllED FROM WITHIN A PARALLEL REGION
	omp_set_lock(&h5lock);

	hdf5_dummy( suffix );

	omp_unset_lock(&h5lock);
}


void caHdl::write_clustersizedistr_h5_create( void )
{
	//A HDF5 file must be created before one can sucessively add groups of data into it via reopening

	//omp_set_lock(&h5lock);

	hid_t file; //, space, dset, group, dcpl;
	herr_t status;

	//create a new file or overwrite data if they exist in every case using the default HDF5 properties
	string fname = "SCORE." + std::to_string(this->myensHdl->simid) + ".PercClusterSizeDistr.h5";
	file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Fclose(file);

	//omp_unset_lock(&h5lock);
}

 
void caHdl::write_clustersizedistr_h5( unsigned int* dist, unsigned int ndist )
{
	//###implement
	//default file name SCORE.<simid>.RXClusterSizeDistr.Step.<step>
	//https:\//support.hdfgroup.org/HDF5/examples/api18-c.html
	omp_set_lock(&h5lock);

	hid_t file, space, dset, group; // dcpl;
	herr_t status;

	//create a new file or overwrite data if they exist in every case using the default HDF5 properties
	string fname = "SCORE." + std::to_string(this->myensHdl->simid) + ".PercClusterSizeDistr.h5";
	file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT );

	//for each size distribution create DAMASK dataset groups in the file
	//for further references see M. Diehl, P. Eisenlohr, C. Zhang, J. Nastola, P. Shanthraj and F. Roters:
	//"A flexible and efficient output file format for grain scale multiphysics simulations"
	//Integrating Materials and Manufacturing Innovation 2016

	//generate group of data for that integration step
	string grpname = "/inc_" + std::to_string(this->step);
	group = H5Gcreate2( file, grpname.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
	status = H5Gclose(group);

	string entry = grpname + "/" + "cluster";
	hsize_t dims[2] = { ndist, 1 };
	space = H5Screate_simple(2, dims, NULL);
	dset = H5Dcreate(file, entry.c_str() , H5T_STD_U32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

	status = H5Dwrite(dset, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL, H5P_DEFAULT, dist );

	status = H5Sclose (space);
	status = H5Dclose (dset);
	status = H5Fclose(file);

	omp_unset_lock(&h5lock);
}

bool caHdl::hdf5_read_DAMASK(){
        hid_t file;
        hid_t group_ID;
        hid_t attr_id;
        herr_t ret;
        double *float_array;
        int *int_array;

        string fname = "/home/georg.falkinger/Rolling_partial_RX/CA_example/simulation/postProc/CA_example_tensionX_increment1000_segmented.dream3D";
	file = HDF5_openFile( fname );
        group_ID = HDF5_openGroup( file, "geometry");
        attr_id  = HDF5_openAttribute( group_ID, "size");
        float_array = HDF5_read_attribute_float(attr_id);
        attr_id  = HDF5_openAttribute( group_ID, "cells");
        int_array = HDF5_read_attribute_int(attr_id);
        cout << file;
	if (file > 0) {std::cout << "file open successful -------------" << endl;}
        cout << group_ID;
	if (group_ID > 0) {std::cout << "group open successful -------------" << endl;}
        cout << attr_id;
	if (attr_id > 0) {std::cout << "attribute open successful -------------" << endl;}
        cout << ret;
	if (attr_id > 0) {std::cout << "no error open successful -------------" << endl;}
        
        return true;
}

hid_t caHdl::HDF5_openFile( string fname ){
// the files can be opened in parallel however as parallelization architecture is not known, only simple file opening is done
       hid_t file;
       
       file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT); // simplest file opening
       // can be improved further to open only when file exists etc (like in DAMASK)
       return file;

}

hid_t caHdl::HDF5_openGroup( hid_t loc_id, string groupname ){

        hid_t group_ID;
        
        group_ID = H5Gopen( loc_id, groupname.c_str(), H5P_DEFAULT );
        return group_ID;
}

hid_t caHdl::HDF5_createGroup(hid_t loc_id, string groupname){

// check if group exists and create if needed
        hid_t group_ID;

        group_ID = H5Gopen(loc_id, groupname);
        if (group_ID < 0){
        std::cout << "Error group needs to be created" << group_ID << std::endl;
        }
}

hid_t caHdl::HDF5_openAttribute( hid_t obj_id, string attribute_name ){
        hid_t attr_id;
       
        attr_id = H5Aopen( obj_id, attribute_name.c_str(), H5P_DEFAULT);
        return attr_id;
}

double* caHdl::HDF5_read_attribute_float(hid_t attr_id){
       hid_t mem_type_id, aspace;
       herr_t ret;
       int i;
       double *float_array;
       size_t npoints;
       
       mem_type_id = H5Aget_type( attr_id );

       // assuming a float array as attribute
       // reference for code:  https://support.hdfgroup.org/ftp/HDF5/examples/misc-examples/attrs.c

       aspace = H5Aget_space( attr_id );
       npoints = H5Sget_simple_extent_npoints( aspace );

       printf("The array size is ---- %d",(int)npoints);

       if (H5T_FLOAT == H5Tget_class(mem_type_id)){
          printf("Type: FLOAT \n");
       }

       float_array = (double *)malloc(sizeof(double)*(int)npoints);
       ret = H5Aread( attr_id, mem_type_id, float_array);
       for( i = 0; i < (int)npoints; i++) printf("%f \n", float_array[i]);
       return float_array;
}

//int* caHdl::HDF5_read_dataset_int_rank_1(hid_t loc_id, string datasetname ){
std::vector<int> caHdl::HDF5_read_dataset_int_rank_1(hid_t loc_id, string datasetname ){
       hid_t mem_type_id, aspace, dset_id, dapl_id;
       herr_t ret;
       int i, ndims;
       double *float_array;
       int *rdata;
       size_t npoints;
       hsize_t adims[1] = {0}, 
               dims[1] = {0};
       
       dset_id = H5Dopen( loc_id, datasetname.c_str(),H5P_DEFAULT );
       mem_type_id = H5Dget_type( dset_id );

       aspace = H5Dget_space( dset_id );

       ndims = H5Sget_simple_extent_dims( aspace, dims, NULL );
       npoints = H5Sget_simple_extent_npoints( aspace );

       //rdata = (int *)malloc(sizeof(int)*(int)dims[0]);
       //ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);
       std::vector<int> rdata_vec(dims[0]);
       ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(rdata_vec.data()));

       //for( i = 0; i < (int)dims[0]; i++) printf("%d \n", rdata[i]);
       for( i = 0; i < (int)dims[0]; i++) printf("%d \n", rdata_vec[i]);
       //return rdata;
       return rdata_vec;
}

std::vector<std::vector<int>> caHdl::HDF5_read_dataset_int_rank_2(hid_t loc_id, string datasetname ){
//int* caHdl::HDF5_read_dataset_int_rank_2(hid_t loc_id, string datasetname ){
       hid_t mem_type_id, aspace, dset_id, dapl_id;
       herr_t ret;
       int i, j, ndims;
       size_t npoints;
       hsize_t dims[2] = {0,0};
       
       dset_id = H5Dopen( loc_id, datasetname.c_str(),H5P_DEFAULT );
       mem_type_id = H5Dget_type( dset_id );

       aspace = H5Dget_space( dset_id );

       ndims = H5Sget_simple_extent_dims( aspace, dims, NULL );
       npoints = H5Sget_simple_extent_npoints( aspace );

       int rdata[dims[0]][dims[1]];
       int* rdata_1D;
       rdata_1D = (int *)malloc(sizeof(int)*(int)dims[0]*dims[1]);
       std::vector<std::vector<int>> rdata_vec(dims[0], std::vector<int>(dims[1]));
       //ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(rdata_vec.data()));
       ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

       for(i = 0; i < (int)dims[0]; i++){
          for( j = 0; j < (int)dims[1]; j++){
              printf("%d \n", rdata[i][j]);
              //rdata_1D[i*dims[0] + j] = rdata[i][j];
              rdata_vec.at(i).at(j) = rdata[i][j];
              //printf("%d \n", rdata_vec.at(i).at(j));
          }
       }

       //return reinterpret_cast<int *>(rdata);
       //return rdata_1D;
       return rdata_vec;
}

std::vector<std::vector<double>> caHdl::HDF5_read_dataset_float_rank_2(hid_t loc_id, string datasetname ){ 
//void caHdl::HDF5_read_dataset_float_rank_2(hid_t loc_id, string datasetname ){
       hid_t mem_type_id, aspace, dset_id, dapl_id;
       herr_t ret;
       int i, j, ndims;
       double *float_array;
       size_t npoints;
       hsize_t dims[2] = {0,0};
       
       dset_id = H5Dopen( loc_id, datasetname.c_str(),H5P_DEFAULT );
       mem_type_id = H5Dget_type( dset_id );

       //ndims = H5Tget_array_dims( mem_type_id, adims );

       // assuming a float array as attribute
       // reference for code:  https://support.hdfgroup.org/ftp/HDF5/examples/misc-examples/attrs.c

       aspace = H5Dget_space( dset_id );

       ndims = H5Sget_simple_extent_dims( aspace, dims, NULL );
       npoints = H5Sget_simple_extent_npoints( aspace );

       double rdata[dims[0]][dims[1]];
       std::vector<std::vector<double>> rdata_vec(dims[0], std::vector<double>(dims[1]));;
       ret = H5Dread( dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

       for(i = 0; i < (int)dims[0]; i++){
          for( j = 0; j < (int)dims[1]; j++){
              //printf("%f \n", rdata[i][j]);
              rdata_vec.at(i).at(j) = rdata[i][j];
          }
       }

       //return rdata;
       return rdata_vec;
}

std::vector<int> caHdl::HDF5_read_dataset_int_rank_4(hid_t loc_id, string datasetname ){
//int* caHdl::HDF5_read_dataset_int_rank_2(hid_t loc_id, string datasetname ){
       hid_t mem_type_id, aspace, dset_id, dapl_id;
       herr_t ret;
       int i, j, k, l, index, ndims;
       size_t npoints;
       hsize_t dims[4] = {0,0,0,0};
       
       dset_id = H5Dopen( loc_id, datasetname.c_str(),H5P_DEFAULT );
       mem_type_id = H5Dget_type( dset_id );

       aspace = H5Dget_space( dset_id );

       ndims = H5Sget_simple_extent_dims( aspace, dims, NULL );
       npoints = H5Sget_simple_extent_npoints( aspace );

       int rdata[dims[0]][dims[1]][dims[2]][dims[3]];
       std::vector<int> rdata_vec(dims[0]*dims[1]*dims[2]*dims[3]);
       //ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, static_cast<void*>(rdata_vec.data()));
       ret = H5Dread( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, rdata);

       for(i = 0; i < (int)dims[0]; i++){
          for( j = 0; j < (int)dims[1]; j++){
             for( k = 0; k < (int)dims[2]; k++){
                   //printf("%d \n", rdata[i][j][k][0]);
                   //rdata_1D[i*dims[0] + j] = rdata[i][j];
                   index = k + j*dims[2] + i*dims[2]*dims[1];
                   rdata_vec.at(index) = rdata[i][j][k][0] - 1;  //grain ID for CA starts from 0, DAMASK is flexible about this and doesnt care
                   //printf("%d \n", rdata_vec.at(i).at(j));
          }
       }
       }

       //return reinterpret_cast<int *>(rdata);
       //return rdata_1D;
       return rdata_vec;
}

int* caHdl::HDF5_read_attribute_int(hid_t attr_id){
       hid_t mem_type_id, aspace;
       herr_t ret;
       int i;
       int *int_array;
       size_t npoints;
       
       mem_type_id = H5Aget_type( attr_id );

       // assuming a float array as attribute
       // reference for code:  https://support.hdfgroup.org/ftp/HDF5/examples/misc-examples/attrs.c

       aspace = H5Aget_space( attr_id );
       npoints = H5Sget_simple_extent_npoints( aspace );

       printf("The array size is ---- %d",(int)npoints);

       if (H5T_INTEGER == H5Tget_class(mem_type_id)){
          printf("Type: Integer \n");
       }

       int_array = (int *)malloc(sizeof(int)*(int)npoints);
       ret = H5Aread( attr_id, mem_type_id, int_array);
       for( i = 0; i < (int)npoints; i++) printf("%d \n", int_array[i]);
       return int_array;
}

void caHdl::HDF5_create_CA_output( string fname ){
     hid_t file;
     
     file = H5Fcreate( fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);   
     H5Fclose( file );
}

void caHdl::HDF5_write_CA_output_int( string fname, double t, std::vector<int> data){
     hid_t file, dset_id, space_id;
     string s = to_string(t);
     hsize_t dims[1] = {data.size()};
     herr_t status;

     string dset_name = "Time_" + s;

     file = H5Fopen( fname.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);

     space_id = H5Screate_simple( 1, dims, NULL );
    
     int* wdata = &data[0];
     dset_id = H5Dcreate( file, dset_name.c_str(), H5T_NATIVE_INT, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );          
     status  = H5Dwrite( dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, wdata);
     
     H5Dclose( dset_id );
     H5Sclose( space_id );
     H5Fclose( file );

}

#endif
