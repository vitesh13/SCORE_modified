// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __SCORE_KERNEL_H_INCLUDED__
#define __SCORE_KERNEL_H_INCLUDED__


//#include "SCORE_Defs_Functionality.h"
//#include "SCORE_Defs_Physics.h"
//#include "SCORE_MPIIOTypes.h"
//#include "SCORE_Io.h"
//#include "SCORE_Math.h"
#include "SCORE_PercAnalyzer.h"

//#include <vector>
//#include <algorithm>
//#include <iomanip>
//#include <limits>

#include <omp.h>

#include "hdf5.h"

//#include "thirdparty/mandatory/lodepng/lodepng.h"
//##MK::in case of local installation of HDF5 library
//#include "thirdparty/hdf5-1.10.0-patch1/hdf5/include/hdf5.h"

//NeXus/HDF5 config files as the new default
#define PARSE_CONFIG_FROM_HDF
//do not define this, implementation not complete
//#define PARSE_CONFIG_FROM_YML
//leave only this defined for legacy IMM UDS file formats
//#define PARSE_CONFIG_FROM_UDS

string replace_all_occurrences( string str, const char search, const char replacement );

using namespace std;


class caHdl;


struct loginfo_xdmf
{
	double time;				//collecting of snapshot to write an XDMF file
	double X;
	uint32_t step;
	uint32_t nx;
	uint32_t ny;
	uint32_t nz;
	unsigned char cmodel;
	loginfo_xdmf() : time(0.0),X(0.0),step(0.0),nx(0),ny(0),nz(0),cmodel(RENDERING_COLOR_UNKNOWN) {}
};
typedef struct loginfo_xdmf* loginfo_xdmfP;

struct aabb
{
	double xmin;		//rendering window
	double xmax;
	double ymin;
	double ymax;
	double zmin;
	double zmax;
	uint32_t xmi;		//inclusive SU coordinates to render
	uint32_t xmx;
	uint32_t ymi;
	uint32_t ymx;
	uint32_t zmi;
	uint32_t zmx;
	aabb() : xmin(0.0),xmax(1.0),ymin(0.0),ymax(1.0),zmin(0.0),zmax(1.0),xmi(0),xmx(0),ymi(0),ymx(0),zmi(0),zmx(0) {}
};
typedef struct aabb * aabbP;


//the dynamic heap object that stores a cell represented deformed matter
struct defcell
{
	bool activity;
	unsigned char infector;
	short ix;
	short iy;
	short iz;
	double frac;
	//P and myrxgid is not necessary as mobility in the microstructure generator is always vmax and defgseeds grows into VACUUM
	uint32_t mydefgseedid;	//the grain that becomes synthetized
	defcell() : activity(false), infector(26), ix(0), iy(0), iz(0), frac(NO_INFECTION), mydefgseedid(NO_GRAIN_ASSIGNED) {}
};
typedef struct defcell * defcellP;


//the dynamic heap object that stores the active cells
struct cell
{
	bool activity;
	unsigned char infector;
	short ix;
	short iy;
	short iz;
	double rxFrac;
	double P;
	//##MK::in order to safe memory it could be worthwhile to discretize P and rxFrac as int
	uint32_t mydefgid;
	uint32_t myrxgid;
	cell() : activity(INACTIVE), infector(26), ix(-1), iy(-1), iz(-1), rxFrac(NO_INFECTION), P(-1.0), mydefgid(NO_GRAIN_ASSIGNED), myrxgid(NO_GRAIN_ASSIGNED) {}
};
typedef struct cell * cellP;


//the heap object that stores infections in halo region which can be questioned by the neighboring regions
struct halocell
{
	unsigned char status;
	unsigned char infector;
	short ix;
	short iy;
	short iz;
	double rxFrac;
	//double P; not necessary is calculated a new once neighboring node interprets the halocell
	//uint32_t mydefgid; not necessary local region does not know infection status of mycellgrid that is in another region
	uint32_t myrxgid;
	halocell() : status(HALOCELL_NEVER_VISITED), infector(26), ix(-1), iy(-1), iz(-1), rxFrac(NO_INFECTION), myrxgid(NO_GRAIN_ASSIGNED) {}
};
typedef struct halocell * halocellP;


struct cellstatus
{
	uint32_t ixyz;			//coordinate in global SU coordinates
	uint32_t gid;			//mydefgid if rxFrac < threshold, mydefgpool.size()+myrxgid otherwise
	cellstatus() : ixyz(INVALID_CELLASSIGNMENT), gid(I_DONT_KNOW_YET) {}
};
typedef struct cellstatus * cellstatusP;


struct halo
{
	uint32_t extendx;
	uint32_t extendy;
	uint32_t extendz;
	uint32_t extendxy;
	uint32_t extendxyz;

	uint32_t originx;
	uint32_t originy;
	uint32_t originz;

	uint32_t inregion;											//references the thread region ID in which the locations assigned to the halo are located

//the idea of the HaloReference is like with myFullRXList it guides which positions in the fixed have in the current timestep
//been infected new and require the thread that handles the region <inregion> where to read new infections and synchronize
//locally with the respective mycellgrid in that region
	uint32_t nextFreeHaloRefs;
	uint32_t* theHaloRefs;

	halocell* theHaloCells;
	halo() : extendx(0), extendy(0), extendz(0), extendxy(0), extendxyz(0), originx(0), originy(0), originz(0), inregion(0), nextFreeHaloRefs(0), theHaloRefs(NULL), theHaloCells(NULL) {}
};
typedef struct halo * haloP;


//the recrystallized grain
struct rxg														//rxg - recrystallizing grains
{
	uint32_t ori;												//an ori-index from the oripool
	double tincub;												//an incubation time in seconds after which the nucleus starts growing
	rxg() : ori(0), tincub(0.0) {}
};
typedef struct rxg * rxgP;


//a deformed grain that can be consumed
struct defg														//defg - deformed grains
{
	uint32_t ori;												//an ori index from the oripool
	double rho0;												//initial dislocation density
	double rho;													//instantaneous dislocation density

	//...add further properties if desired, like the following which are not utilized at the moment...
	defg() : ori(0), rho0(0.0), rho(0.0) {}
};
typedef struct defg * defgP;


//a crystallographic orientation
struct ori
{
	double bunge1;												//Bunge (3,1,3) convention Euler angles
	double bunge2;
	double bunge3;

	double q0;													//unit quaternion representing this orientation
	double q1;
	double q2;
	double q3;

	uint32_t closestideal;										//the discrete closest orientation to which the grain is associated default is RANDOM_ORIENTATION
	unsigned char RGB_R;										//a color/alpha channel information in accord with the RGB color model
	unsigned char RGB_G;
	unsigned char RGB_B;
	unsigned char RGB_A;
	ori() : bunge1(0.0), bunge2(0.0), bunge3(0.0), q0(1.0), q1(0.0), q2(0.0), q3(0.0), closestideal(RANDOM_ORIENTATION), RGB_R(UCHAR_RANGE_MAX), RGB_G(UCHAR_RANGE_MAX), RGB_B(UCHAR_RANGE_MAX), RGB_A(UCHAR_RANGE_MAX) {}
};
typedef struct ori * oriP;


//a crystalsymmetry operator in quaternion notation
struct qsymm_fcc
{
	double q0;													//unit quaternion representing this orientation
	double q1;
	double q2;
	double q3;
	qsymm_fcc() : q0(1.0), q1(0.0), q2(0.0), q3(0.0) {}
};
typedef struct qsymm_fcc * qsymm_fccP;


struct cadefg
{
	uint32_t worlddefgid;										//where in my ensHdl I can find this grain
	uint32_t caori;												//ori index in caHdl local myoripool
	uint32_t cellcount;											//number of cells comprising this grain
	double rho0;
	double rho;
	//double dav0;
	//double dav;
	//double avdg0;
	//double avdg;
	cadefg() : worlddefgid(0), caori(0), cellcount(0), rho0(0.0), rho(0.0) {}
};
typedef struct cadefg * cadefgP;


struct dgopt
{
	double rho;
	uint32_t id;
	uint32_t cnt;
	dgopt() : rho(RHOMAX_WELLANNEALED), id (0), cnt(0) {}
};
typedef struct dgopt * dgoptP;


struct carxg
{
	uint32_t caori;												//ori index in caHdl local myoripool
	uint32_t cellcount;											//number of cells comprising this orientation
	uint32_t nucsite;											//serves as a switch flag, initially the coordinate is placed, once placed or identified as an unoccupyable place marked with flag ,implicit 3D coordinate ix + iy*box + iz * boxxy
	uint32_t startingsite;										//implicit 3D coordinate ix + iy*box + iz * boxxy persistent can be utilized to get the original location of the nucleus
	double tincub;
	carxg() : caori(0), cellcount(0), nucsite(0), tincub(0.0) {}
};
typedef struct carxg * carxgP;


//an ideal orientation and up to which scatter it is considered as such
struct ideal
{
	double bunge1;												//Bunge (3,1,3) convention Euler angles
	double bunge2;
	double bunge3;

	double q0;													//unit quaternion representing the orientation
	double q1;
	double q2;
	double q3;

	double scatter;												//scatter range in degrees utilized
	ideal() : bunge1(0.0), bunge2(0.0), bunge3(0.0), q0(1.0), q1(0.0), q2(0.0), q3(0.0), scatter(DEFAULT_SCATTER) {}
};
typedef struct ideal * idealP;


