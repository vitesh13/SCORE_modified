cmake_minimum_required(VERSION 3.18.4)

################################################################################################################
##USER INTERACTION##############################################################################################
##in this section modifications and paths need to be added by the user##########################################
################################################################################################################

set(MYPROJECTNAME "score")
message([STATUS] "Projectname is ${MYPROJECTNAME}")
project(${MYPROJECTNAME} LANGUAGES CXX)
set(CMAKE_BUILD_DIR "build")

#pull general information about external libraries and paths related to paraprobe
include("Dependencies.cmake")
message([STATUS] "MYPROJECTPATH: ${MYPROJECTPATH}")
message([STATUS] "MYHDFPATH: ${MYHDFPATH}")
#message([STATUS] "LIB_SZPATH: ${LIB_SZPATH}")
#message([STATUS] "LIB_ZPATH: ${LIB_ZPATH}")
#message([STATUS] "LIB_AECPATH: ${LIB_AECPATH}")
message([STATUS] "MYHDFLINKPATH: ${MYHDFLINKFLAGS}")
message([STATUS] "MYRYMLLINKPATH: ${MYRYMLLINKFLAGS}")


################################################################################################################
##AUTOMATIC SECTION#############################################################################################
##normally no further changes should be required below unless your an expert####################################
################################################################################################################

#user input sanity checks
if(EMPLOY_INTELCOMPILER AND EMPLOY_GNUCOMPILER)
	message([FATAL_ERROR] "You cannot utilize two compiler at the same time!")
endif()

#automatically assign project name and compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MYOPTLEVEL}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYOPTLEVEL}")

#setting up compiler-specifics
message([STATUS] "MYOPTLEVEL: ${MYOPTLEVEL}")
if(EMPLOY_INTELCOMPILER)  # intel path
	message([STATUS] "Employing the Intel compiler!")
	set(MYVERBOSE "-qopt-report-phase=vec")
	add_definitions("${MYOPTLEVEL} ${MYVERBOSE}")
	message([WARNING] "Currently not SIMD flags provided for Intel compile options")
	set(MYOMP "-qopenmp -lpthread")
	add_definitions(${MYOMP})
	add_definitions("-std=c++17")
	add_definitions("-Warray-bounds -Wchar-subscripts -Wcomment -Wenum-compare -Wformat 
		-Wuninitialized -Wmaybe-uninitialized -Wmain -Wnarrowing -Wnonnull 
		-Wparentheses -Wpointer-sign -Wreorder -Wreturn-type -Wsign-compare 
		-Wsequence-point -Wtrigraphs -Wunused-variable")
elseif(EMPLOY_GNUCOMPILER)  # gcc path
	message([STATUS] "Employing the GNU compiler!")
	set(MYVERBOSE "-Wall") # all warnings
	add_definitions("${MYOPTLEVEL}")
	add_definitions("${MYVERBOSE}")
	set(MYOMP "-fopenmp -lpthread")
	add_definitions(${MYOMP})
	add_definitions("-std=c++17")
else()
	message([FATAL_ERROR] "You have to utilize a compiler!")
endif()

#parallelization - MPI process-level
#query location of MPI library
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

add_executable(${MYPROJECTNAME}
	${MYSRCPATH}/thirdparty/mandatory/lodepng/lodepng.cpp
	${MYSRCPATH}/thirdparty/mandatory/chrono/CHRONOTimeAndDateHandling.cpp
	${MYSRCPATH}/thirdparty/mandatory/openssl/OPENSSLInterface.cpp
	${MYSRCPATH}/thirdparty/mandatory/hdf5core/HDF5Core.cpp
	# ${MYSRCPATH}/thirdparty/mandatory/yamlcore/RAPIDYAMLCore.h
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
target_link_libraries(${MYPROJECTNAME} ${MYOMP} ${MPI_LIBRARIES} ${MYHDFLINKFLAGS} ${MYSSLLINKFLAGS} ${MYRYMLLINKFLAGS})

#MPI compilation settings
if(MPI_COMPILE_FLAGS)
  set_target_properties(${MYPROJECTNAME} PROPERTIES COMPILE_FLAGS "${MPI_COMPILE_FLAGS}")
endif()

if(MPI_LINK_FLAGS)
  set_target_properties(${MYPROJECTNAME} PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}")
endif()
