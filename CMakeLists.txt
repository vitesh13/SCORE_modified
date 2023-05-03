cmake_minimum_required(VERSION 3.0)

################################################################################################################
##USER INTERACTION##############################################################################################
##in this section modifications and paths need to be added by the user##########################################
################################################################################################################
#please name your project accordingly
set(MYPROJECTNAME "score_test")

#tell where the source code of PARAPROBE is
# set(MYSRCPATH "/home/georg.falkinger/SCORE_orig/src/")
set(MYSRCPATH "/home/mkuehbach/SCORE-VITESH/SCORE_modified/src/cxx")

#choose which compiler to use
##GNU 7.3/MPICH and Intel/IntelMPI 2018 have been tested
set(EMPLOY_INTELCOMPILER OFF)
set(EMPLOY_GNUCOMPILER ON)
set(VERBOSE_VECTORIZATION ON)

#INTEGRATE THE HASH OF THE PARENT COMMIT FROM WHICH THIS VERSION WAS CREATED
#update GitSHA of the paraprobe repository prior commit, i.e. pre-processing upon compilation pulling last GitSHA
#we cannot update the current GitSHA as this would create a circular dependency, namely once we have commit, i.e. know the file
# set(GITSHA "0")
message([STATUS] "MYGITSHA: ${GITSHA}")
string(REGEX REPLACE "-" "_" MYCLEANGITSHA ${GITSHA}) #'-' is an invalid character in a C preprocessor macro
message([STATUS] "MYCLEANGITSHA: ${MYCLEANGITSHA}")
add_definitions("-DGITSHA=${MYCLEANGITSHA}")

#choose optimization level
##-O0 for debug purposes only
##-O1 or -O2 for moderate optimization
##-O3 for production level optimization
set(MYOPTLEVEL "-O2")

#optional HDF5 location
##this is also optional and if used in beta stage support
set(LIB_SZPATH "/opt/szip-2.1.1/GNU/11.2.0/")
set(LIB_ZPATH "/usr/local/")
set(LIB_AECPATH "/usr/local/")

set(EMPLOY_HDFSUPPORT ON)
if (EMPLOY_HDFSUPPORT)
	# set(MYHDFPATH "/opt/hdf5-1.12.1/GNU/11.2.0/OpenMPI/4.1.2/")
	set(MYHDFPATH "/home/mkuehbach/PARAPROBE-RELEASE-GITLAB/paraprobe-toolbox/code/thirdparty/mandatory/hdf5/CMake-hdf5-1.14.0/HDF5-1.14.0-Linux/HDF_Group/HDF5/1.14.0")

	#download Unix tar archive for the HDF5-1.10.2 release from www.hdf5.org
	#copy into local directory, do not use sudo to execute
	#unpack via tar -xvf tarballname
	#change into directory from now on this is referred to as the hdfroot folder
	#execute ./build_unix.sh  this will configure the static libraries, sequential no fortran support and use deflate compression library
	#when configure build and test has been completed
	#take packed archive and extract into the automatically generated build folder within the hdfroot folder
	#deep in these folders are the include directories and the static libraries (a files) we to understand the hdf5.cpp commands and 
	#the libraries we need to link statically against to execute hdf5 functionalities
	include_directories("${MYHDFPATH}/include")
	link_directories("${MYHDFPATH}/lib")
	##link against static libraries
	#set(MYHDFLINKFLAGS "-L${MYHDFPATH}/lib/ ${MYHDFPATH}/lib/libhdf5_hl.a ${MYHDFPATH}/lib/libhdf5.a -lz -ldl")
	# set(MYHDFLINKFLAGS "-L${MYHDFPATH}/lib/ ${MYHDFPATH}/lib/libhdf5_hl.a ${MYHDFPATH}/lib/libhdf5.a ${LIB_ZPATH}/lib/libz.a ${LIB_SZPATH}/lib/libsz.a ${LIB_AECPATH}/lib/libaec.a -ldl")
	set(MYHDFLINKFLAGS "-L${MYHDFPATH}/lib/ ${MYHDFPATH}/lib/libhdf5_hl.a ${MYHDFPATH}/lib/libhdf5.a ${MYHDFPATH}/lib/libz.a ${MYHDFPATH}/lib/libszaec.a ${MYHDFPATH}/lib/libaec.a -ldl")
elseif()
	set(MYHDFPATH "")
	set(MYHDFLINKFLAGS "")
endif()

#find installed locations of files from libssl-dev package via typing dpkg -L libssl-dev on the command line

set(MYOPENSSL "/usr/lib/x86_64-linux-gnu")
include_directories("${MYOPENSSL}/lib")
#find_package(OpenSSL REQUIRED)
set(MYSSLLINKFLAGS "-L${MYOPENSSL} ${MYOPENSSL}/libssl.a ${MYOPENSSL}/libcrypto.a")

################################################################################################################
##AUTOMATIC SECTION#############################################################################################
##normally no further changes should be required below unless your an expert####################################
################################################################################################################

#user input sanity checks
if(EMPLOY_INTELCOMPILER AND EMPLOY_GNUCOMPILER)
	message([FATAL_ERROR] "You cannot utilize two compiler at the same time!")