//##MK::add further if desired
struct physData
{
	double G;
	double b;
	double Gzero;												//0K Shear modulus
	double nadal_lepoac_a;										//Nadal LePoac G(T) temperature-dependent shear modulus, a constant
	double nadal_lepoac_zeta;									//zeta constant
	double bzero;												//0K Burgers vector
	double thermexp_second;										//second order term in thermal lattice expansion coefficient
	double thermexp_first;										//first-order term
	double thermexp_null;										//constant term
	double Tmelt;												//melting temperature

	double LAGBm0;												//low-angle grain boundary preexponential factor intrinsic mobility
	double LAGBHact;											//activation enthalpy
	double HAGBm0;												//general high-angle grain boundary
	double HAGBHact;
	double GSm0;												//close to 40deg111 in disorientation space misoriented grains that are eligible for faster boundaries ie for Al
	double GSHact;

	double RH_HAGBm0;											//Rollett Holm model
	double RH_HAGBHact;
	double RH_LAGBHAGBcut;
	double RH_LAGBHAGBtrans;
	double RH_LAGBHAGBexponent;

	double defgmean_rd;
	double defgmean_td;
	double defgmean_nd;
	double defgmean_poisson;
	//##MK::add sensible defaults
};
typedef struct physData * physDataP;


struct dragData
{
	double ZenerAlpha;											//3/2 or close to it
	double ZenerGamma;											//interfacial energy
	double fr;													//dispersion degree

	unsigned char ZenerConsider;

	//add further physics for solute drag if desired...
	dragData() : ZenerAlpha( DEFAULT_ZENER_ALPHA), ZenerGamma( DEFAULT_ZENER_GAMMA ), fr(0.0), ZenerConsider(DISPERSOIDDRAG_NO) {}
};
typedef struct dragData * dragDataP;


struct recoveryData
{
	double VacancyDiffGeo;
	double VacancyDiffD0;
	double VacancyDiffHact;
	double SoluteDiffD0;
	double SoluteDiffHact;
	double SoluteLsPropFactor;
	double SoluteConcentration;
	double NesAlpha3;
	double NesKappa2;
	double NesC3;
	double NesC4;
	double NesC5;

	unsigned char RecoveryConsider;
	//##MK::add further recovery models here
	//##MK::add sensible defaults
};
typedef struct recoveryData * recoveryDataP;


struct userCAGeom
{
	uint32_t nNucleiCSR;										//so many nuclei in case of prescribed fixed number of nuclei
	uint32_t nboxedge_rd;										//so many cells in each direction
	uint32_t nboxedge_td;										//right-handed x || RD, y || TD, z || ND
	uint32_t nboxedge_nd;
	uint32_t nboxarea_rdtd;
	uint32_t nboxvol_rdtdnd;

	double cellsize;											//in (m)
	double boxedge_rd;
	double boxedge_td;
	double boxedge_nd;
	double boxarea_rdtd;
	double boxvol_rdtdnd;
	userCAGeom() : nNucleiCSR(0), nboxedge_rd(0), nboxedge_td(0), nboxedge_nd(0), nboxarea_rdtd(0), nboxvol_rdtdnd(0), cellsize(0.0), boxedge_rd(0.0), boxedge_td(0.0), boxedge_nd(0.0), boxarea_rdtd(0.0), boxvol_rdtdnd(0.0) {}
};
typedef struct userCAGeom * userCAGeomP;


struct userCADefMS
{
	unsigned char defmstype;
	uint32_t ngrx;												//GIA grain grid
	uint32_t ngry;
	uint32_t ngrz;
	uint32_t ngrxy;
	uint32_t ngrxyz;
	double u_xrd;		//coordinate of the GIA/grain grid origin to the local box origin absolute size in (m)
	double v_ytd;
	double w_znd;
	userCADefMS() : defmstype(CUBOID_DEFMS), ngrx(1), ngry(1), ngrz(1), ngrxy(1), ngrxyz(1), u_xrd(0.0), v_ytd(0.0), w_znd(0.0) {}
};
typedef struct userCADefMS * userCADefMSP;


struct nucData
{
	unsigned char gbnucleation;
	unsigned char csrnucleation;
	unsigned char clustnucleation;
	unsigned char tincubmodel;
	unsigned char continuous_nucleation;

	//##MK::add further parameter here
	uint32_t defaultnucdensity;
	double cluster_nclust;
	double cluster_lambda;
	double cluster_rvesize;
	double cluster_a;
	double cluster_b;
	double cluster_c;
	double gbnucleation_dens2num;

	double gbnucleation_drho2dens;
	double gbnucleation_scatter;
	double tincub_rayleigh_sigma;

	//continuous nucleation
	double pre_factor_nuc_rate;
	double Q_act;
	double rho_0;
	nucData() : gbnucleation(GBNUCLEATION_NO), csrnucleation(CSRNUCLEATION_NO), clustnucleation(CLUSTERNUCLEATION_NO), tincubmodel( TINCUB_SITESATURATION ), continuous_nucleation( CONTNUCLEATION_NO ), defaultnucdensity( DEFAULT_NUCDENSITY_CSR ), cluster_nclust(0.0), cluster_lambda(0.0), cluster_rvesize(0.0), cluster_a(0.0), cluster_b(0.0), cluster_c(0.0), gbnucleation_dens2num(0.0), gbnucleation_drho2dens(0.0), gbnucleation_scatter(0.0), tincub_rayleigh_sigma(0.0), pre_factor_nuc_rate(0.0), Q_act(0.0), rho_0(0.0) {}
};
typedef struct nucData * nucDataP;


/*struct jobticket
{
	int jobid;
	double estimatedMemory;
	double loadfactor;
	jobticket() : jobid(0),
};
typedef struct jobticket * jobticketP;*/


struct agrain
{
	double vol;
	double tincub;
	uint32_t ideal;
	int rank;
	agrain() : vol(0.0), tincub(0.0), ideal(RANDOM_ORIENTATION), rank(MASTER) {}
};
typedef struct agrain * agrainP;


struct profilingData
{
	long JobID;
	long Iterations;											//##MK::change to unsigned
	double tend;
	double MPIWTimeInitialization;
	double MPIWTimeSpendDefMS;
	double MPIWTimeSpendGBDetection;
	double MPIWTimeSpendNucleation;
	double MPIWTimeSpendGrowthSim;
	double MPIWTimeSpendFinalIO;
	profilingData() : JobID(0), Iterations(0), tend(0.0), MPIWTimeInitialization(0.0), MPIWTimeSpendDefMS(0.0), MPIWTimeSpendGBDetection(0.0), MPIWTimeSpendNucleation(0.0), MPIWTimeSpendGrowthSim(0.0), MPIWTimeSpendFinalIO(0.0) {}
};
typedef struct profilingData * profilingDataP;



struct loginfo_ca_physics
{
	uint32_t nx;
	uint32_t ny;
	uint32_t nz;
	uint32_t nxyz;
	uint32_t regx;
	uint32_t regy;
	uint32_t regz;
	uint32_t nboundarycells;									//how many cells where identified close to a boundary
	uint32_t ndgrseeds;											//how many deformed grain seeds where placed
	uint32_t nrxgrseeds;										//how many nuclei were place
	uint32_t ndefmicrotexture;									//into how many disjoint components was the deformation structure split
	uint32_t nnucleimicrotexture;
	uint32_t* defmicrotexture;									//need deleting only in caHdl because they in ensHdl they are only copied from specific ca instance how many cells
	uint32_t* nucleimicrotexture;								//how many nuclei found in each texture class
	double storedenergy;
	loginfo_ca_physics() : nx(0), ny(0), nz(0), nxyz(0), regx(0), regy(0), regz(0), nboundarycells(0), ndgrseeds(0), nrxgrseeds(0), ndefmicrotexture(0), nnucleimicrotexture(0),defmicrotexture(NULL), nucleimicrotexture(NULL) {}
};
typedef struct loginfo_ca_physics * loginfo_ca_physicsP;


struct defgseed
{
	uint32_t ensdefgpoolid;
	uint32_t mydefgpoolid;
	uint32_t location;											//linear implicitly addressed position along x, stacked in y, xy slabs stacked in z
	defgseed() : ensdefgpoolid(NOT_ASSIGNED_YET), mydefgpoolid(I_DONT_KNOW_YET), location(INVALID_ADDRESS) {}
};
typedef struct defgseed * defgseedP;


struct point
{
	double x;
	double y;
	double z;
	point() : x(-1.0),y(-1.0),z(-1.0) {}
};
typedef struct point * pointP;


struct ebsdpoint
{
	double x;
	double y;
	double bunge1;
	double bunge2;
	double bunge3;
	double q0;
	double q1;
	double q2;
	double q3;
	double iq;
	double ci;
	double kam;
	uint32_t gid;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
	ebsdpoint() : x(-1.0), y(-1.0), bunge1(0.0), bunge2(0.0), bunge3(0.0), q0(1.0), q1(0.0), q2(0.0), q3(0.0), iq(0.0), ci(0.0), kam(-1.0), gid(I_DONT_KNOW_YET), r(RGB_MIN), g(RGB_MIN), b(RGB_MIN),a(RGB_MAX) {}
};
typedef struct ebsdpoint * ebsdpointP;


