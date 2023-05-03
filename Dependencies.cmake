cmake_minimum_required(VERSION 3.18.4)

################################################################################################################################
##USER INTERACTION##############################################################################################################
##in this section modifications and local paths need to be modified by the user#################################################
################################################################################################################################
#choose which target system to use
set(USE_WINWSL2_OR_LINUX ON)

#SPECIFY THE HOME/ROOT DIRECTORY OF THIS LOCAL SCORE INSTANCE
if(USE_WINWSL2_OR_LINUX)
	set(MYPROJECTPATH "${CMAKE_CURRENT_LIST_DIR}")
endif()

#INTEGRATE THE HASH OF THE PARENT COMMIT FROM WHICH THIS VERSION WAS CREATED
#update GitSHA of the paraprobe repository prior commit, i.e. pre-processing upon compilation pulling last GitSHA
#we cannot update the current GitSHA as this would create a circular dependency, namely once we have commit, i.e. know the file
# set(GITSHA "0")
message([STATUS] "MYGITSHA: ${GITSHA}")
string(REGEX REPLACE "-" "_" MYCLEANGITSHA ${GITSHA}) #'-' is an invalid character in a C preprocessor macro
message([STATUS] "MYCLEANGITSHA: ${MYCLEANGITSHA}")
add_definitions("-DGITSHA=${MYCLEANGITSHA}")

#tell where the source code of PARAPROBE is
set(MYSRCPATH "${MYPROJECTPATH}/src/cxx")

#SPECIFY LOCATION OF UTILITIES
set(LIB_SZPATH "/opt/szip-2.1.1/GNU/11.2.0/")
set(LIB_ZPATH "/usr/local/")
set(LIB_AECPATH "/usr/local/")

set(MYHDFPATH "${MYPROJECTPATH}/src/cxx/thirdparty/mandatory/hdf5/CMake-hdf5-1.14.0/HDF5-1.14.0-Linux/HDF_Group/HDF5/1.14.0")
include_directories("${MYHDFPATH}/include")
link_directories("${MYHDFPATH}/lib")
#link against static libraries
set(MYHDFLINKFLAGS "-L${MYHDFPATH}/lib/ ${MYHDFPATH}/lib/libhdf5_hl.a ${MYHDFPATH}/lib/libhdf5.a ${MYHDFPATH}/lib/libz.a ${MYHDFPATH}/lib/libszaec.a ${MYHDFPATH}/lib/libaec.a -ldl")

#find installed locations of files from libssl-dev package via typing dpkg -L libssl-dev on the command line

set(MYOPENSSL "/usr/lib/x86_64-linux-gnu")
include_directories("${MYOPENSSL}/lib")
#find_package(OpenSSL REQUIRED)
set(MYSSLLINKFLAGS "-L${MYOPENSSL} ${MYOPENSSL}/libssl.a ${MYOPENSSL}/libcrypto.a")


#choose which compiler to use
set(EMPLOY_INTELCOMPILER OFF)
set(EMPLOY_GNUCOMPILER ON)

#choose optimization level
##-O0 for debug purposes only
##-O1 or -O2 for moderate optimization
##-O3 for production level optimization
set(MYOPTLEVEL "-O2")

