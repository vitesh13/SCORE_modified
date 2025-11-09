// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef __SCORE_MPIIOTYPES_H_INCLUDED__
#define __SCORE_MPIIOTYPES_H_INCLUDED__

#include <mpi.h>

typedef struct
{
	long JobID;
	long Iterations;
	double tend;
	double MPIWTimeInitialization;
	double MPIWTimeSpendDefMS;
	double MPIWTimeSpendGBDetection;
	double MPIWTimeSpendNucleation;
	double MPIWTimeSpendGrowthSim;
	double MPIWTimeSpendFinalIO;
} MPI_IO_CAProfilingInfoData;

//##MK::further optimization necessary
typedef struct
{
	unsigned int nx;
	unsigned int ny;
	unsigned int nz;
	unsigned int nxyz;

	unsigned int regx;		//threaded execution
	unsigned int regy;
	unsigned int regz;
	unsigned int nboundarycells;

	unsigned int ndgrseeds;
	unsigned int nrxgrseeds;
	unsigned int ndefmicrotexture;
	unsigned int nnucleimicrotexture;

	double storedenergy;
} MPI_IO_CAPhysicsInfoData;


typedef struct
{
	double finalvol;
	double tincub;
	unsigned int ideal;
} MPI_IO_FinalGSDInfoData;


typedef struct
{
	double finalvol;
	double tincub;
	int ideal;
	int rank;
} MPI_IO_FGSDComplete;

#endif