struct ebsdgrain
{
	double baryx;
	double baryy;
	double bunge1;
	double bunge2;
	double bunge3;
	double q0;
	double q1;
	double q2;
	double q3;
	double kam;
	uint32_t gid;
	ebsdgrain() : baryx(-1.0), baryy(-1.0), bunge1(0.0), bunge2(0.0), bunge3(0.0), q0(1.0), q1(0.0), q2(0.0), q3(0.0), kam(0.0), gid(I_DONT_KNOW_YET) {}
};
typedef struct ebsdgrain * ebsdgrainP;


struct cellsBndFast
{
	//utilizes the fact that the dimensions of the automaton are constraint to uint32_t
	uint32_t location;
	uint32_t seedid;
	cellsBndFast() : location(NOT_ASSIGNED_YET), seedid(I_DONT_KNOW_YET) {}
};
typedef struct cellsBndFast * cellsBndFastP;


struct cellsTJ
{
	uint32_t location;
	cellsTJ() : location(NOT_ASSIGNED_YET) {}
};
typedef struct cellsTJ * cellsTJP;


struct cellsHJ
{
	uint32_t location;
	cellsHJ() : location(NOT_ASSIGNED_YET) {}
};
typedef struct cellsHJ * cellsHJP;


struct organizeBnd
{
	uint32_t key;				//the seed id gposDown adjacent to grain with seed id pmax, gposUp
	uint32_t cnt;				//how many interface cells in this boundary
	uint32_t poslocal;			//a positional index at which memory location on thefaces to find the face again
	organizeBnd() : key(UINT32T_MAX), cnt(0), poslocal(INVALID_ADDRESS) {}
};
typedef struct organizeBnd * organizeBndP;


struct organizeGBNuc
{
	uint32_t pmax;				//the high grain and its boundary faces
	uint32_t thr;
	uint32_t fid;
	uint32_t istart;			//defining the inclusive limits [istart, iend] on an ID range
	uint32_t iend;
	organizeGBNuc() : pmax(UINT32T_MAX), thr(UINT32T_MAX), fid(UINT32T_MAX), istart(UINT32T_MAX), iend(UINT32T_MAX) {}
};
typedef struct organizeGBNuc * organizeGBNucP;

struct bndFaceFast
{
	double disori;												//disori angle of the two grains
	cellsBndFast* voxelizedface;								//the voxel pair that make up the boundary ALWAYS + 1 to have zero entries denoting positions at which nucleation criteria have already been analyzed
	uint_fast64_t id;											//a unique hashtag (MAXMIN) of disjoint IDs
	uint_fast32_t gposUp;										//the two grains adjoining the face
	uint_fast32_t gposDown;
	uint32_t nvoxeltotal;
	uint32_t nextfreevoxelslot;
	uint32_t gbID;												//a consecutive name for the boundary after consolidation
	//uint32_t pad;
	bndFaceFast() : disori(GBDISORI_NOT_REQUIRED_YET), voxelizedface(NULL), id(0), gposUp(UINT32T_MAX), gposDown(UINT32T_MAX), nvoxeltotal(0), nextfreevoxelslot(0), gbID(GBNAME_UNKNOWN) {}
};
typedef struct bndFaceFast * bndFaceFastP;


struct bndColumnFast
{
	bndFaceFastP thefaces;										//the datastructure carrying metadata and references to voxel of the face
	uint32_t len;												//actual number of faces
	uint32_t maxLen;											//preallocated buffer for faces
};
typedef struct bndColumnFast * bndColumnFastP;


struct rxgcommit
{
	uint32_t nrxgid;
	uint32_t nucsite;
	double tincub;
	rxgcommit() : nrxgid(I_DONT_KNOW_YET), nucsite(NUCLEUS_ALREADY_PLACED), tincub(0.0) {}
};
typedef struct rxgcommit * rxgcommitP;


struct loginfo_nexus
{
	double localtime;
	double localxcoarse;
	double localxexact;
	unsigned int localiteration;
	unsigned int pad;
	loginfo_nexus() : localtime(-1.0), localxcoarse(-1.0), localxexact(-1.0), localiteration(0), pad(0) {};
	loginfo_nexus( const double t, const double xcoarse, const double xexact, unsigned int iteration ) :
		localtime(t), localxcoarse(xcoarse), localxexact(xexact), localiteration(iteration), pad(0) {}
};

struct loginfo_grainevo_ca
{
	double localtime;											//the local time when the timestamp was taken
	double localX;												//the local recrystallized fraction as the timestamp was taken
	uint32_t locallogcnt;
	uint32_t localtstep;										//the local integration step when the log was taken
	uint32_t localSv;											//the local number of cells active in the whole domain
	uint32_t nlocaldata;										//how many grains are tracked here
	uint32_t* localdatabucket;									//pointing to an array of nlocaldata elements where the volume of the grains is stored
	loginfo_grainevo_ca() : localtime(-1.0), localX(-1.0), locallogcnt(UINT32T_MAX), localtstep(UINT32T_MAX),localSv(UINT32T_MAX),nlocaldata(UINT32T_MAX),localdatabucket(NULL) {}
};
typedef struct loginfo_grainevo_ca * loginfo_grainevo_caP;


struct loginfo_rxfrontstats_ca
{
	double localtime;
	double localX;
	double localmemory;
	double localPmax;
	uint32_t localstep;
	uint32_t localSv;

	//track caching and fragmentation state of RXfront list my
	uint32_t ntotalRXFront;
	uint32_t nextSlotNeverActiveRXFront;							//int references still juvenile/INACTIVE end of the RXFront that has been cached
	uint32_t nCurrentlyActive;										//how many cells are currently in the interval [0, nextSlotNeverActiveRXFront) ACTIVE?
	//FullRXList
	uint32_t ntotalFullRXList;
	uint32_t nextSlotToFullRX;										//pointing in FullRXList which cell should next infect and then be switched off thus claimed as INACTIVE
	//RecyclingList
	uint32_t ntotalRecyclingList;
	uint32_t nextSlotThatBecomesRecycled;							//int id referencing which entry in myRXFront can be reused (as having been flagged INACTIVE) to keep the RXFront as filled/unfragmented and compact/short as possible
	uint32_t firstNotRecycledYet;
	//###add thread-based collection of cell fragmentation and utilization
	loginfo_rxfrontstats_ca() : localtime(-1.0), localX(-1.0), localmemory(-1.0), localPmax(-1000.0), localstep(UINT32T_MAX), localSv(UINT32T_MAX), ntotalRXFront(UINT32T_MAX), nextSlotNeverActiveRXFront(UINT32T_MAX), nCurrentlyActive(UINT32T_MAX), ntotalFullRXList(UINT32T_MAX),nextSlotToFullRX(UINT32T_MAX),ntotalRecyclingList(UINT32T_MAX),nextSlotThatBecomesRecycled(UINT32T_MAX),firstNotRecycledYet(UINT32T_MAX) {}
};
typedef struct loginfo_rxfrontstats_ca * loginfo_memory_caP;


struct loginfo_rxareaprofile_ca
{
	double localtime;
	double localX;
	uint32_t localstep;
	uint32_t* localdatabucket;
	loginfo_rxareaprofile_ca() : localtime(-1.0), localX(-1.0), localstep(UINT32T_MAX), localdatabucket(NULL) {}
	loginfo_rxareaprofile_ca(const double _lt, const double _lx, const uint32_t _lstp, uint32_t* _bucket) :
		localtime(_lt), localX(_lx), localstep(_lstp), localdatabucket(_bucket) {}
};


struct omp_log_rxfstats
{
	double localtime;
	double localX;
	double localmemory;

	double tCalcGrowthInside;										//profiling subroutine time
	double tCalcGrowthBorder;
	double tUpdateInside;
	double tUpdateBorder;
	double tSyncHalos;
	double tSeqOverhead;
	double tDefragment;

	double localPmax;
	uint32_t localstep;
	uint32_t regionSvInside;
	uint32_t regionSvBorder;

	uint32_t ntotalRXFrontInside;
	uint32_t ntotalRXFrontBorder;
	uint32_t nextSlotNeverActiveRXFrontInside;
	uint32_t nextSlotNeverActiveRXFrontBorder;
	uint32_t nCurrentlyActiveInside;
	uint32_t nCurrentlyActiveBorder;

	uint32_t ntotalFullRXListInside;
	uint32_t ntotalFullRXListBorder;
	uint32_t nextSlotToFullRXInside;
	uint32_t nextSlotToFullRXBorder;

	uint32_t ntotalRecyclingListInside;
	uint32_t ntotalRecyclingListBorder;
	uint32_t nextSlotThatBecomesRecycledInside;
	uint32_t nextSlotThatBecomesRecycledBorder;
	uint32_t firstNotRecycledYetInside;
	uint32_t firstNotRecycledYetBorder;
	omp_log_rxfstats() : localtime(-1.0), localX(-1.0), localmemory(-1.0), tCalcGrowthInside(0.0), tCalcGrowthBorder(0.0), tUpdateInside(0.0), tUpdateBorder(0.0), tSyncHalos(0.0), tSeqOverhead(0.0), tDefragment(0.0), localPmax(-1000.0), localstep(UINT32T_MAX), regionSvInside(UINT32T_MAX), regionSvBorder(UINT32T_MAX), ntotalRXFrontInside(UINT32T_MAX), ntotalRXFrontBorder(UINT32T_MAX), nextSlotNeverActiveRXFrontInside(UINT32T_MAX),nextSlotNeverActiveRXFrontBorder(UINT32T_MAX), nCurrentlyActiveInside(UINT32T_MAX), nCurrentlyActiveBorder(UINT32T_MAX), ntotalFullRXListInside(UINT32T_MAX), ntotalFullRXListBorder(UINT32T_MAX), nextSlotToFullRXInside(UINT32T_MAX), nextSlotToFullRXBorder(UINT32T_MAX), ntotalRecyclingListInside(UINT32T_MAX), ntotalRecyclingListBorder(UINT32T_MAX), nextSlotThatBecomesRecycledInside(UINT32T_MAX), nextSlotThatBecomesRecycledBorder(UINT32T_MAX), firstNotRecycledYetInside(UINT32T_MAX), firstNotRecycledYetBorder(UINT32T_MAX) {}
};
typedef struct omp_log_rxfstats * omp_log_rxfstatsP;


