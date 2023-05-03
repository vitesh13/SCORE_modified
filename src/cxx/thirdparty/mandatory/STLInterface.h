/*
* This file is part of paraprobe-toolbox.
*
* paraprobe-toolbox is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License,
*  or (at your option) any later version.
*
* paraprobe-toolbox is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with paraprobe-toolbox. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef __STLINTERFACE__
#define __STLINTERFACE__

//C++ STL
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstring>
#include <stdexcept>
#include <string>
#include <iomanip>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <list>

#include <cassert>
#include <map>
#include <iterator>
#include <utility>
#include <set>
#include <unordered_set>
#include <type_traits>

#include <random>
#include <complex>

//low level stuff for interacting with the operating system
//#define NDEBUG
#include <assert.h>
#include <stdint.h>
#include <ios>

//C header stuff to pull low level system and process status pieces of information
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

//##MK::for querying system maximum physical memory see
//https://stackoverflow.com/questions/2513505/how-to-get-available-memory-c-g
//https://stackoverflow.com/questions/850774/how-to-determine-the-hardware-cpu-and-ram-on-a-machine
//https://stackoverflow.com/questions/349889/how-do-you-determine-the-amount-of-linux-system-ram-in-c
//https://stackoverflow.com/questions/5553665/get-ram-system-size


//forward declaration for global scope
using namespace std;

//MK::undefine to switch to use double precision
#define EMPLOY_SINGLE_PRECISION

#define EPSILON_F32							(1.0e-5)
#define EPSILON_F64							(1.0e-12)

#ifdef EMPLOY_SINGLE_PRECISION
	#define EPSILON							EPSILON_F32
	typedef float apt_real;

	typedef int apt_int;
	typedef unsigned int apt_uint;
	#define MYZERO 							(0.f)
	#define MYONE 							(1.f)
#elif
	#define EPSILON							EPSILON_F64
	typedef double apt_real;
	typedef long apt_int;
	typedef unsigned long apt_uint;
	#define MYZERO 							(0.0)
	#define MYONE 							(1.0)
#endif

//##MK::obsolete typedef size_t apt_sizet;

//type range
#define U8MX							(numeric_limits<unsigned char>::max())
#define U8MI							(numeric_limits<unsigned char>::lowest())
#define I8MX							(numeric_limits<char>::max())
#define I8MI							(numeric_limits<char>::lowest())
#define U16MX							(numeric_limits<unsigned short>::max())
#define U16MI							(numeric_limits<unsigned short>::lowest())
#define I16MX							(numeric_limits<short>::max())
#define I16MI							(numeric_limits<short>::lowest())
#define U32MX							(numeric_limits<unsigned int>::max())
#define U32MI							(numeric_limits<unsigned int>::lowest())
#define I32MX							(numeric_limits<int>::max())
#define I32MI							(numeric_limits<int>::lowest())
#define U64MX							(numeric_limits<unsigned long>::max())
#define U64MI							(numeric_limits<unsigned long>::lowest())
#define I64MX							(numeric_limits<long>::max())
#define I64MI							(numeric_limits<long>::lowest())
#define F32MX							(numeric_limits<float>::max())
#define F32MI							(numeric_limits<float>::lowest())
#define F64MX							(numeric_limits<double>::max())
#define F64MI							(numeric_limits<double>::lowest())

#ifdef EMPLOY_SINGLE_PRECISION
	#define UMI							(U32MI)
	#define UMX							(U32MX)
	#define IMI							(I32MI)
	#define IMX							(I32MX)
	#define RMI							(F32MI)
	#define RMX							(F32MX)
#elif
	#define UMI							(U64MI)
	#define UMX							(U64MX)
	#define IMI							(I64MI)
	#define IMX							(I64MX)
	#define RMI							(F64MI)
	#define RMX							(F64MX)
#endif


#endif