endif()


#automatically assign project name and compiler flags
project(${MYPROJECTNAME})
set(CMAKE_BUILD_DIR "build")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MYOPTLEVEL}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYOPTLEVEL}")

#setting up compiler-specifics
#intel path
if(EMPLOY_INTELCOMPILER)
	message([STATUS] "Employing the Intel compiler!")
	if (VERBOSE_VECTORIZATION)
		set(MYVERBOSE "-qopt-report-phase=vec")
	endif()
	add_definitions("${MYOPTLEVEL} ${MYVERBOSE}")
	message([WARNING] "Currently not SIMD flags provided for Intel compile options")
	set(MYOMP "-qopenmp -lpthread")
	add_definitions(${MYOMP})
	#add_definitions("-std=c++0x")
	add_definitions("-std=c++17")
	add_definitions("-Warray-bounds -Wchar-subscripts -Wcomment -Wenum-compare -Wformat 
		-Wuninitialized -Wmaybe-uninitialized -Wmain -Wnarrowing -Wnonnull 
		-Wparentheses -Wpointer-sign -Wreorder -Wreturn-type -Wsign-compare 
		-Wsequence-point -Wtrigraphs -Wunused-variable")
#gcc path
elseif(EMPLOY_GNUCOMPILER)
	message([STATUS] "Employing the GNU compiler!")
	# if(VERBOSE_VECTORIZATION)
	# 	set(MYVERBOSE "-fopt-info")
	# endif()
	# add_definitions("${MYOPTLEVEL} ${MYVERBOSE} ${MYSIMDFLAGS}")
	# set(MYOMP "-fopenmp -lpthread")
	# add_definitions(${MYOMP})
	# add_definitions("-std=c++11")
	# add_definitions("-Wall -Warray-bounds -Wchar-subscripts -Wcomment -Wenum-compare -Wformat 
	# 	-Wuninitialized -Wmaybe-uninitialized -Wmain -Wnarrowing -Wnonnull 
	# 	-Wparentheses -Wreorder -Wreturn-type -Wsign-compare -Wsequence-point 
	# 	-Wtrigraphs -Wunused-function -Wunused-but-set-variable -Wunused-variable")
	# 	# set(MYVERBOSE "-fopt-info") # optimization details only
	set(MYVERBOSE "-Wall") # all warnings
	add_definitions("${MYOPTLEVEL}")
	add_definitions("${MYVERBOSE}")
	set(MYOMP "-fopenmp -lpthread")
	add_definitions(${MYOMP})
	add_definitions("-std=c++17")
else()
	message([FATAL_ERROR] "You have to utilize a compiler!")
endif()

message([STATUS] "Projectname is ${MYPROJECTNAME}")
message([STATUS] "We utilize optimization level ${MYOPTLEVEL}")

#parallelization - MPI process-level
#query location of MPI library
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

add_executable(${MYPROJECTNAME}
	${MYSRCPATH}/thirdparty/mandatory/lodepng/lodepng.cpp
	${MYSRCPATH}/thirdparty/mandatory/chrono/CHRONOTimeAndDateHandling.cpp
	${MYSRCPATH}/thirdparty/mandatory/openssl/OPENSSLInterface.cpp
	${MYSRCPATH}/thirdparty/mandatory/hdf5core/HDF5Core.cpp
	${MYSRCPATH}/SCORE_Random.cpp
	${MYSRCPATH}/SCORE_Math.cpp
	${MYSRCPATH}/SCORE_Io.cpp
	${MYSRCPATH}/SCORE_PercAnalyzer.cpp
	${MYSRCPATH}/SCORE_EBSDImporting.cpp
	${MYSRCPATH}/SCORE_GridPartitioning.cpp
	${MYSRCPATH}/SCORE_OrientationHdl.cpp
	${MYSRCPATH}/SCORE_GrainBoundaryPhysics.cpp
	${MYSRCPATH}/SCORE_RecoveryModels.cpp
	${MYSRCPATH}/SCORE_NucleationModels.cpp
	${MYSRCPATH}/SCORE_GeneralPhysics.cpp
	${MYSRCPATH}/SCORE_JunctionHdl.cpp
	${MYSRCPATH}/SCORE_StructureSynthesis.cpp
	${MYSRCPATH}/SCORE_GrowthMachine.cpp
	${MYSRCPATH}/SCORE_OnTheFlyCharacterization.cpp
	${MYSRCPATH}/SCORE_Io_HDF5.cpp
	${MYSRCPATH}/SCORE_Io_Characterization.cpp
	${MYSRCPATH}/SCORE_SolitaryUnitAveraging.cpp
	${MYSRCPATH}/SCORE_Kernel.cpp
	${MYSRCPATH}/SCORE_Main.cpp
)


#linking process
##mind the y and dl libraries at the end which are required for gcc support when using the hdf libraries
target_link_libraries(${MYPROJECTNAME} ${MYOMP} ${MPI_LIBRARIES} ${MYHDFLINKFLAGS} ${MYSSLLINKFLAGS})

#MPI compilation settings
if(MPI_COMPILE_FLAGS)
  set_target_properties(${MYPROJECTNAME} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(${MYPROJECTNAME} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