struct omp_log_synthfstats
{
	double localX;
	double tCalcGrowthInside;										//profiling subroutine time
	double tCalcGrowthBorder;
	double tUpdateInside;
	double tUpdateBorder;
	double tSyncHalos;
	double tSeqOverhead;
	//key questions all motivated by maintaining at best all cells active:
	//what is the fragmentation degree? (1 - nowActiveSeed/firstneverActiveSeedi)
	uint32_t localstep;
	uint32_t regionSvInside;
	uint32_t regionSvBorder;
	uint32_t ntotalSeedInside;
	uint32_t ntotalSeedBorder;
	uint32_t firstNeverActiveSeedInside;
	uint32_t firstNeverActiveSeedBorder;
	//uint32_t nowActiveSeedInside;
	//uint32_t nowActiveSeedBorder;
	//MK::number of nowInactiveSeedi := firstNeverActiveSeedi - nowActiveSeedi

	omp_log_synthfstats() : localX(-1.0), tCalcGrowthInside(0.0), tCalcGrowthBorder(0.0), tUpdateInside(0.0), tUpdateBorder(0.0), tSyncHalos(0.0), tSeqOverhead(0.0),localstep(0), regionSvInside(0), regionSvBorder(0), ntotalSeedInside(0), ntotalSeedBorder(0), firstNeverActiveSeedInside(0),firstNeverActiveSeedBorder(0) {} //, nowActiveSeedInside(0), nowActiveSeedBorder(0) {}

	/*//omp_log_synthfstats() : localtime(-1.0), localX(-1.0), localmemory(-1.0), tCalcGrowthInside(0.0), tCalcGrowthBorder(0.0), tUpdateInside(0.0), tUpdateBorder(0.0), tSyncHalos(0.0), tSeqOverhead(0.0), tDefragment(0.0), localPmax(-1000.0), localstep(UINT32T_MAX), regionSvInside(UINT32T_MAX), regionSvBorder(UINT32T_MAX), ntotalRXFrontInside(UINT32T_MAX), ntotalRXFrontBorder(UINT32T_MAX), nextSlotNeverActiveRXFrontInside(UINT32T_MAX),nextSlotNeverActiveRXFrontBorder(UINT32T_MAX), nCurrentlyActiveInside(UINT32T_MAX), nCurrentlyActiveBorder(UINT32T_MAX), ntotalFullRXListInside(UINT32T_MAX), ntotalFullRXListBorder(UINT32T_MAX), nextSlotToFullRXInside(UINT32T_MAX), nextSlotToFullRXBorder(UINT32T_MAX), ntotalRecyclingListInside(UINT32T_MAX), ntotalRecyclingListBorder(UINT32T_MAX), nextSlotThatBecomesRecycledInside(UINT32T_MAX), nextSlotThatBecomesRecycledBorder(UINT32T_MAX), firstNotRecycledYetInside(UINT32T_MAX), firstNotRecycledYetBorder(UINT32T_MAX) {}
	//##MK::mining these data will allow to further improve memory utilization
	//status of management routines
	uint32_t ntotalFullSeedListInside;
	uint32_t ntotalFullSeedListBorder;
	uint32_t nextSlotToFullSeedInside;
	uint32_t nextSlotToFullSeedBorder;
	uint32_t ntotalRecyclingListInside;
	uint32_t ntotalRecyclingListBorder;
	uint32_t nextSlotThatBecomesRecycledInside;
	uint32_t nextSlotThatBecomesRecycledBorder;
	uint32_t firstNotRecycledYetInside;
	uint32_t firstNotRecycledYetBorder;*/
};
typedef struct omp_log_synthfstats * omp_log_synthfstatsP;


struct rediscr_window
{
	double* ensRediscrTime;
	double tensmin;
	double tensmax;
	uint32_t nslots;												//rediscretize in so many disjoint slots [tensmin;..;tensmax]
	unsigned char strategy;											//REDISCR_TIME_EQUIDISTANT, linear in n steps between [tensmin; tensmax], REDISCR_TIME_LOGDISTANT
	//rediscr_window() : ensRediscrTime(NULL), tensmin(-1.0), tensmax(-1.0), nslots(0), strategy(255) {}
};
typedef struct rediscr_window * rediscr_windowP;


inline bool SortDoubleAscending(const double &dbl1 , const double &dbl2) {
	return dbl1 < dbl2;
}

inline bool SortDoubleDescending( const dgopt &anopt1, const dgopt &anopt2 ) {
	return anopt1.rho > anopt2.rho;
}

inline bool SortGSDAscending( const agrain &ag1, const agrain &ag2 ) {
	return ag1.vol < ag2.vol;
	//sorts only ascending in volume other properties ignored
}

inline bool SortFGSDCompleteAsc( const MPI_IO_FGSDComplete &fg1, const MPI_IO_FGSDComplete &fg2 ) {
	return fg1.finalvol < fg2.finalvol;
}

inline bool SortIntAscending( const uint32_t &it1, const uint32_t &it2 )
{
	return it1 < it2;
}



class caHdl;
typedef caHdl * caHdlP;

class caregionMemHdl;
typedef caregionMemHdl * caregionMemHdlP;

struct caregionGeom
{
	uint32_t nreg_rdmin;											//inclusive which boxcoordinates the region contains
	uint32_t nreg_rdmax;
	uint32_t nreg_tdmin;
	uint32_t nreg_tdmax;
	uint32_t nreg_ndmin;
	uint32_t nreg_ndmax;

	uint32_t nreg_rd;												//dimensions of the region
	uint32_t nreg_td;
	uint32_t nreg_nd;
	uint32_t nregarea_rdtd;
	uint32_t nregvol_rdtdnd;

	uint32_t nedge_global_rd;
	uint32_t nedge_global_td;
	uint32_t nedge_global_nd;

	double cellsize;												//in (m)
};
typedef struct caregionGeom * caregionGeomP;


struct caregionPart
{
	uint32_t nreg_rdx;
	uint32_t nreg_tdy;
	uint32_t nreg_ndz;
};
typedef struct caregionPart * caregionPartP;



class ensembleHdl : public io //, public mathMethods
{
	//##MK::friend class caHdl
	//ensembleHdl is a friend of the caHdl so can for IO purpose access output from the caHdl

public:
	ensembleHdl();
	~ensembleHdl();

	//prototypes
	uint32_t ens_get_closest_standardlage( double * quat );
	uint32_t ens_check_disjunctness_core( double * q_ori, bool thesebunge, double phi1, double Phi, double phi2 );
	uint32_t ens_check_disjunctness_io( double * bunge_ori );					//tests for disjunctness of orientations that are introduced into the orientation pool

	void init_ensprng( bool SetDissimilarSeedsForAll );				//true - seed is -1*myRank, false - all the same DEFAULT_PRNG_SEED
	void init_ensprng( int targetseed );							//##MK::only for single CA sets seeds explicitly the same for all CAs

	void init_mpidatatypes();
	bool init_parameter ();

#ifdef PARSE_CONFIG_FROM_HDF
	bool init_parameter_from_hdf_runtime();
	bool init_parameter_from_hdf_output();
	bool init_parameter_from_hdf_deformation();
	bool init_parameter_from_hdf_nucleation();
	bool init_parameter_from_hdf_material();
	bool init_parameter_from_hdf_mobilities();
	bool init_parameter_from_hdf_recovery();
	bool init_parameter_from_hdf_drag();
	bool init_parameter_from_hdf_idealtexture();
	bool init_parameter_from_hdf_processing();
	bool init_parameter_from_hdf_defgpool();
	bool init_parameter_from_hdf_rxgpool();
	bool init_parameter_from_hdf_additional();
#else
	#ifdef PARSE_CONFIG_FROM_YML
		bool init_parameter_from_yml_runtime();
		bool init_parameter_from_yml_output();
		bool init_parameter_from_yml_ensemble();
		bool init_parameter_from_yml_nucleation();
		bool init_parameter_from_yml_material();
		bool init_parameter_from_yml_mobilities();
		bool init_parameter_from_yml_recovery();
		bool init_parameter_from_yml_drag();
		bool init_parameter_from_yml_idealtexture();
		bool init_parameter_from_yml_processing();
		bool init_parameter_from_yml_defgpool();
		bool init_parameter_from_yml_rxgpool();
		bool init_parameter_from_yml_additional();
	#else
		bool init_parameter_from_uds_runtime();
		bool init_parameter_from_uds_output();
		bool init_parameter_from_uds_ensemble();
		bool init_parameter_from_uds_nucleation();
		bool init_parameter_from_uds_material();
		bool init_parameter_from_uds_mobilities();
		bool init_parameter_from_uds_recovery();
		bool init_parameter_from_uds_drag();
		bool init_parameter_from_uds_idealtexture();
		bool init_parameter_from_uds_processing();
		bool init_parameter_from_uds_defgpool();
		bool init_parameter_from_uds_rxgpool();
		bool init_parameter_from_uds_additional();
	#endif
#endif
	bool init_ebsdmap_read( const char* ebsdfname );
	bool init_parameter_defgpool_exp();
	bool init_parameter_rxgpool_exp();
	bool init_parameter_defgpool_damask();
	bool init_parameter_rxgpool_damask();

