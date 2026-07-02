cmake_minimum_required(VERSION 3.18.4)

################################################################################################################################
##USER INTERACTION##############################################################################################################
##in this section modifications and local paths need to be modified by the user#################################################
################################################################################################################################
#choose which target system to use
set(USE_WINWSL2_OR_LINUX ON)
set(CONDA_BUILD ON)

#SPECIFY THE HOME/ROOT DIRECTORY OF THIS LOCAL SCORE INSTANCE
if(USE_WINWSL2_OR_LINUX)
	set(MYPROJECTPATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

#INTEGRATE THE HASH OF THE PARENT COMMIT FROM WHICH THIS VERSION WAS CREATED
#update GitSHA of the paraprobe repository prior commit, i.e. pre-processing upon compilation pulling last GitSHA
#we cannot update the current GitSHA as this would create a circular dependency, namely once we have commit, i.e. know the file
#'-' is an invalid character in a C preprocessor macro
string(REGEX REPLACE "-" "_" MYCLEANSCOREVERSION ${SCOREVERSION}) 
add_definitions("-DSCOREVERSION=${MYCLEANSCOREVERSION}")
string(REGEX REPLACE "-" "_" MYCLEANNEXUSVERSION ${NEXUSVERSION})
add_definitions("-DNEXUSVERSION=${MYCLEANNEXUSVERSION}")
message([STATUS] "DSCOREVERSION: ${DSCOREVERSION}")
message([STATUS] "DNEXUSVERSION: ${DNEXUSVERSION}")


#tell where the source code is
set(MYSRCPATH "${MYPROJECTPATH}/src/cxx")

#mandatory hdf5 library
if(CONDA_BUILD)
	if(NOT DEFINED ENV{CONDA_PREFIX})
		message(FATAL_ERROR "CONDA_PREFIX not set. Activate your conda environment first.")
	endif()
	set(CONDA_PREFIX $ENV{CONDA_PREFIX})
	
	include_directories("${CONDA_PREFIX}/include")
	link_directories("${CONDA_PREFIX}/lib")
	set(MYHDFLINKFLAGS "-L${CONDA_PREFIX}/lib -lhdf5_hl -lhdf5 -ldl")
	
	include_directories("${CONDA_PREFIX}/include")
	link_directories("${CONDA_PREFIX}/lib")
	set(MYSSLLINKFLAGS "-L${CONDA_PREFIX}/lib -lssl -lcrypto")
else()
	set(MYHDFPATH "${MYPROJECTPATH}/src/cxx/thirdparty/mandatory/hdf5/HDF5-1.14.6-Linux/HDF_Group/HDF5/1.14.6")
	set(MYHDFLINKFLAGS "-L${MYHDFPATH}/lib/ ${MYHDFPATH}/lib/libhdf5_hl.a ${MYHDFPATH}/lib/libhdf5.a ${MYHDFPATH}/lib/libszaec.a ${MYHDFPATH}/lib/libaec.a -ldl")
	include_directories("${MYHDFPATH}/include")
	link_directories("${MYHDFPATH}/lib")
	
	#find installed locations of files from libssl-dev package via typing dpkg -L libssl-dev on the command line
	set(MYOPENSSL "/usr/lib/x86_64-linux-gnu")
	include_directories("${MYOPENSSL}/lib")
	set(MYSSLLINKFLAGS "-L${MYOPENSSL} ${MYOPENSSL}/libssl.a ${MYOPENSSL}/libcrypto.a")
endif()

# optional stuff for developers for using ryml
# python
# git clone --recursive https://github.com/biojppm/rapidyaml.git
# cd tools && python amalgamate.py ryml_all.hpp && cd ..
# mkdir build && cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# make -j8 && cd ..
# set(MYRYMLLINKFLAGS "-L${MYPROJECTPATH}/src/cxx/thirdparty/mandatory/rapidyaml/build/libryml.a")


#choose which compiler to use
set(EMPLOY_INTELCOMPILER OFF)
set(EMPLOY_GNUCOMPILER ON)

#choose optimization level
##-O0 -g for debug purposes only
##-O1 or -O2 for moderate optimization
##-O3 for production level optimization
set(MYOPTLEVEL "-O2")