	void report_successful_setup();
	void report_datastructures();
	void compare_two_ang_files( const char* inputang, const char* snpang );
	void ang_file_coloring( const char* inputang, unsigned int nskip );
	void ipf_colormap( double ipf_stretch_r, double ipf_stretch_g, double ipf_stretch_b, unsigned int id );

	void init_distributeWorkOnRanks();						//does the work distribution at process level, then caHdl takes over
	void init_distributeWorkOnThreads();
	void SIMULATE_myCAs ();

	//internally MPI synchronized postprocessing routines, MASTER writing output
	void postprocess_write_mycas_mpislow();
	void postprocess_write_mycas_mpifast();
	void postprocess_initrediscrtimes();						//set the timeslotting in which local data are analyzed
	void postprocess_init ();									//ensembleHdl is friend to caHdl so it can access output data

	void postprocess_rediscr_kinetics();
	void postprocess_rediscr_macrotexture();
	void postprocess_rediscr_finalgrainsizedistribution();
	bool postprocess_rediscr_finalgrainsizedistribution_mpifast();
	void PERFORM_SOLITARYUNIT_MODELING_ANALYSIS();

	void destroy_myCAs();

	void init_nexus_results_file();
	void finish_nexus_results_file();

	vector<double> UserDefLogPoint_X_CellListDefragment;
	vector<double> UserDefLogPoint_X_Output;
	vector<double> UserDefLogPoint_MS_Rendering;
	vector<double> UserDefLogPoint_MS_RenderZPos;
	vector<uint32_t> UserDefLogPoint_WhichCAtoOutput;

	//my ensemble of CAs
	vector<loginfo_ca_physics> myCAPhysics;							//keeps track of how many nuclei and other quantities in local domains
	vector<profilingData> myCAProfiler;								//keeps information how long individual simulations took
	vector<uint32_t> WhichCAonWhichRank;							//identifies which SU runs on which MPI rank
	vector<uint32_t> myIDs;
	vector< vector<caHdlP> > myCAs;									//pointing to the caHdl objects that await execution on myRank

	vector<ori> worldoripool;										//all disjoint orientations in the simulations, local ranks take only a sublist
	vector<ideal> standardlagen;									//commonly referred to texture components in Materials Science
	vector<defg> worlddefgpool;										//all disjoint deformed grain (orientations) with a combination of dislocation density and stored elastic energy
	vector<rxg> worldrxgpool;										//all disjoint recrystallizing grain orientations

	//annealing schedule global to all SUs
	vector<double> ttime;											//in (s)
	vector<double> ttemperature;									//in (K)
	vector<double> dispersoidtime;									//in (s)
	vector<double> dispersoidfr;									//zenerfac * f/r in (J/m^2 * 1/m)

	//potential EBSD input data
	vector<ebsdpoint> expData;
	/*vector<uint32_t> expGrainIDs;*/
	map<uint32_t,uint32_t> expUniqueGrainIDs;
	vector<ebsdgrain> expGrains;

	struct rediscr_window ensRediscretization;
	struct userCAGeom ensCAGeometry;
	struct physData ensPhysData;
	struct dragData ensDragData;
	struct recoveryData ensRecoveryModel;
	struct nucData ensNucleationModel;

	//integrator accuracy
	//MPI process relevant information in the parallel environment
	double maxfillperstep;											//how much partial volume of a cell does the fastest boundary sweep in one timestep
	double ensMemGuard;												//how much memory is currently consumed in the process in MB
	double initialRelCellCaching;									//how much of the total number of cells to reserve to cache pieces of information
	double transientRelCellRecaching;								//how much of the already existent number of cells to reserve to carry additional cells
	double ebsdstepsize;
	double XMAX;
	double TMAX;
	uint32_t NTSTEPSMAX;
	uint32_t simid;
	uint32_t output_freq;

	int myRank;														//my MPI ID in the MPI_COMM_WORLD
	int nRanks;														//total number of MPI processes that work in the world
	uint32_t nworldCAs;												//so many SUs in MPI_COMM_WORLD
	uint32_t nensCAs;												//so many SUs in my, i.e. the local MPI ranks's ensemble/queue

	//exiting, IO and file access at MPI process level
	unsigned char percolation;
	unsigned char defmsmethod;
	unsigned char mobilitymodel;
	unsigned char outopt_rendermethod;
	unsigned char outopt_rendercolormodel;
	unsigned char outopt_renderfileformat;
	unsigned char outopt_logboundaries;
	unsigned char outopt_localrenderboundaries;
	unsigned char outopt_rxfront;
	unsigned char outopt_threadprofiling;
	unsigned char outopt_singlegrainevo;
	unsigned char outopt_generateDAMASKgeom;
	unsigned char outopt_artsemebsd;
	char * filename;
	string ebsd_file_name;
	string damask_file_name;

	bool onthefly_defragmentation;
	bool ensembleSuccess;
	bool postProcessing;
	bool experimentInput;
	bool damaskInput;

	struct aabb renderwindow;
	int ensembleprngseed;
	randomClass ensembleprng;
	mathMethods ensmath;
#ifdef PARSE_CONFIG_FROM_HDF
	string nexus_config_file_name;
#else
#ifdef PARSE_CONFIG_FROM_YML
	string yaml_config_file_name;
	ryml::Tree tree;
#else
	FILE * score_input;
#endif
#endif
	string uds_config_file_name;
	string nexus_results_file_name;

	//MPIDatatypes
	MPI_Datatype MPI_IO_CAProfilingInfoData_Type;
	MPI_Datatype MPI_IO_CAPhysicsInfoData_Type;
	MPI_Datatype MPI_IO_FinalGSDInfoData_Type;
	MPI_Datatype MPI_IO_FGSDComplete_Type;

	//performance counters
	double prof_t0;
	double prof_tstart;
	double prof_tend;
	string simwrkdir;
};
typedef class ensembleHdl * ensembleHdlP;


class caHdl : public randomClass, public percAnalyzer //, public mathMethods //, not public io because IO is only done by the ensembleHdl each object of caHdl is associated with
{
	friend class ensembleHdl;

public:
	caHdl();
	~caHdl();

//private:
	ensembleHdlP myensHdl;					//to allow the caHdl access to the public available elements of his "group/ensemble" leader at the process level
	int jobid;								//a unique ID that identifies which automaton this caHdl object represents, its request is restricted as the jobid is private, only the ensembleHdl can ask in his myCAs[threadid][0-(myCAs[threadid].size-1)] which this ID references!
	vector<caregionMemHdlP> regions;		//thread local regions

	//handling of orientations
	uint32_t ca_get_closest_standardlage( double * quat );
	uint32_t ca_check_disjunctness_core( double* q_ori, bool thesebunge, double phi1, double Phi, double phi2 );
	uint32_t ca_check_disjunctness_io( double * bunge_ori );
	void colorize_myoripool_ipfz();

	//general physics
	double kam2rho( double kam );
	double get_shearmodulus( double T );
	double get_burgersvector( double T );
	double calculateBoundaryDisoriFast( uint_fast32_t seedup, uint_fast32_t seeddown );

	//EBSD importing
	void ebsd2polycrystal();
	void ebsd2polycrystal2(); //directly sets voxel

	// DAMASK importing
	void damask2polycrystal();

	//GB physics
	double calc_mobilityweight( uint32_t rgp, uint32_t dgp );
	double calc_mobilityweight_sebaldgottstein( uint32_t rgpoolid, uint32_t dgpoolid );
	double calc_mobilityweight_rollettholm( uint32_t rgp, uint32_t dgp );

	//handling of recovery submodels
	void nes_networkgrowthmodel_vacancycorediff();
	void nes_networkgrowthmodel_solutedrag();
	void nes_michalakpaxton();
	void update_mydgoptimize_rho();
	void update_mydgoptimize_cnt();
	void update_myrhomax();
	unsigned char rho2grayscale( double r );
	//void log_initrho_scalebar();

	//handling of nucleation submpdels
	uint32_t solve_nucmodeling_countgbcells( vector<organizeGBNuc>* luu );
	struct organizeGBNuc find_gbface( uint32_t idx, vector<organizeGBNuc>* luu );
	void solve_nucmodeling_gbnuc_pickrandomly();
	void solve_nucmodeling_csr_enforced();
	void solve_nucmodeling_csr_pickrandomly();
	uint32_t getDeformedGrainID( uint32_t xglo, uint32_t yglo, uint32_t zglo );
	void solve_nucmodeling_csrdiehl();
	std::vector<double> calculate_nuc_rate( void );
	void solve_continuous_nucleation( void );
    void solve_nucmodelling_dislo_ori_based ();
	void init_myMaternUniverse();
	void pickrandomly_myMaternUniverse(void );
	void solve_nucmodeling_ellipsoidalcluster_pickrandomly();

	//domain grid partitioning for threading of Solitary Unit Execution
	void ompshar_init_regions( int threadid );
	void ompshar_ca2regions();
	void ompshar_init_regionlimits( int threadid );
	uint32_t exists_nborhalo( int regid, short dx, short dy, short dz, int npx, int npy, int npz, uint32_t cand );
	void ompshar_link_haloregions( int threadid );

	//CA core functionalities
	void solve_INITIALIZATION ();
	void solve_INITIALIZATION_THREADED_DOMAIN_DECOMPOSITION();
	void solve_SYNTHESIZED_DEFORMEDSTRUCTURE_CUBOIDBLOCKS ();
	void solve_SYNTHESIZED_DEFORMEDSTRUCTURE_FROMEBSDDATA();
        void solve_SYNTHESIZED_DEFORMEDSTRUCTURE_FROM_DAMASK();
	void solve_SYNTHESIZE_DEFORMATIONSTRUCTURE();
	void solve_DETECT_GRAINBOUNDARIES();
	void solve_GRAINBOUNDARYNUCLEATION();
	void solve_NUCLEATIONMODELING();
	void solve_REPLACE_CA_STRUCTURE();
	void solve_RXGROWTH();
	void solve_FINAL_IO();

	//CA helper functionalities and init
	//MK::data in ensembleHdl serve as a library for all possible grain orientations and mobility weights
	void init_cahdlprng();
	void init_parameter ();
	void init_processing ();
	void init_zenerdrag ();
	void ompshar_initialize_region_myrxgpools( int regionid );
	void ompshar_initialize_region_mydfgpools( int regionid );
	void synchronize_mydefpool_counts();
	void synchronize_myrxgpool_counts();
	void determine_initial_nucleitexture_myrxgpoolids();

//MK::there are two CAs, one that is utilized to synthetize the deformation structure
	//3D OMP parallelized synthesis automaton for Poisson and other structures based on seeds for deformed grains
	void ompshar_deformedgrains_infect_JuvenileNucleation( uint32_t seedid, uint32_t xglo, uint32_t yglo, uint32_t zglo, uint32_t infectWhereRegion, bool inside, double frac0 );
	void ompshar_placeAllSeeds();
	void grow_deformedgrains_voxelize_omp();
	//regular cuboids
	void determine_polycrystalline_geometry();
	void pick_deformedgrains(); //selects from ensembleHdl GIA defgpool deformed grains
	void ompshar_voxelize_defms();

	//3D growth machine
	void ompshar_sim_myCA_sitesaturatedNucleation( vector<rxgcommit>* sync );
	bool ompshar_sim_myCA_infect_JuvenileNucleation( uint32_t rxgpoolid, uint32_t xglo, uint32_t yglo, uint32_t zglo, uint32_t infectWhereRegion, bool inside, double rxfrac0 ); //if inside is true infection is not at the outer shell of the region here divide juvenile infection
	inline uint32_t workPartitioning( uint32_t gid, uint32_t nworker ){ return (gid % nworker); }

	//handle processing during the simulation
	void update_system_fixedtime( double time );
	void update_temperature();				//to the time set in this->t
	void update_atomisticproperties();		//G and b and temperature dependence
	void update_intrinsicmobilities();		//Grain boundary properties
	void update_microchemistry();
	void update_deformedsubstructure(void );
	//helper functions for managing the dynamic adaptive timestepping at the CA scale at any time
	double get_rho( uint32_t defgid );
	double get_zener();
	double get_currentintrinsicmobility( double Pvalue );
	uint32_t characterize_cellstatus( uint32_t r, uint32_t lx, uint32_t ly, uint32_t lz );

	double get_dtmax_instantslope_cells();
	double get_dtmax_instantslope_temp ();
	//exchange when recovery and Zener drag is simulated
	double get_dtmax_instantslope_rho ();
	double get_dtmax_instantslope_zener ();
	double get_dtmax_instantslope_nucleation();
	double get_dtmax_minimum ();


	//on the fly microstructural state characterization
	void log_initialization();
	double log_rxareafraction( double zpos );
	void log_rxfrontstats();						//traces status information on the current state of the automaton
	void log_grainevo();							//keeps track of what the individual grains did
	void log_rxareaprofile();
	void log_rxareaprofile( uint32_t threshold );
	void log_ca_physics( loginfo_ca_physicsP  container );
	void log_OUTPUT();
	void log_OUTPUT_Diehl();
	void log_OUTPUT_NeXus();

	void nexus_write_automaton_geometry();
	void nexus_write_snapshot_kinetics();
	void nexus_write_snapshot_grain_based();
	void nexus_write_snapshot_front_based();
	void nexus_write_snapshot_cell_based();
	void nexus_write_snapshot();
	void nexus_write_summary_stats();

	void binarize_partiallyrxed_microstructure( vector<bool>* out );
	void log_PERCOLATION();
        void log_grain_IDs( double t);
        void log_grain_rho_cell_based( double t);
        void log_grain_ori_cell_based( double t);
        void log_RX_frac( double t);
        void log_grain_rho_grain_based( double t );
        void log_grain_ori_grain_based( double t);
        double calculate_RXfrac_total( std::vector<double> RXfrac );

	//post-processing solitary unit averaging
	double get_interpCellCount( uint32_t localid, double when );

	//characterization I/O allow a particular node to write status information into a plain file
	//##MK::SOME OF THESE FUNCTIONS HAVE NOT BEEN TESTED WITH THE HYBRID CODE!
	void write_rxfrontstats();					//outputs simulation metadata from the myca-th of my automata in a file
	void write_ThreadProfilingSummarySynthMachine();
	void write_ThreadProfilingSummaryGrowthMachine();
	void write_grainevolution_ascii();			//outputs grain-resolved metadata from the myca-th of my automata in a file
	void write_grainevolution_binary();
	void write_PercolationProfilingSummary();
	void write_rxareaprofile_ascii();

	//OMP Thread
	void ompcrit_write_voxeldata_coloring_regions();
	void ompcrit_write_voxeldata_coloring_grainids( string postfix );
	//void ompcrit_write_zsection_coloring_ipfz( double zpos, double xnow, char mode );
	//void ompcrit_write_zsection_looping_ipfz( double rxfraction, char thatmode );
	void ompcrit_clarify_status_infectedcells( double rxthreshold );
	//void test_lodepng();

	//artificial SEM/EBSD output
	void ompcrit_write_semebsd_core( double zpos, char mode );
	void ompcrit_write_semebsd_core( double zpos, double xnow, uint32_t threshold, char mode );
	double ompcrit_probe_semebsd_core( double zpos, uint32_t threshold );
	void ompcrit_write_semebsd_looping( char thatmode );

	//DAMASK output
	void write_damask_matconfig_ascii( string suffix );
	void write_damask_geom_ascii( string suffix );

	//deformed grain boundary tracking functionalities
	void write_junction_skeleton( unsigned char mode );
	void write_junction_log_gb( uint32_t thread, bool consolidated );
	void write_junction_log_tj();
	void write_junction_log_hj();

	bool mpiio_write_voxelgrid_ipfz( string fname );
	//HDF5 I/O
	bool hdf5_write_coordinates( string fname, string grpname );
	bool hdf5_write_voxelgrid_grainid( string fname, string grpname, string subgrpname );
	bool hdf5_write_voxelgrid_rho( string fname, string grpname, string subgrpname );
	bool hdf5_write_voxelgrid_ipfz( string fname, string grpname, string subgrpname );
	void hdf5_write_xmffile_3dcorectmesh( string fname, string hfname, string gname );

        // modular HDF5 functions
		//##MK::2023-V
        void HDF5_create_CA_output( string fname );
        void HDF5_write_CA_output_int( string fname, double t, std::vector<int>, string dset_name );
        void HDF5_write_CA_output_double_rank_1( string fname, string group_name, std::vector<double>, string dset_name );
        void HDF5_write_CA_output_double_rank_2( string fname, string group_name, std::vector<std::vector<double>>, string dset_name );
        void HDF5_writeFileAttribute( hid_t obj_id, string attribute_name, string attribute_value );
        void HDF5_writeGroupAttribute_int( hid_t obj_id, string attribute_name, uint32_t value );
        void HDF5_writeGroupAttribute_int_rank_1( hid_t obj_id, string attribute_name, std::vector<uint32_t> );
        void HDF5_writeGroupAttribute_float( hid_t obj_id, string attribute_name, double value );
        void HDF5_write_float_attribute(string fname, string group_name, double frac_RX, string attribute_name);

        hid_t HDF5_openFile( string fname );
        hid_t HDF5_openGroup( hid_t loc_id, string groupname );
        hid_t HDF5_openAttribute( hid_t loc_id, string attribute_name );
        double* HDF5_read_attribute_float( hid_t attr_id );
        //int* HDF5_read_dataset_int_rank_1( hid_t loc_id, string datasetname );
        std::vector<int> HDF5_read_dataset_int_rank_1( hid_t loc_id, string datasetname );
        std::vector<int> HDF5_read_dataset_int_rank_4( hid_t loc_id, string datasetname );
        //int* HDF5_read_dataset_int_rank_2( hid_t loc_id, string datasetname );
        std::vector<std::vector<int>> HDF5_read_dataset_int_rank_2( hid_t loc_id, string datasetname );
        //void HDF5_read_dataset_float_rank_2( hid_t loc_id, string datasetname );
        std::vector<std::vector<double>> HDF5_read_dataset_float_rank_2( hid_t loc_id, string datasetname );
        int* HDF5_read_attribute_int( hid_t attr_id );

	void raw_write_xmffile_3dcorectmesh( string fname, string binprefix );

	void ompcrit_write_voxeldata_h5( unsigned char colormodel, string grpnm, bool xdmf, string xmfsuffix, double now, double xfraction, unsigned int stp );
	void h5_return_dislocationdensity( float* buffer, uint32_t buffersize );
	void hdf5_dummy( string fnsuffix );
	void write_damask_geom_h5( string suffix ); //outputs microstructure as an ASCII file which can be loaded into DAMASK
	void write_clustersizedistr_h5_create();
	void write_clustersizedistr_h5( unsigned int* dist, unsigned int ndist ); //outputs data column with volume of current cluster of recrystallized matter to an HDF5 file

	//cleaning functions
	void cleanMyGrainEvolution();
	void cleanBookkeeping();

	vector<double> defragRXFront_atthisX;
	vector<double> output_atthisX; //copied from ensembleHdl when should the automaton log local information?
	vector<double> rendering_atthisX;
	vector<double> rendering_atthisZPos;
	vector<double> mygrainevolutionlocaltime;
	vector<loginfo_nexus> mynexusstats;
	vector<loginfo_rxfrontstats_ca> myrxfrontstatus; //keeps track of the state of the myRXFront array counters fragmentation and occupancy
	vector<loginfo_grainevo_ca> mygrainevolution;
	vector<loginfo_rxareaprofile_ca> myrxprofile;
	vector<loginfo_perc> myhk;
	vector<loginfo_xdmf> renderingxdmf;

	vector<ori> myoripool;
	vector<cadefg> mydefgpool;
	vector<carxg> myrxgpool; //directly references orientation and incubation time
	vector<point> mypointprocess;
	vector<dgopt> mydgoptimize;

	//local copies from ensembleHdl for faster access
	struct userCAGeom myCAGeometry;				//copy from the ensemble Hdl
	struct physData myPhysData;
	struct dragData myDragData;
	struct recoveryData myRecoveryModel;
	struct userCADefMS myCADefMS;				//local dimensions of GIA deformed grain grid topology
	struct nucData myNucleationModel;

	//MK::the deformation synthesis and the subsequent RX CA operate both on camemregion local mycellgrid s but
	//have own different management routines as complexity is different which we can utilize to withstain from function pointing or complex ifs

//3DCA TO SYNTHESIZE DEFORMATION STRUCTURE
	defgseedP tmpdefgseeds;						//a list of deformed grains that is sampled from the input data set and placed at particular positions
	uint32_t ndefgseeds;



//3DCA TO PERFORM RX SIMULATION the following constructs are classic arrays to avoid STL overhead and in particular the invocation of the copy constructor
	uint32_t* mycellgrid;						//the voxelized version of mydefggrid - this is where the cells infect IDS are stored in range ##MK [ 0; .. mydefgpool .. ;(mydefgpool.size+myrxgpool.size) )

	vector<vector<cellstatus>*> cellStatii;

	vector<double> ttime;						//in (s)
	vector<double> ttemperature;				//in (K)
	double tprocessingend;

	vector<double> zenertime;					//in (s)
	vector<double> zenerdispersion;				//zenerfac * f/r in (J/m^2 * 1/m)

	//##MK::currently not utilized variables with which to track local MS evolution
	//vector<double> RhoMax;					//local recovery history
	//vector<uint32_t> RhoMaxID;
	//vector<double> currZenerDrag;
	//vector<double> deltatime;

	uint32_t SvDeformed;
	uint32_t DefMicrotextureClasses;
	uint32_t NucleiMicrotextureClasses;
	uint32_t* DefMicrotexture;
	uint32_t* NucleiMicrotexture;


	double myMemGuard;							//how much memory is currently consumed in MB
	double StoredEnergy;
	double CurrentG;
	double CurrentBurgersVector;
	double CurrentTemperature;
	double t;									//current time, X(t), and temperature
	vector<double> nuclei_density;				//continuous nucleation, accumulated nuclei per voxel, size nboxvol_rdtdnd
	double X;									//current RX fraction
	double Xcells;
	double dXstep;								//accumulated change in rxVolume accounting for overshoot as well
	double dt;									//time increment
	double tsimend;								//maximum physical real time simulated
	double XMAX;
	double TMAX;
	uint32_t stepsimend;
	uint32_t Sv;										//number of cells in the front
	uint32_t step;
	uint32_t NTSTEPSMAX;
        uint32_t output_freq;

	double myMobilityWeightMax;					//maximum mobility weight detected in this automaton, if for instance locally there is no GS or only a factor of the maximum value, the local adaptive timestepping can profit from this information
	double myrhomax;							//maximum dislocation density in the system
	double myrhomax0;							//absolute maximum rho at the beginning of the simulation to enabling a scaling of the dislocation density interval [RHO_RECRYSTALLIZED_MATERIAL, myrhomax0]
	double myrhomin0;							//absolute minimum rho at the beginning of the simulation ....
	double mypzmin;
	double maxfillperstep;
	double initialRelCellCaching;				//what to do when too few memory in the cells
	double transientRelCellRecaching;
	double ebsdstepsize;

	//updated intrinsic mobilities
	double mLAGB;
	double mHAGB;
	double mGS;
	double mRHHAGB;
	double Gbhalfsq;
	double _cellsize;

	//double _kT;
	//double zenerfac;							//fgeo * gammaGB

	//performance caching variables and things used to clarify the code
	uint32_t nmydefgpool;
	uint32_t nmyrxgpool;
	uint32_t nmynuclei;

	uint32_t loginfo_rxfrontstats_cnt;
	uint32_t loginfo_mpm_cnt;
	uint32_t loginfo_grainevo_cnt;
	uint32_t loginfo_rendering_cnt;
	vector<uint32_t> loginfo_rendering_sectionbased_cnt;
	uint32_t loginfo_defrag_cnt;
	uint32_t loginfo_percolation_cnt;


	//Parallel environment relevant information
	int myensRank;
	int nRanks;
	omp_lock_t h5lock;

	unsigned char percolation;
	unsigned char mobilitymodel;
	unsigned char nucleationmodel_status;
	unsigned char outopt_localrenderhow;
	unsigned char outopt_localrendercolor;
	unsigned char outopt_localrenderfileformat;
	unsigned char outopt_localrenderboundaries;
	unsigned char outopt_logboundaries;
	unsigned char outopt_localrxfront;
	unsigned char outopt_localthreadprof;
	unsigned char outopt_localsinglegrainevo;
	unsigned char outopt_localgenDAMASKgeom;
	unsigned char outopt_localartsemebsd;


	bool mySuccess;
	bool onthefly_defragmentation;
	bool renderingForThisCA;

        char * filename;
    string damask_file_name;

	struct aabb myrenderwindow;
	randomClass localprng;
	mathMethods localmath; //allows each CA to have independent stream of random numbers for calls to localmath->r.MersenneTwister();
	//file interaction with respect to input data should be performed by ensembleHdl at processLevel exclusively
};



class juncHdl : public caHdl
{
public:
	juncHdl();
	~juncHdl();

	caHdlP mycaHdl;
	caregionMemHdlP mymemreg;
	int mythreadid;

	//detection of grain boundary faces and the skeleton of higher order junctions
	bndColumnFast* fjuncSkeleton;					//voxel adjacent to exactly 2 disjoint IDs
	cellsTJ* tjuncSkeleton;							//voxel adjacent to exactly 3 disjoint IDs
	cellsHJ* hjuncSkeleton;							//voxel adjacent to more than 3 disjoint IDs

	uint32_t fjuncSkeletonSize;						//how many elements in the static array?
	uint32_t tjuncSkeletonSize;
	uint32_t hjuncSkeletonSize;

	uint32_t tjuncnextfreeslot;
	uint32_t hjuncnextfreeslot;

	double juncMemGuard;

	//prototypes
	uint32_t read_cellstate_mx( int caller, uint32_t x, uint32_t y, uint32_t z );
	uint32_t read_cellstate_px( int caller, uint32_t x, uint32_t y, uint32_t z );
	uint32_t read_cellstate_my( int caller, uint32_t x, uint32_t y, uint32_t z );
	uint32_t read_cellstate_py( int caller, uint32_t x, uint32_t y, uint32_t z );
	uint32_t read_cellstate_mz( int caller, uint32_t x, uint32_t y, uint32_t z );
	uint32_t read_cellstate_pz( int caller, uint32_t x, uint32_t y, uint32_t z );

	void addVoxelGB( uint32_t c, uint32_t a, uint32_t b );
	void addVoxelTR( uint32_t c );
	void addVoxelHT( uint32_t c );
	inline uint32_t workPartitioning( uint32_t gid, uint32_t nworker ){ return (gid % nworker); }
	void updateFaces( uint32_t p, vector<organizeBnd>* luu );
	void registerMissingFaces( uint32_t p, vector<organizeBnd>* luu );
	void updateFaceCellBuffers( uint32_t p, vector<organizeBnd>* luu );
	void addRemoteFaceCells( uint32_t p, uint32_t, vector<organizeBnd>* luu );
	void ompshar_init_junctions();
	void ompshar_detect_junctions();
	void ompshar_report_junctions();
	void ompshar_consolidate_junctions();
	void ompshar_calc_junction_properties();
	void ompshar_cleanup_foreign_junctions();
};
typedef class juncHdl * juncHdlP;




class caregionMemHdl : public caHdl
{
	//encapsulates the deformation structure in one region to store it ccNUMA friendly local to the thread
	//friend class caHdl;

public:
	caregionMemHdl();
	~caregionMemHdl();

	caHdlP mycaHdl;									//the CA that the region is a part of
	int mythreadid;									//the pinned thread's which handles this region ID

	struct caregionGeom myGeom;
	struct caregionPart thePartitioning;

	//thread local memory management for the list of active seed cells
	//MK::it is practical to have two automata as fewer memory is necessary to infect into vacuum and the functions are simpler

	double regMemGuard;
	double dtCalcGrowthInside;
	double dtCalcGrowthBorder;
	double dtUpdateInside;
	double dtUpdateBorder;
	double dtSyncHalo;
	double dtSeqOverhead;
	double dtDefragment;

	//SeedFrontManagement at thread level
	haloP myhalos;									//the idea of the halos is to make infections directed outside the node at most independent and parallel

	uint32_t* mynborhalos;							//references the id of a neighboring halo from the myhalos list local to the neighbor!
	uint32_t* mynborregions;						//order optimized for most probable infections, faces, edges, corners

	uint32_t* mycellgrid;


	//3DCA TO PERFORM VOXELIZATION
	defcellP mySeedFrontInside;
	defcellP mySeedFrontBorder;
	uint32_t* myFullSeedInside;
	uint32_t* myFullSeedBorder;
	uint32_t* myRecyclingSeedInside;
	uint32_t* myRecyclingSeedBorder;

	uint32_t ntotalSeedFrontInside;
	uint32_t ntotalSeedFrontBorder;
	uint32_t nextSlotNeverActiveSeedInside;
	uint32_t nextSlotNeverActiveSeedBorder;
	uint32_t ncurrentActiveSeedInside;
	uint32_t ncurrentActiveSeedBorder;

	uint32_t ntotalFullSeedInside;
	uint32_t ntotalFullSeedBorder;
	uint32_t nextSlotToFullSeedInside;
	uint32_t nextSlotToFullSeedBorder;

	uint32_t ntotalRecyclingSeedInside;
	uint32_t ntotalRecyclingSeedBorder;
	uint32_t nextToRecycleSeedInside;
	uint32_t nextToRecycleSeedBorder;
	uint32_t firstNotRecycleSeedInside;
	uint32_t firstNotRecycleSeedBorder;


	//3DCA TO PERFORM RX SIMULATION the following constructs are classic arrays to avoid the STL overhead and in particular the invocation of the copy constructor
	cellP myRXFrontInside;							//bookkeeps active cells that are guaranteed in the inner cuboid of a region
	cellP myRXFrontBorder;							//bookkeeps active cells that are inside the limiting single cell layer delimiting region from all others
	uint32_t* myFullRXListInside;					//bookkeeps IDs to entries from myRXFrontInside that can infect other cells
	uint32_t* myFullRXListBorder;					//bookkeeps IDs to entries from myRXFrontBorder that can infect other cells
	uint32_t* myRecyclingListInside;				//bookkeeps IDs to entries from myRXFrontInside that are INACTIVE and such can be utilized again
	uint32_t* myRecyclingListBorder;				//bookkeeps IDs to entries from myRXFrontInside that are INACTIVE and such can be utilized again

	uint32_t ntotalRXFrontInside;
	uint32_t ntotalRXFrontBorder;
	uint32_t nextSlotNeverActiveRXFrontInside;		//int references still juvenile/INACTIVE end of the RXFront that has been cached
	uint32_t nextSlotNeverActiveRXFrontBorder;
	uint32_t nCurrentlyActiveInside;				//how many cells are currently in the interval [0, nextSlotNeverActiveRXFront i) ACTIVE?
	uint32_t nCurrentlyActiveBorder;

	//FullRXList
	uint32_t ntotalFullRXListInside;
	uint32_t ntotalFullRXListBorder;
	uint32_t nextSlotToFullRXInside;				//pointing in FullRXList which cell should next infect and then be switched off thus claimed as INACTIVE
	uint32_t nextSlotToFullRXBorder;

	//RecyclingList
	uint32_t ntotalRecyclingListInside;
	uint32_t ntotalRecyclingListBorder;
	uint32_t nextSlotThatBecomesRecycledInside;
	uint32_t nextSlotThatBecomesRecycledBorder;		//int id referencing which entry in myRXFront can be reused (as having been flagged INACTIVE) to keep the RXFront as filled/unfragmented and compact/short as possible
	uint32_t firstNotRecycledYetInside;				//##MK:: in the interval [0;nextToRecycle) all references in RecycleList have already been utilized
	uint32_t firstNotRecycledYetBorder;

	//management
	uint32_t reg_nmydefgpool;
	uint32_t reg_nCurrActiveInside;
	uint32_t reg_nCurrActiveBorder;
	uint32_t reg_nUpdatedCellsInside;
	uint32_t reg_nUpdatedCellsBorder;
	uint32_t reg_SvInside;
	uint32_t reg_SvBorder;
	double reg_dXstepInside;
	double reg_dXstepBorder;
	double reg_myMobilityWeightMax;
	uint32_t reg_nmynuclei;

	//detection of grain boundary faces and the skeleton of higher order junctions
	juncHdl myjunctions;

	//Initialization and halo management
	void ompshar_init_mycellgrid();
	uint32_t get_nbortid( int regid, short dx, short dy, short dz, int npx, int npy, int npz );
	void ompshar_init_neighbortopology( uint32_t party, uint32_t partz );
	void ompshar_initialize_halo( uint32_t whichhalo, uint32_t extx, uint32_t exty, uint32_t extz, uint32_t originx, uint32_t originy, uint32_t originz );
	void determine_origins( uint32_t regid, short dx, short dy, short dz, int npx, int npy, int npz, uint32_t * origin );
	void ompshar_init_haloregions( uint32_t party, uint32_t partz );
	double reg_calc_mobilityweight( uint32_t rgpoolid, uint32_t dgpoolid );


	//3D CA to VOXELIZE STRUCTURE
	void ompshar_init_mySeedFrontInside();
	void ompshar_init_mySeedFrontBorder();
	uint32_t omp_getNextFreeSlotInSeedFrontInside( bool how );
	uint32_t omp_getNextFreeSlotInSeedFrontBorder( bool how );
	void ompshar_voxelize_growthStepInside();
	void ompshar_voxelize_growthStepBorder();
	void ompshar_voxelize_updateFullInside();
	void ompshar_voxelize_updateFullBorder();
	void ompshar_synchronize_haloregions_def();
	void omp_infect_OutOfSeedFrontInside( uint32_t seedid, uint32_t seedfrontid, bool intoborder, short dx, short dy, short dz, unsigned char direction, double carryOver ); //###currently no overshoot!
	void omp_infect_OutOfSeedFrontBorder( uint32_t seedid, uint32_t seedfrontid, short dx, short dy, short dz, unsigned char direction, double carryOver ); //##currently no carryOver
	void append_to_fullseed_inside_list( uint32_t cid );
	void append_to_recycseed_inside_list( uint32_t cid );
	void append_to_fullseed_border_list( uint32_t cid );
	void append_to_recycseed_border_list( uint32_t cid );
	void omp_resetHaloRegions();
	void omp_resetInternalCounters();
	void omp_voxelize_memoryCleanup();


	//3D CA to EVOLVE RX GRAINS
	void ompshar_init_myRXFrontInside ();
	void ompshar_init_myRXFrontBorder ();
	uint32_t omp_getNextFreeSlotBorderAnotherRegion ( uint32_t nbortid, bool how );
	uint32_t omp_getNextFreeSlotInRXFrontInside( bool how );
	uint32_t omp_getNextFreeSlotInRXFrontBorder( bool how );
	void ompshar_defragmentation();
	void ompshar_sim_myCA_calcGrowthStepInside();
	void ompshar_sim_myCA_calcGrowthStepBorder();
	void ompshar_sim_myCA_updateFullRXInside();
	void ompshar_clear_halo_bookkeeping();
	void ompshar_sim_myCA_updateFullRXBorder();
	void ompshar_synchronize_haloregions_rx();
	//void ompshar_sim_myCA_clarify_status( double threshold );
	//void ompshar_sectioning_analysis_rx();
	void omp_infect_OutOfRXFrontInside( uint32_t rxgpoolid, uint32_t rxfrontid, bool intoborder, short dx, short dy, short dz, unsigned char direction, double carryOver ); //###currently no overshoot!
	void omp_infect_OutOfRXFrontBorder( uint32_t rxgpoolid, uint32_t rxfrontid, short dx, short dy, short dz, unsigned char direction, double carryOver ); //##currently no carryOver
	void append_to_fullrx_inside_list( uint32_t cid );
	void append_to_recycling_inside_list( uint32_t cid );
	void append_to_fullrx_border_list( uint32_t cid );
	void append_to_recycling_border_list( uint32_t cid );
	void omp_cleanupMemoryUsedForGrowthMachine();
	void omp_cleanupMemoryUsedForManagement();
	void omp_cleanupMemoryUsedForHaloRegion();
	void omp_log_synthfrontstats();
	void omp_log_rxfrontstats(); //traces status information on the current state of the automaton

	uint32_t* reg_myrxgp_counts;
	uint32_t* reg_mydfgp_counts;
	uint32_t reg_nmyrxgp_counts;
	uint32_t reg_nmydfgp_counts;
	//vector<uint32_t> reg_myrxgp_counts;
	//vector<uint32_t> reg_mydfgp_counts;
	vector<omp_log_synthfstats> profiling_synthmachine;
	vector<omp_log_rxfstats> profiling_growthmachine;
	uint32_t reg_defrag_cnt;
};
typedef class caregionMemHdl * caregionMemHdlP;


#endif

